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
#include <trousers/tss.h>
#include <trousers/trousers.h>
#include "libtcg.h"

TCG_KEY 	kSRK = {0, 0, 0, 0, 0, 0, 0, 1, 0, "SRK"};
TCG_KEY	*currentKey = &kSRK;

TSS_HCONTEXT hContext = 0;
TSS_HTPM hTpm = 0;
BYTE wellKnown[TCPA_SHA1_160_HASH_LEN] = TSS_WELL_KNOWN_SECRET;
TSS_UUID SRK_UUID = TSS_UUID_SRK;

/*******************************************************************************************************/
/*******************************************************************************************************/

void tspiError(const char *a_szName, TSS_RESULT a_iResult) {
	fprintf(stderr, "%s failed: 0x%08x - layer=%s, code=%04x (%d), %s\n",
		 a_szName, a_iResult, Trspi_Error_Layer(a_iResult),
		 Trspi_Error_Code(a_iResult),
		 Trspi_Error_Code(a_iResult),
		 Trspi_Error_String(a_iResult));
}


long getTaille(FILE *f) {
	long current;
	long taille;

	if(f == 0)
		return 0;
	
	current = ftell(f);

	fseek(f, 0, SEEK_END);
	taille = ftell(f);
	fseek(f, 0, SEEK_SET);
	taille = taille - ftell(f);

	fseek(f, current, SEEK_SET);

	return taille; 
}


