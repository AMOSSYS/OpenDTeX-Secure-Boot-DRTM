// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organizations nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>
#include "crypto.h"
#include "libtcg.h"

static BYTE SYMKEY[AES_MAXKEYBYTES];
static BYTE *SEALED_SYMKEY = 0;
static UINT32 len_SEALED_SYMKEY = 0;

static char filenameIN[MAXSIZE] = "";
static char filenameOUT[MAXSIZE] = "";
static FILE * filein;
static FILE * fileout;


/*******************************************************************************************************/
/*******************************************************************************************************/

static void usage() {
	printf("tcg_seal [-k <keyfile>]* [-z] [-p <PCR1:PCR2:...>] [-h | --help] [-o <file out>] [-i <file in>]\n");
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//Load keys
static int RecLoadArchitecture(TCG_KEY *kKey) {
	TSS_RESULT result;
	
	if(kKey == 0)
		return 0;

	if(kKey->depth) {
		//We load the key from the disk
		if(LoadKeyFromDisk(kKey))
			return 1;
	} else {
		//SRK KEY
		result = Tspi_Context_LoadKeyByUUID(hContext, TSS_PS_TYPE_SYSTEM, SRK_UUID, &kKey->hKey);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During loading architecture of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Context_LoadKeyByUUID", result);
			return 1;
		}

		result = Tspi_GetPolicyObject(kKey->hKey, TSS_POLICY_USAGE, &kKey->hPolicy);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During loading architecture of %s : \n", __FUNCTION__, kKey->name);			
			tspiError("Echec Tspi_GetPolicyObject", result);
			return 1;
		}

		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During loading architecture of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	}

	//RECURSIVE CALL
	return RecLoadArchitecture(kKey->kChildren);
}

/*******************************************************************************************************/
/*******************************************************************************************************/
//return 0 if success
static int generate_key() {
	FILE 	*file;
	int 	res = 0;

	file = fopen("/dev/urandom", "rb");
	if(file == 0)
		return 1;

	res = fread(SYMKEY, sizeof(SYMKEY), 1, file);
	if(res != 1) {
		fprintf(stderr, "(%s) : Read error\n", __FUNCTION__);
		fclose(file);
		return 1;
	}
	printf("symmetric key : ");
	for(res = 0 ; res < sizeof(SYMKEY) ; res++) {
		printf("%02x ", SYMKEY[res]);
	}
	printf("\n");

	return 0;
}

static int seal_key(int isWellknown, int flagsPCR) {
	TSS_RESULT 		result;
	TSS_HENCDATA		hEncdata;
	TSS_HPOLICY		hEncPolicy;
	TSS_HPCRS		hPcrs;
	char 			pwd[MAXSIZE];
	char 			pwd2[MAXSIZE];
	
	//Creation de la liste des PCR
	if(CreatePCR(&hPcrs, flagsPCR))
		return 1;
	
	//Creation d'un blob de chiffrement
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_ENCDATA, TSS_ENCDATA_SEAL, &hEncdata);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}

	//Creation du secret d'authentification de la clé symétrique à sceller
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_POLICY, TSS_POLICY_USAGE, &hEncPolicy);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}
	
	if(isWellknown) {
		result = Tspi_Policy_SetSecret(hEncPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	} else {
		//Use of password
		while(1) {
			fprintf(stderr, "Enter a password for data : ");
			scanf("%s", pwd);
			
			if(strlen(pwd) > MAXSIZE) {
				fprintf(stderr, "Password too long. Limited to %d octets\n", MAXSIZE);
				continue;
			}
			
			fprintf(stderr, "Enter a password for data (bis) : ");
			scanf("%s", pwd2);
			
			if(strncmp(pwd, pwd2, MAXSIZE) != 0) {
				fprintf(stderr, "Passwords don't match\n");
				continue;
			}
			
			break;
		}

		result = Tspi_Policy_SetSecret(hEncPolicy, TSS_SECRET_MODE_PLAIN, strlen(pwd), (BYTE *) pwd);
		burn(pwd, sizeof(pwd))
		burn(pwd2, sizeof(pwd2))
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	}
	
	//Application du secret d'authentification
	result = Tspi_Policy_AssignToObject(hEncPolicy, hEncdata);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Policy_AssignToObject", result);
		return 1;
	}
	
	//scellement de la clé symétrique
	result = Tspi_Data_Seal(hEncdata, currentKey->hKey, sizeof(SYMKEY), SYMKEY, hPcrs);
	if(result != TSS_SUCCESS) {
		//IF the wellknown password has been used for the sealement key THEN we ask for a password
		if(currentKey->isWellKnown) {
			currentKey->isWellKnown = 0;
		
			fprintf(stderr, "Password of %s : ", currentKey->name);
			scanf("%s", pwd);

			result = Tspi_Policy_SetSecret(currentKey->hPolicy, TSS_SECRET_MODE_PLAIN, strlen(pwd), (BYTE *)pwd);
			burn(pwd, sizeof(pwd))
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
				tspiError("Echec Tspi_Policy_SetSecret", result);	
				return 1;
			}
		
			result = Tspi_Data_Seal(hEncdata, currentKey->hKey, sizeof(SYMKEY), SYMKEY, hPcrs);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
				tspiError("Tspi_Data_Seal", result);
				return 1;
			}
		} else {
			fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
			tspiError("Echec Tspi_Data_Seal", result);
			return 1;
		}
	}
	
	result = Tspi_GetAttribData(hEncdata, TSS_TSPATTRIB_ENCDATA_BLOB, TSS_TSPATTRIB_ENCDATABLOB_BLOB, &len_SEALED_SYMKEY, &SEALED_SYMKEY);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During sealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_GetAttribData", result);
		return 1;
	}
		
	return 0;
}

