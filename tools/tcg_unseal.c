// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2013, AMOSSYS
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

/*
 * File contributors:
 *   - Goulven Guiheux (AMOSSYS)
 */


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>
#include "crypto.h"
#include "libtcg.h"

static BYTE *SYMKEY;
static UINT32 len_SYMKEY = 0;
static BYTE *SEALED_SYMKEY = 0;
static UINT32 len_SEALED_SYMKEY = 0;

static char filenameIN[MAXSIZE] = "";
static char filenameOUT[MAXSIZE] = "";
static FILE * filein;
static FILE * fileout;

/*******************************************************************************************************/
/*******************************************************************************************************/

static void usage() {
	printf("tcg_unseal [-k <keyfile>]* [-z] [-h | --help] [-o <file out>] [-i <file in>]\n");
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
static int extract_sealedkey() {
	int res;
	
	res = fread(&len_SEALED_SYMKEY, sizeof(len_SEALED_SYMKEY), 1, filein);
	if(res != 1) {
		fprintf(stderr, "(%s) : Read error during reading the size of the sealed key\n", __FUNCTION__);
		return 1;
	}
	
	SEALED_SYMKEY = malloc(len_SEALED_SYMKEY);
	if(SEALED_SYMKEY == 0) {
		fprintf(stderr, "(%s) : Out of memory\n", __FUNCTION__);
		return 1;
	}

	res = fread(SEALED_SYMKEY, len_SEALED_SYMKEY, 1, filein);
	if(res != 1) {
		fprintf(stderr, "(%s) : Read error during reading the sealed key\n", __FUNCTION__);
		return 1;
	}

	return 0;
}

//return 0 if success
static int unseal_key(int isWellknown) {
	TSS_RESULT 		result;
	TSS_HENCDATA	hDecdata;
	TSS_HPOLICY		hDecPolicy;
	char 				pwd[MAXSIZE];
	
	/* Creation d'un blob de chiffrement à partir des données chiffrées*/
	result = Tspi_Context_CreateObject(hContext,	TSS_OBJECT_TYPE_ENCDATA, TSS_ENCDATA_SEAL, &hDecdata);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}

	result = Tspi_SetAttribData(hDecdata, TSS_TSPATTRIB_ENCDATA_BLOB, TSS_TSPATTRIB_ENCDATABLOB_BLOB, len_SEALED_SYMKEY, SEALED_SYMKEY);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_SetAttribData", result);
		return 1;
	}
	
	//Creation du secret d'authentification de la clé symétrique à desceller
	result = Tspi_Context_CreateObject(hContext,	TSS_OBJECT_TYPE_POLICY, TSS_POLICY_USAGE, &hDecPolicy);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}
	
	if(isWellknown) {
		result = Tspi_Policy_SetSecret(hDecPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	} else {
		//Demander le mot de passe de la donnée
		fprintf(stderr, "Password of the data : ");			
		scanf("%s", pwd);

		result = Tspi_Policy_SetSecret(hDecPolicy, TSS_SECRET_MODE_PLAIN, strlen(pwd), (BYTE *)pwd);
		burn(pwd, sizeof(pwd))
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	}

	result = Tspi_Policy_AssignToObject(hDecPolicy, hDecdata);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
		tspiError("Echec Tspi_Policy_AssignToObject", result);
		return 1;
	}

	/* scellement des données */
	result = Tspi_Data_Unseal(hDecdata, currentKey->hKey, &len_SYMKEY, &SYMKEY);
	if(result != TSS_SUCCESS) {
		if(currentKey->isWellKnown) {
			currentKey->isWellKnown = 0;

			fprintf(stderr, "Password of %s : ", currentKey->name);
			scanf("%s", pwd);

			result = Tspi_Policy_SetSecret(currentKey->hPolicy, TSS_SECRET_MODE_PLAIN,	strlen(pwd), (BYTE *)pwd);
			burn(pwd, sizeof(pwd))
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
				tspiError("Echec Tspi_Policy_SetSecret", result);	
				return 1;
			}
		
			result = Tspi_Data_Unseal(hDecdata, currentKey->hKey, &len_SYMKEY, &SYMKEY);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During unsealing the key\n", __FUNCTION__);
				tspiError("Echec Tspi_Data_Unseal", result);
				return 1;
			}
		} else {
			tspiError("Echec Tspi_Data_Unseal", result);
			return 1;
		}
	}
		
	return 0;
}

