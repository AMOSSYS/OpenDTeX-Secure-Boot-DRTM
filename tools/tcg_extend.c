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

//TODO Extend with a file

#define SIZEMAX 256
char filename[SIZEMAX] = "";


void usage() {
	fprintf(stderr, "Usage : extend -p <index> [-o file]\n");
}

int extend(int iPcr) {
	TSS_RESULT		result; 
	BYTE 			wellKnown[TCPA_SHA1_160_HASH_LEN] = TSS_WELL_KNOWN_SECRET;
	UINT32 			pcrSize;
	BYTE 			*pcrValue;

	result = Tspi_TPM_PcrExtend(hTpm, iPcr, sizeof(wellKnown), wellKnown, 0, &pcrSize, &pcrValue);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_TPM_PcrExtend", result);
		return 1;
	}

	Tspi_Context_FreeMemory(hContext, pcrValue);
	return 0;
}

int main(int argc, char **argv) {
	int 			i = 1;
	int 			iPcr = 0;
	TSS_RESULT 		result; 

	while(i < argc) {
		if(strncmp(argv[i], "-o", sizeof("-o")) == 0) {
			i++;
			if(i >= argc) {
				usage();
				return 1;
			}

			strncpy(filename, argv[i], sizeof(filename));
		} else if(strncmp(argv[i], "-p", sizeof("-p")) == 0) {
			i++;
			if(i >= argc) {
				usage();
				return 1;
			}

			iPcr = atoi(argv[i]);
			if((iPcr < 0) || (iPcr > NBMAX_PCR)) {
				fprintf(stderr, "ERREUR : index of PCR are between 0 and %d\n", NBMAX_PCR - 1);
				return 1;
			}
			
		} else {
			usage();
			return 1;
		}

		i++;
	}
	
	//Creation of the TSS Context
	result = Tspi_Context_Create(&hContext);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Create", result);
		return 1;
	}

	//Connection to the TSS Context
	result = Tspi_Context_Connect(hContext, NULL);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Connext", result);
		return 1;
	}

	//Get the handle of the TPM
	result = Tspi_Context_GetTpmObject(hContext, &hTpm);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_GetTpmObject", result);
		result = 1;
		goto end;
	}

	result = extend(iPcr);

end:
	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);
	
	return result;
}