/*
 * Scelle le fichier passe dans filein.
 */
static int cipher_file() {
	int 			cpt;
	int 			res;
	int			written;
	BYTE 			buffer[AES_BLOCK_SIZE * 5];
	BYTE 			cipher[AES_BLOCK_SIZE * 5];
	AESCBC_ctx 		ctx;
	int 			eof = 0;

	//Writing size of SEALED_SYMKEY
	res = fwrite(&len_SEALED_SYMKEY, sizeof(len_SEALED_SYMKEY), 1, fileout);
	if(res != 1) {
		fprintf(stderr, "(%s) : Write error during writing the size of the sealed key\n", __FUNCTION__);
		return 1;
	}

	//Writing SEALED_SYMKEY
	res = fwrite(SEALED_SYMKEY, len_SEALED_SYMKEY, 1, fileout);
	if(res != 1) {
		fprintf(stderr, "(%s) : Write error during writing the sealed key\n", __FUNCTION__);
		return 1;
	}

	//Initialization of AES CBC
	if(init_AESCBC(&ctx, SYMKEY, sizeof(SYMKEY)))
		return 1;

	//Reading the data, encrypt it and write it
	//On lit par blocs de 5*16
	cpt = 0;
	while(1) {
		res = fread(&buffer[cpt], 1, AES_BLOCK_SIZE * 5 - cpt, filein);
		cpt += res;

		if(res == 0)
			eof = 1;
		
		//we process all complete block and we shift ending bytes to the beginning of buffer
		encrypt_AESCBC(&ctx, cipher, buffer, (cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE);

		written = 0;
		while(written < (cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE) {
			res = fwrite(&cipher[written], 1, (cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE - written, fileout);
			if(res == 0) {
				fprintf(stderr, "(%s) : Write error during writing the data\n", __FUNCTION__);
				return 1;
			}
			written += res;
		}
		// Correction : cpy of block size minus "(cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE)" instead of "cpt"
		memcpy(buffer, &buffer[(cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE], AES_BLOCK_SIZE * 5 - (cpt / AES_BLOCK_SIZE) * AES_BLOCK_SIZE); 
	
		cpt %= AES_BLOCK_SIZE;
		
		//end of the file, so we use PKCS5PADDING
		if(eof) {		
			memset(&buffer[cpt], AES_BLOCK_SIZE - cpt, AES_BLOCK_SIZE - cpt);
			encrypt_AESCBC(&ctx, cipher, buffer, AES_BLOCK_SIZE);

			written = 0;
			while(written < AES_BLOCK_SIZE) {
				res = fwrite(&cipher[written], 1, AES_BLOCK_SIZE - written, fileout);
				if(res == 0) {
					fprintf(stderr, "(%s) : Write error during padding\n", __FUNCTION__);
					return 1;
				}
				written += res;
			}
			break;
		}
	}
	
	close_AESCBC(&ctx);
	burn(buffer, sizeof(buffer))
	
	return 0;
}

/*******************************************************************************************************/
/*******************************************************************************************************/

int main(int argc, char **argv) {
	TSS_RESULT	result = 0;
	int 			i = 1;
	int 			flagsPCR = 0;
	int			isWellknown = 0;
	int 			depth = 0;

	
	while(i < argc) {
	  if ( strncmp(argv[i], "-a", sizeof("-a")) == 0 ) {
	    fprintf(stdout, "Automatic initialization... \n\tBeware you have a key1.key with pass key1, and an struct.data which contains data to seal.\n");
	    
	    depth++;
	    if(InsertKey(depth, "key1.key", 1, 0)) {
	      fprintf(stderr, "InsertKey function has returned 0.");
	      return 1;
	    }
	    isWellknown = 1;
	    strncpy(filenameIN, "struct.data", MAXSIZE);
	    strncpy(filenameOUT, "data.seal", MAXSIZE);
	  } else if(strncmp(argv[i], "-k", sizeof("-k")) == 0) {
	    if(i + 1 >= argc) {
	      fprintf(stderr, "Syntax error\n");
	      usage();
	      return 1;
	    }
	    
	    depth++;
	    if(InsertKey(depth, argv[i + 1], 1, 0)) {
	      return 1;
	    }
	    
	    i += 2;
	  } else if(strncmp(argv[i], "-p", sizeof("-p")) == 0) {
	    if(i + 1 >= argc) {
	      fprintf(stderr, "Syntax error\n");
	      usage();
	      return 1;
	    }
	    
	    if(parsePCR(argv[i + 1], &flagsPCR)) {
	      fprintf(stderr, "Invalid list of PCR : %s\n", argv[i + 3]);
	      return 1;
	    }
	    
	    i += 2;
	  } else if(strncmp(argv[i], "-i", sizeof("-i")) == 0) {
	    if(i + 1 >= argc) {
	      fprintf(stderr, "Syntax error\n");
	      usage();
	      return 1;
	    }
	    
	    strncpy(filenameIN, argv[i + 1], MAXSIZE);
	    
	    i += 2;
	  } else if(strncmp(argv[i], "-o", sizeof("-o")) == 0) {
	    if(i + 1 >= argc) {
	      fprintf(stderr, "Syntax error\n");
	      usage();
	      return 1;
	    }
	    
	    strncpy(filenameOUT, argv[i + 1], MAXSIZE);
	    
	    i += 2;
	  } else if(strncmp(argv[i], "-z", sizeof("-z")) == 0) {
	    isWellknown = 1;
	    i++;
	  } else if(strncmp(argv[i], "-h", sizeof("-h")) == 0) {
	    usage();
	    return 0;
	  } else if(strncmp(argv[i], "--help", sizeof("--help")) == 0) {
	    usage();
	    return 0;
	  } else {
	    fprintf(stderr, "Syntax error\n");
	    usage();
	    return 1;
	  }
	}

	//Display architecture of key
	RecPrintArchitecture(&kSRK);

	//Creation of the TSS Context
	result = Tspi_Context_Create(&hContext);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Create", result);
		exit(1);
	}

	//Connection to the TSS Context
	result = Tspi_Context_Connect(hContext, NULL);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Connext", result);
		exit(1);
	}

	filein = stdin;
	fileout = stdout;
	
	//Get the handle of the TPM
	result = Tspi_Context_GetTpmObject(hContext, &hTpm);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_GetTpmObject", result);
		result = 1;
		goto end;
	}

	//Build key architecture
	result = RecLoadArchitecture(&kSRK);
	if(result)
		goto end;

	result = generate_key();
	if(result)
		goto end;
	
	result = seal_key(isWellknown, flagsPCR);
	if(result)
		goto end;

	//OPEN INPUT
	if(filenameIN[0] != 0) {
		//On ouvre le fichier de donnees, et on construit le fichier formatte
		
		filein = fopen(filenameIN, "rb");
		if(filein == 0) {
			filein = stdin;
			fprintf(stderr, "Unable to open file %s\n", filenameIN);
			result = 1;
			goto end;
		 }
	}
		

	//OPEN OUTPUT
	if(filenameOUT[0] != 0) {
		fileout = fopen(filenameOUT, "wb");
		if(fileout == 0) {
			fileout = stdout;
			fprintf(stderr, "Unable to open file %s\n", filenameOUT);
			result = 1;
			goto end;
		}
	}
	
	result = cipher_file();
	burn(SYMKEY, sizeof(SYMKEY))
	
end:
	if(filein != stdin)
		fclose(filein);
	if(fileout != stdout)
		fclose(fileout);

	//Close TSS session
	if(SEALED_SYMKEY)
		Tspi_Context_FreeMemory(hContext, SEALED_SYMKEY);

	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);

	return result;
}

