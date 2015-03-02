// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS, Bertin
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


#include <uc/types.h>
#include <uc/uc.h>
#include <uvideo/uvideo.h>
#include <libtpm/libtpm.h>
#include <command.h>
#include <multiboot.h>
#include <kbd.h>
#include <aes.h>
#include <stub.h>

extern bool is_tpm_ready(uint32_t locality);
typedef uint32_t tpm_nv_index_t;
extern uint32_t tpm_nv_read_value(uint32_t locality, tpm_nv_index_t index,
                                  uint32_t offset, uint8_t *data,
                                  uint32_t *data_size);

#define TPM_NV_SECKEY_INDEX    0x20000004
#define TPM_NV_KEYPAIR_INDEX   0x20000005

static char * keypair_name = (char *)"Key Pair Unsealing";

uint8_t seckey_seal[400];
uint32_t seckey_size = 400;
uint8_t keypair_seal[600];
uint32_t keypair_size = 600;

static int decrypt_kernel(uint8_t * module_addr, uint32_t module_size) {
   unsigned int i;
   uint32_t     size_read = 0;
   AES_KEY      key;
   uint32_t     kern_size = module_size;
   uint32_t     encrypt_size = 0;
   uint32_t     block_size = 4096;
   uint8_t      IV[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
   unsigned char * in = NULL;
   BYTE * data_seal = NULL;
   uint32_t data_size = 40;

   if(!is_tpm_ready(0)){
      uvideo_printf("TPM is disable !!\r\n");
      return 0;
   }

   if(TPM_SUCCESS != tpm_nv_read_value(TPM_LOC_ZERO,TPM_NV_KEYPAIR_INDEX,
                                       0, keypair_seal,
                                       &keypair_size)){
      uvideo_printf("Reading TPM NV RAM (data size) failed !!\r\n");
      return 0;
   }

   memcpy((void *)&size_read , keypair_seal , 4);
   
   if(TPM_SUCCESS != tpm_nv_read_value(TPM_LOC_ZERO,TPM_NV_KEYPAIR_INDEX,
                                       5, keypair_seal,
                                       &size_read)){
      uvideo_printf("Reading TPM NV RAM (data) failed !!\r\n");
      return 0;
   }

   if(TSS_SUCCESS != TPM_LoadKey(TPM_LOC_ZERO,
                                 (BYTE *)keypair_seal,size_read,
                                 keypair_name,(char *)"SRK",NULL,0)){
      uvideo_printf("Loading Key Pair to unseal secret key failed !!\r\n");
      return 0;
   }

   if(TPM_SUCCESS != tpm_nv_read_value(TPM_LOC_ZERO,TPM_NV_SECKEY_INDEX,
                                       0, seckey_seal,
                                       &seckey_size)){
      uvideo_printf("Reading TPM NV RAM (data size) failed !!\r\n");
      TPM_UnloadKey(TPM_LOC_ZERO,keypair_name);
      return 0;
   }

   memcpy((void *)&size_read , seckey_seal , 4);

   if(TPM_SUCCESS != tpm_nv_read_value(TPM_LOC_ZERO,TPM_NV_SECKEY_INDEX,
                                       5, seckey_seal,
                                       &size_read)){
      uvideo_printf("Reading TPM NV RAM (data) failed !!\r\n");
      TPM_UnloadKey(TPM_LOC_ZERO,keypair_name);
      return 0;
   }

   if(TSS_SUCCESS != TPM_Unseal(TPM_LOC_ZERO,
                                &data_seal,&data_size,
                                seckey_seal,size_read,
                                keypair_name,
                                NULL,0,NULL,0)){
      uvideo_printf("Unsealing secret key failed !!\r\n");
      TPM_UnloadKey(TPM_LOC_ZERO,keypair_name);
      return 0;
      }

   TPM_UnloadKey(TPM_LOC_ZERO,keypair_name);

   uvideo_printf("Decrypting kernel ...\n");

   if(AES_set_decrypt_key(data_seal,data_size*8,&key)){
      uvideo_printf("AES_set_decrypt_key failed !!\r\n");
      stub_free(data_seal);
      return 0;
   }

   stub_free(data_seal);

   // Decypher kernel
   in = (unsigned char *)module_addr;
   //      uvideo_printf("Kernel size = %d - start adress = %p\n",kern_size,in);
   i = 0;
   do{
      //              uvideo_printf("1 - block_size = %d\n",block_size);
      AES_cbc_encrypt(in, in,	block_size, &key, IV, AES_DECRYPT);

      i +=  block_size;
      in += block_size;

      encrypt_size += block_size;

      if( (kern_size - encrypt_size) < block_size)
         block_size = kern_size - encrypt_size;
      //                uvideo_printf("2 - block_size = %d, encrypt_size = %d\n",
      //                             block_size,encrypt_size);
   } while(encrypt_size < kern_size);


   in -= i;

   uvideo_printf("Kernel decrypted.");

   return 1;
}

/**
 * Launch a kernel sealed with the TPM
 */
void command_sealedkernel32(uint8_t * module_addr, uint32_t module_size,
                        char * module_name,
                        int32_t argc, char ** argv) {
   int idx = 0;

   uvideo_printf("command_sealedkernel : '%s' mod_start: 0x%x, mod_end: 0x%x\r\n",
                module_name, module_addr, module_addr + module_size);

   uvideo_printf("command_sealedkernel : cmd line '");
   for(idx=0; idx<argc; idx++) uvideo_printf("%s ", argv[idx]);
   uvideo_printf("'\r\n");

   decrypt_kernel(module_addr, module_size);

   command_kernel32(module_addr, module_size, module_name, argc, argv);

   return;
}

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 3
 * tab-width: 3
 * indent-tabs-mode: nil
 * End:
 */
