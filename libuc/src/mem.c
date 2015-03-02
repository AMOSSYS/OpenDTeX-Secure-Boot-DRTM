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

void * UCCALL uc_memset(void * s, int c, size_t n) {
   size_t maxword_len;
   size_t byte_len;
   size_t i;
   __type_mem * l;
   unsigned char * b;
   
   if(n == 0) {
      return s;
   }
   
   l = (__type_mem *) s;
   maxword_len = n / sizeof(__type_mem);
   byte_len = n % sizeof(__type_mem);
   
   if(maxword_len) {
       __type_mem val;
      val = c & 0xff;
      val = (val << 8) + val;
      val = (val << 16) + val;
      #if ARCHI == 64
      val = (val << 32) + val;
      #endif
      
      for(i = 0 ; i < maxword_len ; i++) {
         *l++ = val;
      }
   }
   
   b = (unsigned char *) l;
   for(i = 0 ; i < byte_len ; i++) {
      *b++ = c;
   }

   return s;
}


int UCCALL uc_memcmp(const void * s1, const void * s2, size_t n) {
   size_t maxword_len;
   size_t byte_len;
   size_t i;
   __type_mem * l1;
   __type_mem * l2;
   unsigned char * b1;
   unsigned char * b2;
   
   if((n == 0) || (s1 == s2)) {
      return 0;
   }
   
   l1 = (__type_mem *) s1;
   l2 = (__type_mem *) s2;
   maxword_len = n / sizeof(__type_mem);
   byte_len = n % sizeof(__type_mem);
   
   for(i = 0 ; i < maxword_len ; i++) {
      __type_mem val1 = *l1++;
      __type_mem val2 = *l2++;
      
      if(val1 != val2) {
         #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
         return val1 < val2 ? -1 : 1;
         #else
         l1--;
         l2--;
         byte_len = sizeof(__type_mem);
         #endif
      }
   }
   
   b1 = (unsigned char *) l1;
   b2 = (unsigned char *) l2;
   
   for(i = 0 ; i < byte_len ; i++) {
      unsigned char val1 = *b1++;
      unsigned char val2 = *b2++;
      
      if(val1 != val2) {
         return val1 < val2 ? -1 : 1;
      }
   }
   
   return 0;
}

void * UCCALL uc_memcpy(void * dst, const void * src, size_t n) {
   size_t maxword_len;
   size_t byte_len;
   size_t i;
   __type_mem * dl;
   __type_mem * sl;
   unsigned char * db;
   unsigned char * sb;
   
   if(n == 0) {
      return dst;
   }
   
   if(dst < src) {
      dl = (__type_mem *) dst;
      sl = (__type_mem *) src;
      maxword_len = n / sizeof(__type_mem);
      byte_len = n % sizeof(__type_mem);
      
      for(i = 0 ; i < maxword_len ; i++) {
         *dl++ = *sl++;
      }
      
      db = (unsigned char *) dl;
      sb = (unsigned char *) sl;
      
      for(i = 0 ; i < byte_len ; i++) {
         *db++ = *sb++;
      }
   } else {
      dl = (__type_mem *) (((unsigned char *) dst) + n);
      sl = (__type_mem *) (((unsigned char *) src) + n);
      maxword_len = n / sizeof(__type_mem);
      byte_len = n % sizeof(__type_mem);
      
      for(i = 0 ; i < maxword_len ; i++) {
         *--dl = *--sl;
      }
      
      db = (unsigned char *) dl;
      sb = (unsigned char *) sl;
      
      for(i = 0 ; i < byte_len ; i++) {
         *--db = *--sb;
      }
   }
   
   return dst;
}

