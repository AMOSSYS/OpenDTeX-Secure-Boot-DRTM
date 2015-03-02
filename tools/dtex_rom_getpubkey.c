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

#define ROM_GUID 16
#define ROM_TPMKEYSIZE 768
#define ROM_SEALEDKEYSIZE 512
#define ROM_SECRETTEXTSIZE 256
#define ROM_IVSIZE 32

typedef struct {
   uint8_t  guid[ROM_GUID];
   uint8_t  tpmkey_blob[ROM_TPMKEYSIZE];
   uint8_t  tpmsignkey_blob[ROM_TPMKEYSIZE];
   uint8_t  sealedkey_blob[ROM_SEALEDKEYSIZE];
   uint8_t  iv[ROM_IVSIZE];
   uint8_t  secrettext_blob[ROM_SECRETTEXTSIZE];
   uint32_t tpmkey_size;
   uint32_t tpmsignkey_size;
   uint32_t sealedkey_size;
   uint32_t secrettext_size;
} rom_hdr_t;

void usage(void) {
   fprintf(stderr, "Usage: dtex_rom_getpubkey [-f <rom file>]\n");
}

int main(int argc, char ** argv) {
   FILE *      file;
   size_t      res;
   rom_hdr_t   hdr;
   TSS_RESULT	result;
   char        pathrom[256] = "";
   TSS_HKEY	   hSRK;
   TSS_HPOLICY	pSRK;
   TSS_HKEY	   hKey;
   BYTE *      pubInfo;
   UINT32      pubInfoSize;
   int         i;
   
   for(i = 1 ; i < argc ; i++) {
      if(strncmp("-f", argv[i], sizeof("-f")) == 0) {
         if(i + 1 >= argc) {
            usage();
            return 1;
         }
         
         snprintf(pathrom, sizeof(pathrom), "%s", argv[i + 1]);
         pathrom[sizeof(pathrom) - 1] = 0;
         i++;
      } else {
         usage();
         return 1;
      }
   }
   
   if(pathrom[0] == 0) {
      char * home=getenv("HOME");
      if(home == NULL) {
         fprintf(stderr, "No $HOME environment variable defined\n");
         return 1;
      }
      
      snprintf(pathrom, sizeof(pathrom), "%s/.dtex/rom.bin", home);
      pathrom[sizeof(pathrom) - 1] = 0;
   }
   
   file = fopen(pathrom, "rb");
   if(file == NULL) {
      fprintf(stderr, "Can't open rom file\n");
      return 1;
   }
   
   res = fread(&hdr, sizeof(hdr), 1, file);
   if(res != 1) {
      fprintf(stderr, "Can't read rom file\n");
      fclose(file);
      return 1;
   }
   
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
	
	result = Tspi_SetAttribData(hKey, TSS_TSPATTRIB_KEY_BLOB, TSS_TSPATTRIB_KEYBLOB_BLOB, hdr.tpmsignkey_size, hdr.tpmsignkey_blob);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_SetAttribData", result);
		res = 1;
      goto err;
	}
	
	result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_KEY_BLOB, TSS_TSPATTRIB_KEYBLOB_PUBLIC_KEY, &pubInfoSize, &pubInfo);
	if(result != TSS_SUCCESS) {
		tspiError("Echec Tspi_GetAttribData", result);
		res = 1;
      goto err;
	}
	
	if(fwrite(pubInfo, pubInfoSize, 1, stdout) != 1) {
	   fprintf(stderr, "Write error\n");
		res = 1;
      goto err;
	}
	
	result = Tspi_Context_FreeMemory(hContext, pubInfo);
	if(result != TSS_SUCCESS) {
	   tspiError("Echec Tspi_Context_FreeMemory", result);
		res = 1;
      goto err;
	}
	
	res = 0;
	
err:
   //Close TSS session
	Tspi_Context_FreeMemory(hContext, NULL);
	Tspi_Context_Close(hContext);
	
	return res;
}