static int uncipher_file() {
	int 			res;
	int			written;
	BYTE 			buffer[AES_BLOCK_SIZE * 2];
	BYTE 			clear[AES_BLOCK_SIZE];
	AESCBC_ctx 	ctx;

	//Initialization of AES CBC
	if(init_AESCBC(&ctx, SYMKEY, len_SYMKEY))
		return 1;
	
	//Intialization by reading the first block
	res = fread(buffer, AES_BLOCK_SIZE, 1, filein);
	if(res == 0) {
		return 0;
	} else if(res != 1) {
		fprintf(stderr, "(%s) : Read error during writing the data\n", __FUNCTION__);
		return 1;
	}

	//Reading the data, decrypt it and write it
	while(1) {
		//Read one block
		res = fread(&buffer[AES_BLOCK_SIZE], 1, AES_BLOCK_SIZE, filein);
		if(res != 0) {
			if(res % AES_BLOCK_SIZE != 0) {
				fprintf(stderr, "(%s) : encrypted data not aligned\n", __FUNCTION__);
				return 1;
			}

			//we process a block and we shift the second to the beginning of buffer
			decrypt_AESCBC(&ctx, clear, buffer, AES_BLOCK_SIZE);

			res = fwrite(clear, AES_BLOCK_SIZE, 1, fileout);
			if(res != 1) {
				fprintf(stderr, "(%s) : Write error during writing the data\n", __FUNCTION__);
				return 1;
			}
		
			memcpy(buffer, &buffer[AES_BLOCK_SIZE], AES_BLOCK_SIZE);
		} else {
			//No more data to read
			BYTE val;
			
			decrypt_AESCBC(&ctx, clear, buffer, AES_BLOCK_SIZE);

			val = clear[AES_BLOCK_SIZE - 1];

			//verify padding
			for(res = 0 ; res < val ; res++) {
				if(clear[AES_BLOCK_SIZE - res - 1] != val) {
					fprintf(stderr, "(%s) : Padding error during writing the data\n", __FUNCTION__);
					return 1;
				}
			}

			written = 0;
			while(written < AES_BLOCK_SIZE - val) {
				res = fwrite(&clear[written], 1, AES_BLOCK_SIZE - val - written, fileout);
				if(res == 0) {
					fprintf(stderr, "(%s) : Write error during writing the data\n", __FUNCTION__);
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
	int			isWellknown = 0;
	int			depth = 0;

	if ( strncmp(argv[1], "-a", sizeof("-a")) == 0 ) {
		fprintf(stdout, "Automatic initialization... \n\tBeware you have a key1.key, and a data.seal which contains data to seal.\n");
		
		depth++;
		if(InsertKey(depth, "key1.key", 1, 0)) {
			fprintf(stderr, "InsertKey function has returned 0.");
			return 1;
		}
		isWellknown = 1;
		strncpy(filenameIN, "data.seal", MAXSIZE);
		strncpy(filenameOUT, "data.unseal", MAXSIZE);
	}
	else {
		while(i < argc) {
			if(strncmp(argv[i], "-k", sizeof("-k")) == 0) {
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

	//OPEN INPUT
	if(filenameIN[0] != 0) {
		filein = fopen(filenameIN, "rb");
		if(filein == 0) {
			filein = stdin;
			fprintf(stderr, "Enable to open file %s\n", filenameIN);
			result = 1;
			goto end;
		 }
	}

	//OPEN OUTPUT
	if(filenameOUT[0] != 0) {
		fileout = fopen(filenameOUT, "wb");
		if(fileout == 0) {
			fileout = stdout;
			fprintf(stderr, "Enable to open file %s\n", filenameOUT);
			result = 1;
			goto end;
		}
	}
		
	
	result = extract_sealedkey();
	if(result)
		goto end;

	result = unseal_key(isWellknown);
	if(result)
		goto end;

	result = uncipher_file();
	burn(SYMKEY, len_SYMKEY)
	
end:
	if(filein != stdin)
		fclose(filein);
	if(fileout != stdout)
		fclose(fileout);
	
	if(SEALED_SYMKEY)
		free(SEALED_SYMKEY);

	//Close TSS session
	if(SYMKEY)
		Tspi_Context_FreeMemory(hContext, SYMKEY);
	
	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);

	return result;
}

