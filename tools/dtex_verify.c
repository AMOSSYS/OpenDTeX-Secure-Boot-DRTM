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
#include <stdlib.h>
#include <string.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>
#include "libtcg.h"

#define MAXDATASIZE 8192

void usage(void) {
   fprintf(stderr, "Usage: dtex_verify [-f <data file>] -k <tpm public key> -s <signature file>\n");
}

int main(int argc, char ** argv) {
   int         i;
   BYTE        keyblob[1024];
   size_t      keyblobSize = 0;
   char        keypathfile[256] = "";
   BYTE        sig[1024];
   size_t      sigSize = 0;
   char        sigpathfile[256] = "";
   BYTE        data[MAXDATASIZE];
   size_t      dataSize = 0;
   char        datapathfile[256] = "";
   FILE *      file;
   size_t      res;
   TSS_RESULT	result;
   TSS_HKEY	   hKey;
   TSS_HHASH   hHash;
   
   for(i = 1 ; i < argc ; i++) {
      if(strncmp("-f", argv[i], sizeof("-f")) == 0) {
         if(i + 1 >= argc) {
            usage();
            return 1;
         }
         
         snprintf(datapathfile, sizeof(datapathfile), "%s", argv[i + 1]);
         datapathfile[sizeof(datapathfile) - 1] = 0;
         i++;
      } else if(strncmp("-k", argv[i], sizeof("-k")) == 0) {
         if(i + 1 >= argc) {
            usage();
            return 1;
         }
         
         snprintf(keypathfile, sizeof(keypathfile), "%s", argv[i + 1]);
         keypathfile[sizeof(keypathfile) - 1] = 0;
         i++;
      } else if(strncmp("-s", argv[i], sizeof("-s")) == 0) {
         if(i + 1 >= argc) {
            usage();
            return 1;
         }
         
         snprintf(sigpathfile, sizeof(sigpathfile), "%s", argv[i + 1]);
         sigpathfile[sizeof(sigpathfile) - 1] = 0;
         i++;
      } else {
         fprintf(stderr, "Invalid option %s\n", argv[i]);
         usage();
         return 1;
      }
   }
   
   if(sigpathfile[0] == 0) {
      usage();
      return 1;
   }
   
   if(keypathfile[0] == 0) {
      usage();
      return 1;
   }
   
   file = fopen(keypathfile, "rb");
   if(file == NULL) {
      fprintf(stderr, "Can't open key file\n");
      return 1;
   }
   
   res = fread(&keyblob, 1, sizeof(keyblob), file);
   if(res < 0) {
      fprintf(stderr, "Can't read key file\n");
      fclose(file);
      return 1;
   }
   keyblobSize = res;
   fclose(file);
   
   file = fopen(sigpathfile, "rb");
   if(file == NULL) {
      fprintf(stderr, "Can't open key file\n");
      return 1;
   }
   
   res = fread(&sig, 1, sizeof(sig), file);
   if(res < 0) {
      fprintf(stderr, "Can't read sig file\n");
      fclose(file);
      return 1;
   }
   sigSize = res;
   fclose(file);
   
   if(datapathfile[0] == 0) {
      file = stdin;
   } else {
      file = fopen(datapathfile, "rb");
      if(file == NULL) {
         fprintf(stderr, "Can't open data file\n");
         return 1;
      }
   }
   
   res = fread(&data, 1, sizeof(data), file);
   if(res < 0) {
      fprintf(stderr, "Can't read data file\n");
      fclose(file);
      return 1;
   }
   dataSize = res;
   fclose(file);
   
   //Creation of the TSS Context
	result = Tspi_Context_Create(&hContext);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Create", result);
		res = 1;
      goto err;
	}

	//Connection to the TSS Context
	result = Tspi_Context_Connect(hContext, NULL);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_Connext", result);
		Tspi_Context_FreeMemory(hContext, NULL);
	   Tspi_Context_Close(hContext);
		res = 1;
      goto err;
	}
	
	// Create object key in order to put the public key
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_RSAKEY, TSS_KEY_EMPTY_KEY, &hKey);
   if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_LoadKeyByBlob", result);
		res = 1;
      goto err;
	}
	
	result = Tspi_SetAttribData(hKey, TSS_TSPATTRIB_KEY_BLOB, TSS_TSPATTRIB_KEYBLOB_PUBLIC_KEY, keyblobSize, keyblob);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_SetAttribData", result);
		res = 1;
      goto err;
	}
	
	// Create object hash
	result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_HASH, TSS_HASH_SHA1, &hHash);
   if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_LoadKeyByBlob", result);
		res = 1;
      goto err;
	}
	
	result = Tspi_Hash_UpdateHashValue(hHash, dataSize, data);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Context_LoadKeyByBlob", result);
		res = 1;
      goto err;
	}
	
	result = Tspi_Hash_VerifySignature(hHash, hKey, sigSize, sig);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_Hash_VerifySignature", result);
		res = 1;
      goto err;
	}
   
   printf("Signature OK\n");
   
   res = 0;
   
err:
   //Close TSS session
	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);
	
	return res;
}

