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


/**
 * \file command_banner.c
 * \brief Implementation of banner command processing.
 * \author Goulven Guiheux
 *
 * Implementation of banner command processing. Currently supported banner types are:
 * - graphic (bmp);
 * - text.
 */

#include <uc/uc.h>
#include <libtpm/libtpm.h>
#include <uvideo/uvideo.h>
#include <tloader.h>
#include <command.h>
#include <aes.h>
#include <memory.h>
#include <kbd.h>
#include <log.h>

typedef enum {
   BANNER_TEXT,
   BANNER_IMAGE,
   BANNER_SOUND
} BANNER_TYPE;

/**
 * \fn void command_banner(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv)
 * \brief Function that implements the banner processing. A banner can be either graphical or textual.
 *
 * \param module_addr
 * \param module_size
 * \param module_name
 * \param argc
 * \param argv
 */
void command_banner(uint8_t * module_addr, uint32_t module_size, char * module_name, int32_t argc, char ** argv) {
   uint8_t *   bufProtectedData = module_addr;
   size_t      lenProtectedData = module_size;
   uint8_t *   bufSealedData;
   uint32_t    lenSealedData;
   uint8_t *   data;
   uint32_t    lenData;
   TSS_RESULT  result;
   char        pwdKey[256];
   char        pwdData[256];
   uint8_t *   key;
   uint32_t    keySize;
   AES_KEY     aeskey;
   uint8_t     iv[AES_BLOCK_SIZE];
   char        padding;
   char        i;
   BANNER_TYPE typeBanner;
   uint32_t    lenBanner;
   uint8_t *   dataBanner;
   
   if(argc != 1) {
      LOGERROR("!! cmd_banner: bad parameters !!\n");
      return;
   }
   
   lenSealedData = *((uint32_t *) &bufProtectedData[0]);
   bufSealedData = &bufProtectedData[sizeof(uint32_t)];
   
   data = malloc(lenProtectedData - lenSealedData - sizeof(UINT32));
   if(data == 0) {
      LOGERROR("Error: not enough memory\n");
      return;
   }
   
   prompt_password("Enter password of parent key:", pwdKey, sizeof(pwdKey));
   prompt_password("Enter password of sealed data:", pwdData, sizeof(pwdData));
   
   result = TPM_Unseal(TLOADER_LOCALITY, &key, &keySize, bufSealedData, lenSealedData, argv[0], pwdKey, sizeof(pwdKey), pwdData, sizeof(pwdData));

   memset(pwdKey, 0, sizeof(pwdKey));
   memset(pwdData, 0, sizeof(pwdData));
   
   if (result != TSS_SUCCESS) {
      free(data);
      return; 
   }
   
   AES_set_decrypt_key(key, keySize * 8, &aeskey);
   memset(iv, 0, sizeof(iv));
   
   AES_cbc_encrypt(&bufSealedData[lenSealedData], data, lenProtectedData - lenSealedData - sizeof(uint32_t), &aeskey, iv, AES_DECRYPT);

   memset(&aeskey, 0, sizeof(aeskey));
   memset(key, 0, keySize);
   free(key);
   
   lenData = lenProtectedData - lenSealedData - sizeof(uint32_t);
   
   padding = data[lenData - 1];
   if(padding > AES_BLOCK_SIZE) {
      LOGERROR("Error : AES padding value too big\n");
      memset(data, 0, lenProtectedData - lenSealedData - sizeof(uint32_t));
      free(data);
      
      return;
   }
   
   for(i = 0 ; i < padding ; i++) {
      if(data[lenData - i - 1] != padding) {
         LOGERROR("Error : AES padding error\n");
         memset(data, 0, lenProtectedData - lenSealedData - sizeof(uint32_t));
         free(data);
         
         return;
      }
   }
   
   lenData -= padding;
   
   if(lenData <= 8) {
      LOGERROR("Error: sealed data are too small for a banner structure\n");
      memset(data, 0, lenProtectedData - lenSealedData - sizeof(uint32_t));
      free(data);
         
      return;
   }
   
   typeBanner = *((uint32_t *) data);
   lenBanner = *(((uint32_t *) data) + 1);
   dataBanner = data + 2 * sizeof(uint32_t);
   
   if(lenData != lenBanner + 2 * sizeof(UINT32)) {
      LOGERROR("Error: length error in the banner structure\n");
      memset(data, 0, lenProtectedData - lenSealedData - sizeof(uint32_t));
      free(data);
         
      return;
   }
   
   // Interpret banner
   switch (typeBanner) {
   case BANNER_TEXT:
      uvideo_cls();
      uvideo_printf("!! Secured banner %s will be displayed !!\n", module_name);
      uvideo_printf("<press a key to continue>\n");
      
      kbd_getc();
      uvideo_cls();
      
      uvideo_printf("**************************\n");
      dataBanner[lenBanner - 1] = 0;
      uvideo_printf("%s\n", dataBanner);
      uvideo_printf("**************************\n");
      
      kbd_getc();
      uvideo_cls();
      break;
      
   case BANNER_IMAGE:
      uvideo_cls();
      uvideo_printf("!! Secured banner %s will be displayed !!\n", module_name);
      uvideo_printf("<press a key to continue>\n");
      
      kbd_getc();
      uvideo_cls();
      
      uvideo_display(dataBanner, lenBanner);

      kbd_getc();
      uvideo_cls();
      break;
      
   case BANNER_SOUND:
      uvideo_printf("Unsealed data is type sound (NOT IMPLEMENTED)\n");
      break;
      
   default:
      uvideo_printf("Error: unknown banner type (%u)\n", typeBanner);
      break;
   }
   
   //Free memory
   memset(data, 0, lenProtectedData - lenSealedData - sizeof(uint32_t));
   free(data);   
}

