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


#include <uc/uc.h>
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "crypto/sha.h"
#include "core.h"

static UINT64 get_tsc(void);
static TSS_RESULT Feed(UINT32 locality);

#define SIZEPOOL SHA1HashSize

static UINT64 get_tsc(void) {
	UINT32 lo = 0;
	UINT32 hi = 0;

	__asm__ __volatile__  ("rdtsc":"=a" (lo), "=d" (hi));

	return (((UINT64) hi) << 32) | lo;
}

static BYTE internalpool[SIZEPOOL];
static BYTE externalpool[SIZEPOOL * 512];
static UINT32 cptread = 0;
static UINT32 init = 0;

static TSS_RESULT Feed(UINT32 locality) {
   TSS_RESULT           result;
   BYTE                 tpmbuffer[SIZEPOOL];
   BYTE                 cpubuffer[SIZEPOOL];
   BYTE 		            sha[SIZEPOOL];
   UINT32               i;
   struct USHAContext   ctx;
   
   LOGDEBUG("Feed random pool");
   
   result = TCS_GetRandom(locality, tpmbuffer, sizeof(tpmbuffer));
   if(result != TSS_SUCCESS) {
      LOGERROR("TCS_GetRandom returns %x", result);
      return result;
   }
   
   for(i = 0 ; i < sizeof(internalpool) / sizeof(UINT32) ; i++) {
      ((UINT32 *) internalpool)[i] ^= ((UINT32 *) tpmbuffer)[i];
      ((UINT32 *) cpubuffer)[i] ^= get_tsc();
   }
   
   USHAReset(&ctx, SHA1);
	USHAInput(&ctx, cpubuffer, sizeof(cpubuffer));
	USHAResult(&ctx, sha);
	
	/*result = TCS_StirRandom(locality, sha, SIZEPOOL);
	if(result != TSS_SUCCESS) {
      LOGERROR("TCS_StirRandom returns %x", result);
      return result;
   }*/
   
   for(i = 0 ; i < sizeof(internalpool) / sizeof(UINT32) ; i++) {
      ((UINT32 *) sha)[i] ^= ((UINT32 *) internalpool)[i];
   }
   
   USHAReset(&ctx, SHA1);
	USHAInput(&ctx, sha, sizeof(sha));
	USHAResult(&ctx, sha);
	
	// ...
	for(i = 0 ; i < sizeof(externalpool) / sizeof(UINT32) ; i++) {
      ((UINT32 *) externalpool)[i] = ((UINT32 *) sha)[i % (sizeof(sha) / sizeof(UINT32))];
      
      if(i && ((i % (sizeof(sha) / sizeof(UINT32))) == 0)) {
         USHAReset(&ctx, SHA1);
	      USHAInput(&ctx, sha, sizeof(sha));
	      USHAResult(&ctx, sha);
      }
   }
	
	LOGDEBUG("End feeding");
	
	return TSS_SUCCESS;
}

TSS_RESULT LIBTPMCALL LIBTPM_GetRand(UINT32 locality __attribute__((unused)), BYTE * buf, UINT32 len) {
   TSS_RESULT  result;
   UINT32      cpt = 0;
   
   if(init != 1) {
      result = Feed(locality);
      if(result != TSS_SUCCESS) {
         return result;
      }
      cptread = 0;
      init = 1;
   }

   while(cpt < len) {
      UINT32 tocopy = len - cpt;
      
      if(cptread + tocopy >= sizeof(externalpool)) {
         tocopy = sizeof(externalpool) - cptread;
      }
      
      LOGDEBUG("Read %u bytes from external pool", tocopy);
      
      memcpy(&buf[cpt], &externalpool[cptread], tocopy);
      
      cpt += tocopy;
      cptread += tocopy;
      
      if(cptread >= sizeof(externalpool)) {
         result = Feed(locality);
         if(result != TSS_SUCCESS) {
            return result;
         }
         cptread = 0;
      }
   }
   
   return TSS_SUCCESS;
}
