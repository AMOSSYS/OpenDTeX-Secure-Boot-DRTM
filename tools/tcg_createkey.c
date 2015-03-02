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
#include "libtcg.h"

static void usage() {
	printf("tcg_createkey [-k | -z <depth> <keyfile> <PCR1:PCR2:...>]* [-h | --help]\n");
	printf("\t0 is the depth of SRK\n");
	printf("\t-z if for key which use the wellknown password\n");
	printf("\t-k if for key which use a password\n");
	printf("\tuse ':' for a null list of PCR\n");
	printf("tcg_createkey <-f file>\n");
	printf("\tLoad a configuration from a file. Entry = -k | -z <depth> <keyfile> <PCR1:PCR2:...>\n");
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//return 0 if success
static int CreateKey(TCG_KEY *kKey) {
	char 			pwd[MAXSIZE];
	char 			pwd2[MAXSIZE];
	TSS_RESULT	result;
	TSS_FLAG 	keyFlags = TSS_KEY_TYPE_STORAGE | TSS_KEY_SIZE_2048 |
			 						TSS_KEY_VOLATILE | TSS_KEY_AUTHORIZATION |
			 						TSS_KEY_NOT_MIGRATABLE;
	
	//Creation of key object
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_RSAKEY, keyFlags, &kKey->hKey);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}

	//Creation of policy object for the key
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_POLICY, TSS_POLICY_USAGE, &kKey->hPolicy);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}

	if(kKey->isWellKnown) {
		//Use of the WellKnown password
		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	} else {
		//Use of password
		while(1) {
			fprintf(stderr, "Enter a password for %s : ", kKey->name);
			scanf("%s", pwd);
			
			if(strlen(pwd) > MAXSIZE) {
				fprintf(stderr, "Password too long. Limited to %d octets\n", MAXSIZE);
				continue;
			}
			
			fprintf(stderr, "Enter a password for %s (bis) : ", kKey->name);
			scanf("%s", pwd2);
			
			if(strncmp(pwd, pwd2, MAXSIZE) != 0) {
				fprintf(stderr, "Passwords don't match\n");
				continue;
			}
			
			break;
		}

		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_PLAIN,	strlen(pwd), (BYTE *) pwd);
		burn(pwd, sizeof(pwd))
		burn(pwd2, sizeof(pwd2))
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			
			return 1;
		}
	}

	//Link the policy with the key
	result = Tspi_Policy_AssignToObject(kKey->hPolicy, kKey->hKey);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
		tspiError("Tspi_Policy_AssignToObject", result);
		return 1;
	}

	//Creation of the key
	result = Tspi_Key_CreateKey(kKey->hKey, kKey->kParent->hKey, kKey->hPcrs);
	if(result != TSS_SUCCESS) {
		//IF the parent key is SRK and the wellknown password for SRK fails THEN we ask for a password
		if((kKey->depth == 1) && (kKey->kParent->isWellKnown)) {
			kKey->kParent->isWellKnown = 0;
			
			fprintf(stderr, "Password of SRK : ");
			scanf("%s", pwd);

			result = Tspi_Policy_SetSecret(kKey->kParent->hPolicy, TSS_SECRET_MODE_PLAIN,	strlen(pwd), (BYTE *)pwd);
			burn(pwd, sizeof(pwd))
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
				tspiError("Echec Tspi_Policy_SetSecret", result);
				return 1;
			}
			
			result = Tspi_Key_CreateKey(kKey->hKey, kKey->kParent->hKey, kKey->hPcrs);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
				fprintf(stderr, "(%s) : Fail to use SRK", __FUNCTION__);
				tspiError("Tspi_Key_CreateKey", result);
				return 1;
			}
		} else {
			fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Tspi_Key_CreateKey", result);
			return 1;
		}
	}
	
	//Load the key if they are children
	if(kKey->kChildren) {
		result = Tspi_Key_LoadKey(kKey->hKey, kKey->kParent->hKey);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creation of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Tspi_Key_LoadKey", result);
			return 1;
		}
	}
	
	return 0;
}

//return 0 if success
static int WriteKey(TCG_KEY *kKey) {
	TSS_RESULT		result;
	BYTE 				*key;
	unsigned int	keylen;
	FILE				*fout;
	int 				i;
	int 				res = 0;
	
	//We write the key on the disk
	result = Tspi_GetAttribData(kKey->hKey, TSS_TSPATTRIB_KEY_BLOB, TSS_TSPATTRIB_KEYBLOB_BLOB, &keylen, &key);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During writing %s : \n", __FUNCTION__, kKey->name);
		tspiError("Tspi_GetAttribData", result);
		return 1;
	}

	fout = fopen(kKey->name, "wb");
	if(fout == 0) {
		fprintf(stderr, "(%s) : During writing %s : \n", __FUNCTION__, kKey->name);
		fprintf(stderr, "(%s) : Fail to create file %s\n", __FUNCTION__, kKey->name);
		Tspi_Context_FreeMemory(hContext, key);
		return 1;
	}

	i = 0;
	while(i < keylen) {
		res = fwrite(&key[i], 1, keylen - i, fout);
		if(res == 0) {
			fprintf(stderr, "(%s) : During writing %s : \n", __FUNCTION__, kKey->name);
			fprintf(stderr, "(%s) : Writting error\n", __FUNCTION__);
			break;
		}

		i += res;
	}
	burn(key, keylen)
	fclose(fout);
	Tspi_Context_FreeMemory(hContext, key);
	
	return 0;
}