void dump(BYTE *p, UINT64 len) {
	UINT64 i = 0;
	UINT64 j = 0;

	if(p == 0)
		return;

	while(i < len) {
		for(j = 0 ; j < 16 ; j++) {
			if(i + j < len) {	
				printf("%02x ", p[j]);
			} else {
				printf("   ");
			}
		}

		printf(":");

		for(j = 0 ; j < 16 ; j++) {
			if(i + j < len) {		
				if(p[j] < 0x20) {
					printf(".");				
				} else {
					printf("%c", p[j]);
				}
			} else {
				printf(" ");
			}
		}
		printf("\n");

		i += j;
		p += j;
	}
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//return 0 if success
int parsePCR(char *list, int *flagsPCR) {
	char 		*endptr;
	long int	val;

	*flagsPCR = 0;
	
	if(*list == ':')
		return 0;

	do {
		val = strtol(list, &endptr, 0);
		if(list == endptr)
			return 1;

		if((0 <= val) && (val < NBMAX_PCR)) {
			*flagsPCR |= (1 << val);
		} else {
			return 1;
		}

		if(*endptr == ':') {
			list = endptr + 1;
			continue;
		}

		if(*endptr != 0)
			return 1;
	} while(*endptr != 0);

	return 0;
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//Insert a key in the key architecture. This key becomes the current key
//return 0 if success
int InsertKey(long int depth, char *name, int isWellKnown, int flagsPCR) {
	TCG_KEY *kKey;

	if(currentKey->depth + 1 == depth) {
		//Insertion of key in the children of the current key
		kKey = malloc(sizeof(TCG_KEY));
		if(kKey == 0) {
			fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
			fprintf(stderr, "(%s) : out of memory\n", __FUNCTION__);
			return 1;
		}

		//Init the key
		kKey->kParent = currentKey;
		kKey->kChildren = 0;
		kKey->kBrother = 0;
		kKey->hKey = 0;
		kKey->hPolicy = 0;
		kKey->hPcrs = 0;
		kKey->depth = depth;
		kKey->isWellKnown = isWellKnown;
		kKey->flagsPCR = flagsPCR;
		strncpy(kKey->name, name, MAXSIZE);
		
		
		//Become a children of the current key
		currentKey->kChildren = kKey;
		
		//Become the current key
		currentKey = kKey;


	} else if(currentKey->depth == depth) {
		//Insertion of brother key
		if(depth == 0) {
			fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
			fprintf(stderr, "(%s) : error, trying to create a key a the same level of SRK !\n", __FUNCTION__);
			return 1;
		}

		kKey = malloc(sizeof(TCG_KEY));
		if(kKey == 0) {
			fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
			fprintf(stderr, "(%s) : error, out of memory\n", __FUNCTION__);
			return 1;
		}

		//Init the key
		kKey->kParent = currentKey->kParent;
		kKey->kChildren = 0;
		kKey->kBrother = 0;
		kKey->hKey = 0;
		kKey->hPolicy = 0;
		kKey->hPcrs = 0;
		kKey->depth = currentKey->depth;
		kKey->isWellKnown = isWellKnown;
		kKey->flagsPCR = flagsPCR;
		strncpy(kKey->name, name, MAXSIZE);
		
		//Become a brother of the current key
		currentKey->kBrother = kKey;
		
		//Become the current key
		currentKey = kKey;
	} else if(currentKey->depth > depth){
		//Insertion of a brother key at a lesser depth
		if(depth == 0) {
			fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
			fprintf(stderr, "(%s) : error, trying to create a key a the same level of SRK !\n", __FUNCTION__);
			return 1;
		}

		while(currentKey->depth != depth) {
			currentKey = currentKey->kParent;
		}
		
		//Insertion of a brother
		kKey = malloc(sizeof(TCG_KEY));
		if(kKey == 0) {
			fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
			fprintf(stderr, "(%s) : error, out of memory\n", __FUNCTION__);
			return 1;
		}

		//Init the key
		kKey->kParent = currentKey->kParent;
		kKey->kChildren = 0;
		kKey->kBrother = 0;
		kKey->hKey = 0;
		kKey->hPolicy = 0;
		kKey->hPcrs = 0;
		kKey->depth = currentKey->depth;
		kKey->isWellKnown = isWellKnown;
		kKey->flagsPCR = flagsPCR;
		strncpy(kKey->name, name, MAXSIZE);
		
		//Become a brother of the current key
		currentKey->kBrother = kKey;
		
		//Become the current key
		currentKey = kKey;

	} else {
		//Invalid insertion
		fprintf(stderr, "(%s) : During insertion of %s : \n", __FUNCTION__, name);
		fprintf(stderr, "(%s) : error, invalid insertion\n", __FUNCTION__);
		return 1;
	}

	return 0;
}

//Display architecture
void RecPrintArchitecture(TCG_KEY *kKey) {
	if(kKey == 0)
		return;

	if(kKey->depth)
		fprintf(stderr, "%*c(%04ld)-> %s(WellKnown : %d)(%08x) \t| child of %s\n", (int) kKey->depth * 3, ' ', kKey->depth, kKey->name, kKey->isWellKnown, kKey->flagsPCR, kKey->kParent->name);
	else
		fprintf(stderr, "(%04ld)-> %s(WellKnown : %d)\n", kKey->depth, kKey->name, kKey->isWellKnown);

	RecPrintArchitecture(kKey->kChildren);
	RecPrintArchitecture(kKey->kBrother);
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//return 0 if success
int CreatePCR(TSS_HPCRS *hPcrs, int flagsPCR) {
	TSS_FLAG 	initFlag = 0;
	TSS_RESULT	result;
	int 			i;
	
	*hPcrs = 0;
	
	//IF registers PCR16-23 are used THEN we must use long structure
	for(i = 16 ; i < NBMAX_PCR ; i++) {
		if(flagsPCR & (1 << i)) {
			initFlag |= TSS_PCRS_STRUCT_INFO_LONG;	
		}
	}

	//Create PCR structure
	for(i = 0 ; i < NBMAX_PCR ; i++) {
		UINT32 pcrSize;
		BYTE *pcrValue;

		//PCR  i selected ?
		if(flagsPCR & (1 << i)) {

			//Create PCR blob if it doesn't exist
			if(*hPcrs == 0) {
				result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_PCRS, initFlag, hPcrs);
				if(result != TSS_SUCCESS) {
					fprintf(stderr, "(%s) : During generation of PCR\n", __FUNCTION__);
					tspiError("Echec Tspi_Context_CreateObject", result);
					return 1;
				}
			}

			result = Tspi_TPM_PcrRead(hTpm, i, &pcrSize, &pcrValue);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During generation of PCR\n", __FUNCTION__);
				tspiError("Echec Tspi_TPM_PcrRead", result);
				return 1;
			}

			result = Tspi_PcrComposite_SetPcrValue(*hPcrs, i, pcrSize, pcrValue);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During generation of PCR\n", __FUNCTION__);
				tspiError("Echec Tspi_PcrComposite_SetPcrValue", result);
				Tspi_Context_FreeMemory(hContext, pcrValue);
				return 1;
			}

			Tspi_Context_FreeMemory(hContext, pcrValue);
		}
	}

	if(initFlag) {
		UINT32 localityValue = TPM_LOC_ZERO | TPM_LOC_ONE | TPM_LOC_TWO | TPM_LOC_THREE | TPM_LOC_FOUR;
		result =	Tspi_PcrComposite_SetPcrLocality(*hPcrs, localityValue);			
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During generation of PCR\n", __FUNCTION__);
			tspiError("Echec Tspi_PcrComposite_SetPcrLocality", result);
			return 1;
		}
	}
	
	return 0;
}