//Create keys
static void RecCreateArchitecture(TCG_KEY *kKey) {
	TSS_RESULT		result;
	struct stat 	bufStat;
	
	if(kKey == 0)
		return;

	if(kKey->depth) {
		if(stat(kKey->name, &bufStat)) {
			//The key file doesn't exist, so we create it.
		
			//Create PCR structure
			if(CreatePCR(&kKey->hPcrs, kKey->flagsPCR))
				goto error;

			//Create a key and loadd it if there are children
			if(CreateKey(kKey))
				goto error;

			if(WriteKey(kKey))
				goto error;
			
		} else {
			//We load the key from the disk
			if(LoadKeyFromDisk(kKey))
				goto error;
		}
	} else {
		//SRK KEY
		result = Tspi_Context_LoadKeyByUUID(hContext, TSS_PS_TYPE_SYSTEM, SRK_UUID, &kKey->hKey);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creating architecture of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Context_LoadKeyByUUID", result);
			goto end;
		}

		result = Tspi_GetPolicyObject(kKey->hKey, TSS_POLICY_USAGE, &kKey->hPolicy);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creating architecture of %s : \n", __FUNCTION__, kKey->name);			
			tspiError("Echec Tspi_GetPolicyObject", result);
			goto end;
		}

		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During creating architecture of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			goto end;
		}
	}

	//RECURSIVE CALL
	RecCreateArchitecture(kKey->kChildren);

error:
	//IF error, we try to continue with brother keys
	RecCreateArchitecture(kKey->kBrother);
	
end:
	//IF we can't use SRK
	return;
}

/*******************************************************************************************************/
/*******************************************************************************************************/

int main(int argc, char **argv) {
	int 			i = 1;
	char 			*endptr;
	long int		depth;
	int 			flagsPCR = 0;
	TSS_RESULT	result = 0;
	int			modefile = 0;
	int			modeline = 0;
	FILE			*f;
	char			filename[256] = "";
	
	while(i < argc) {
		if(strncmp(argv[i], "-k", sizeof("-k")) == 0) {
			modeline = 1;
			if(i + 3 >= argc) {
				fprintf(stderr, "Syntax error\n");
				usage();
				return 1;
			}
			
			depth = strtol(argv[i + 1], &endptr, 0);
			if(*endptr != 0) {
				fprintf(stderr, "Profondeur invalide : %s\n", argv[i + 1]);
				usage();
				return 1;
			}

			if(parsePCR(argv[i + 3], &flagsPCR)) {
				fprintf(stderr, "Invalid list of PCR : %s\n", argv[i + 3]);
				return 1;
			}

			if(InsertKey(depth, argv[i + 2], 0, flagsPCR)) {
				return 1;
			}

			i += 4;
		} else if(strncmp(argv[i], "-z", sizeof("-z")) == 0) {
			modeline = 1;
			if(i + 3 >= argc) {
				fprintf(stderr, "Syntax error\n");
				usage();
				return 1;
			}
			
			depth = strtol(argv[i + 1], &endptr, 0);
			if(*endptr != 0) {
				fprintf(stderr, "Invalid depth : %s\n", argv[i + 1]);
				return 1;
			}

			if(parsePCR(argv[i + 3], &flagsPCR)) {
				fprintf(stderr, "Invalid list of PCR : %s\n", argv[i + 3]);
				return 1;
			}

			if(InsertKey(depth, argv[i + 2], 1, flagsPCR)) {
				return 1;
			}

			i += 4;
		} else if(strncmp(argv[i], "-f", sizeof("-f")) == 0) {
			modefile = 1;
			if(i + 1 >= argc) {
				fprintf(stderr, "Syntax error\n");
				usage();
				return 1;
			}
			strncpy(filename, argv[i + 1], sizeof(filename));
			
			i += 2;
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

	if(modeline & modefile) {
		usage();
		return 1;
	}
	
	if(modefile) {
		int res = 0;
		
		f = fopen(filename, "r");
		if(f == 0) {
			fprintf(stderr, "Fail to open %s\n", filename);
			return 1;
		}
		//TODO : It is better to use fgets and sscanf
		while(res != EOF) {
			char type[MAXSIZE];
			char name[MAXSIZE];
			char pcrs[MAXSIZE];
			
				
			res = fscanf(f, "%s %ld %s %s", type, &depth, name, pcrs);
			if((res != 4) && (res != EOF)){
				fprintf(stderr, "Syntax error in file %s\n", filename);
				usage();
				fclose(f);
				return 1;
			} else if(res != EOF) {
				//Insert a key in the architecture
				if(parsePCR(pcrs, &flagsPCR)) {
					fprintf(stderr, "Invalid list of PCR : %s\n", pcrs);
					usage();
					fclose(f);
					return 1;
				}
				
				if(strncmp(type, "-k", sizeof("-k")) == 0) {
					if(InsertKey(depth, name, 0, flagsPCR)) {
						fclose(f);
						return 1;
					}
				} else if(strncmp(type, "-z", sizeof("-z")) == 0) {
					if(InsertKey(depth, name, 1, flagsPCR)) {
						fclose(f);
						return 1;
					}
				} else {
					fprintf(stderr, "Syntax error in file %s\n", filename);
					usage();
					return 1;
				}
				
			}
		}
		
		fclose(f);
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

	//Get the handle of the TPM
	result = Tspi_Context_GetTpmObject(hContext, &hTpm);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_GetTpmObject", result);
		result = 1;
		goto end;
	}

	//Build key architecture
	RecCreateArchitecture(&kSRK);		

end:
	//Close TSS session
	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);

	return result;
}