/*******************************************************************************************************/
/*******************************************************************************************************/

//return 0 if success
int LoadKeyFromDisk(TCG_KEY *kKey) {
	TSS_RESULT		result;
	BYTE 				*key;
	unsigned int	keylen;
	char 				pwd[MAXSIZE];
	FILE				*fin;
	int 				i;
	int 				res = 0;

	fin = fopen(kKey->name, "rb");
	if(fin == 0) {
		fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
		fprintf(stderr, "(%s) : Fail to read file %s\n", __FUNCTION__, kKey->name);
		return 1;
	}

	keylen = getTaille(fin);
	key = malloc(keylen);
	if((long) key == 0) {
		fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
		fprintf(stderr, "(%s) : error, out of memory\n", __FUNCTION__);
		fclose(fin);
		return 1;
	}

	/* Read the key */
	i = 0;
	while(i < keylen) {
		res = fread(&key[i], 1, keylen - i, fin);
		if(res == 0) {
			break;
		}

		i += res;
	}
	if(keylen != i) {
		fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
		fprintf(stderr, "(%s) : error, read error\n", __FUNCTION__);
		fclose(fin);
		free(key);
		return 1;
	}

	//Load the key
	result = Tspi_Context_LoadKeyByBlob(hContext, kKey->kParent->hKey, keylen, key, &kKey->hKey);
	if(result != TSS_SUCCESS) {
		//IF the parent key is SRK and the wellknown password is used for SRK THEN we ask for a password
		if(kKey->kParent->isWellKnown) {
			kKey->kParent->isWellKnown = 0;
		
			fprintf(stderr, "Password of %s : ", kKey->kParent->name);
			scanf("%s", pwd);

			result = Tspi_Policy_SetSecret(kKey->kParent->hPolicy, TSS_SECRET_MODE_PLAIN,	strlen(pwd), (BYTE *)pwd);
			burn(pwd, sizeof(pwd))
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
				tspiError("Echec Tspi_Policy_SetSecret", result);	
				free(key);
				return 1;
			}
		
			result = Tspi_Context_LoadKeyByBlob(hContext, kKey->kParent->hKey, keylen, key, &kKey->hKey);
			if(result != TSS_SUCCESS) {
				fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
				tspiError("Tspi_Context_LoadKeyByBlob", result);
				free(key);
				return 1;
			}
		} else {
			fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Tspi_Context_LoadKeyByBlob", result);
			free(key);
			return 1;
		}
	}

	free(key);

	//We set the authentification secret
	result = Tspi_Context_CreateObject(hContext,	TSS_OBJECT_TYPE_POLICY,	TSS_POLICY_USAGE,	&kKey->hPolicy);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
		tspiError("Echec Tspi_Context_CreateObject", result);
		return 1;
	}

	if(kKey->isWellKnown) {
		//Use of the WellKnown password
		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_SHA1, sizeof(wellKnown), wellKnown);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			return 1;
		}
	} else {
		//Ask for a password
		fprintf(stderr, "Password of %s : ", kKey->name);
		scanf("%s", pwd);

		result = Tspi_Policy_SetSecret(kKey->hPolicy, TSS_SECRET_MODE_PLAIN,	strlen(pwd), (BYTE *) pwd);
		if(result != TSS_SUCCESS) {
			fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
			tspiError("Echec Tspi_Policy_SetSecret", result);
			burn(pwd, sizeof(pwd))
			return 1;
		}
	}

	result = Tspi_Policy_AssignToObject(kKey->hPolicy, kKey->hKey);
	if(result != TSS_SUCCESS) {
		fprintf(stderr, "(%s) : During loading of %s : \n", __FUNCTION__, kKey->name);
		tspiError("Tspi_Policy_AssignToObject", result);
		return 1;
	}
	
	return 0;
}

