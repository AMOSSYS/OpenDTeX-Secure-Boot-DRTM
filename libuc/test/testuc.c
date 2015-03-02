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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static unsigned int GetTickCount(void) {
   struct timeval tv;
   if(gettimeofday(&tv, NULL) != 0)
      return 0;

   return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int res = 0;

static void hexdump(unsigned char * data, size_t size) {
   size_t i;
   
   for(i = 0 ; i < size ; i++) {
      if(i && ((i % 16) == 0)) {
         printf("\n");
      }
      printf("%02x ", data[i]);
   }
   
   printf("\n");
}

static void test_memset_sub_static(int val, size_t size) {
   void * tabG = malloc(size + 16);
   void * tabT = malloc(size + 16);
   void * valT;
   
   
   memset(tabG, 0xff, size + 16);
   memset(tabT, 0xff, size + 16);
   
   
   memset(((unsigned char *) tabG) + 8, val, size);
   valT = uc_memset(((unsigned char *) tabT) + 8, val, size);
   
   if(valT != ((unsigned char *) tabT) + 8) {
      printf("ERROR in [%s(%xh,%u)] : bad returned values %pxh != %pxh\n", __FUNCTION__, val, size, valT, ((unsigned char *) tabT) + 8);
      res = 1;
      goto end;
   }
   
   if(memcmp(tabG, tabT, size + 10)) {
      printf("ERROR in [%s(%xh,%u)] : bad data\n", __FUNCTION__, val, size);
      hexdump(tabT, size + 16);
      res = 1;
   }
   
end:
   free(tabG);
   free(tabT);
}

static void test_memset_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 400000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      int val = rand();
      unsigned char * tabG = (unsigned char *) malloc(size + 16);
      unsigned char * tabT = (unsigned char *) malloc(size + 16);
      void * valT;
   
   
      memset(tabG, 0xff, size + 16);
      memset(tabT, 0xff, size + 16);
      
      
      memset(((unsigned char *) tabG) + 8, val, size);
      valT = uc_memset(((unsigned char *) tabT) + 8, val, size);
      
      if(valT != tabT + 8) {
         printf("ERROR in [%s(%xh,%u)] : bad returned values %pxh != %pxh\n", __FUNCTION__, val, size, valT, tabT + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG, tabT, size + 16)) {
         printf("ERROR in [%s(%xh,%u)] : bad data\n", __FUNCTION__, val, size);
         hexdump(tabT, size + 16);
         res = 1;
      }
      
   end:
      free(tabG);
      free(tabT);
   }
}

static void test_memset(void) {
   test_memset_sub_static(0xdd, 32);
   test_memset_sub_static(0xdd, 16);
   test_memset_sub_static(0xdd, 1024);
   test_memset_sub_static(0xdd, 7);
   test_memset_sub_static(0xdd, 1);
   test_memset_sub_static(0xdd, 2);
   test_memset_sub_static(0xdd, 3);
   test_memset_sub_static(0xdd, 15);
   test_memset_sub_static(0xdd, 1021);
   test_memset_sub_static(0x11dd, 16);
   test_memset_sub_static(0x11dd, 15);
   test_memset_sub_static(0xdd, 0);
   test_memset_sub_random();
}

static void test_memcmp_sub_randequality(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      unsigned char * tab1 = (unsigned char *) malloc(size);
      unsigned char * tab2 = (unsigned char *) malloc(size);
      unsigned int j;
          
      for(j = 0 ; j < size ; j++) {
         tab1[j] = rand();
         tab2[j] = tab1[j];
      }
      
      if(uc_memcmp(tab1, tab2, size)) {
         printf("ERROR in [%s(%u)] : bad comparaison\n", __FUNCTION__, size);
         hexdump(tab1, size);
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_memcmp_sub_random(void) {
   unsigned int maxsize = 256;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      unsigned char * tab1 = (unsigned char *) malloc(size);
      unsigned char * tab2 = (unsigned char *) malloc(size);
      unsigned int j;
      int resG;
      int resT;
          
      for(j = 0 ; j < size ; j++) {
         tab1[j] = rand();
         if(rand() & 1) {
            tab2[j] = rand();
         } else {
            tab2[j] = tab1[j];          
         }
      }
      
      resG = memcmp(tab1, tab2, size);
      resT = uc_memcmp(tab1, tab2, size);
      
      if(resG > 0)
         resG = 1;
      else if(resG < 0)
         resG = -1;
         
      if(resT > 0)
         resT = 1;
      else if(resT < 0)
         resT = -1;
      
      if(resG != resT) {
         printf("ERROR in [%s(%u)] : bad comparaison %d != %d\n", __FUNCTION__, size, resG, resT);
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_memcmp(void) {
   test_memcmp_sub_randequality();
   test_memcmp_sub_random();
}

static void test_memcpy_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      unsigned char * tabD = (unsigned char *) malloc(size + 16);
      unsigned char * tabS = (unsigned char *) malloc(size + 16);
      void * valD;
      unsigned int j;
   
   
      memset(tabD, 0xff, size + 16);
      memset(tabS, 0xff, size + 16);
      
      for(j = 0 ; j < size ; j++) {
         tabS[j + 8] = rand();
      }
      
      valD = uc_memcpy(&tabD[8], &tabS[8], size);
            
      if(valD != tabD + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %pxh != %pxh\n", __FUNCTION__, size, valD, tabD + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabD, tabS, size + 16)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump(tabD, size + 16);
         res = 1;
      }
      
   end:
      free(tabD);
      free(tabS);
   }
}

static void test_memcpy(void) {
   test_memcpy_sub_random();
}

static void test_strlen_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab = (char *) malloc(size);
      unsigned int j;
      size_t valG;
      size_t valT;

      memset(tab, 0xff, size);
      
      for(j = 0 ; j < size ; j++) {
         tab[j] = rand();
      }
      
      valG = strlen(tab);
      valT = uc_strlen(tab);
      if(valG != valT) {
         printf("ERROR in [%s(%u)] : bad returned values %u != %u\n", __FUNCTION__, size, (unsigned int) valG, (unsigned int) valT);
         res = 1;
         goto end;
      }

   end:
      free(tab);
   }
}

static void test_strlen(void) {
    test_strlen_sub_random();
}

static void test_strcmp_sub_randequality(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab1 = (char *) malloc(size + 1);
      char * tab2 = (char *) malloc(size + 1);
      unsigned int j;

      for(j = 0 ; j < size + 1; j++) {
         tab1[j] = rand();
         tab2[j] = tab1[j];
      }
      
      tab1[size] = 0;
      tab2[size] = 0;
      
      if(uc_strcmp(tab1, tab2)) {
         printf("ERROR in [%s(%u)] : bad comparaison\n", __FUNCTION__, size);
         hexdump((unsigned char *) tab1, size);
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_strcmp_sub_random(void) {
   unsigned int maxsize = 256;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab1 = (char *) malloc(size + 1);
      char * tab2 = (char *) malloc(size + 1);
      unsigned int j;
      int resG;
      int resT;
          
      for(j = 0 ; j < size + 1 ; j++) {
         tab1[j] = rand();
         if(rand() & 1) {
            tab2[j] = rand();
         } else {
            tab2[j] = tab1[j];          
         }
      }
      
      tab1[size] = 0;
      tab2[size] = 0;
      
      resG = strcmp(tab1, tab2);
      resT = uc_strcmp(tab1, tab2);
      
      if(resG > 0)
         resG = 1;
      else if(resG < 0)
         resG = -1;
         
      if(resT > 0)
         resT = 1;
      else if(resT < 0)
         resT = -1;
      
      if(resG != resT) {
         printf("ERROR in [%s(%u)] : bad comparaison %d != %d\n", __FUNCTION__, size, resG, resT);
         hexdump((unsigned char *) tab1, strlen(tab1));
         printf("------\n");
         hexdump((unsigned char *) tab2, strlen(tab2));
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_strcmp(void) {
   test_strcmp_sub_randequality();
   test_strcmp_sub_random();
}

static void test_strncmp_sub_randequality(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab1 = (char *) malloc(size + 1);
      char * tab2 = (char *) malloc(size + 1);
      unsigned int j;
          
      for(j = 0 ; j < size + 1 ; j++) {
         tab1[j] = rand();
         tab2[j] = tab1[j];
      }
      
      tab1[size] = 0;
      tab2[size] = 0;
      
      if(uc_strncmp(tab1, tab2, size + 1)) {
         printf("ERROR in [%s(%u)] : bad comparaison\n", __FUNCTION__, size);
         hexdump((unsigned char *) tab1, size);
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_strncmp_sub_random(void) {
   unsigned int maxsize = 256;
   unsigned int maxiter = 40000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab1 = (char *) malloc(size + 1);
      char * tab2 = (char *) malloc(size + 1);
      unsigned int j;
      int resG;
      int resT;
          
      for(j = 0 ; j < size + 1 ; j++) {
         tab1[j] = rand();
         if(rand() & 1) {
            tab2[j] = rand();
         } else {
            tab2[j] = tab1[j];          
         }
      }
      
      tab1[size] = 0;
      tab2[size] = 0;
      
      resG = strncmp(tab1, tab2, size + 1);
      resT = uc_strncmp(tab1, tab2, size + 1);
      
      if(resG > 0)
         resG = 1;
      else if(resG < 0)
         resG = -1;
         
      if(resT > 0)
         resT = 1;
      else if(resT < 0)
         resT = -1;
      
      if(resG != resT) {
         printf("ERROR in [%s(%u)] : bad comparaison %d != %d\n", __FUNCTION__, size, resG, resT);
         res = 1;
      }
      
      free(tab1);
      free(tab2);
   }
}

static void test_strncmp(void) {
   test_strncmp_sub_randequality();
   test_strncmp_sub_random();
}

static void test_strcpy_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tabD = (char *) malloc(size + 16 + 1);
      char * tabS = (char *) malloc(size + 16 + 1);
      void * valD;
      unsigned int j;
   
   
      memset(tabD, 0xff, size + 16 + 1);
      memset(tabS, 0xff, size + 16 + 1);
      
      for(j = 0 ; j < size + 1 ; j++) {
         tabS[j + 8] = rand();
      }
      
      tabS[size + 8] = 0;
      
      valD = uc_strcpy(&tabD[8], &tabS[8]);
            
      if(valD != tabD + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %ph != %ph\n", __FUNCTION__, size, valD, tabD + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabD, tabS, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the beginning\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabD + 8 + size + 1, tabS + 8 + size + 1, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the end\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(strcmp(tabD + 8, tabS + 8)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump((unsigned char *) tabD, size + 16);
         res = 1;
      }
      
   end:
      free(tabD);
      free(tabS);
   }
}

static void test_strcpy(void) {
   test_strcpy_sub_random();
}

static void test_strncpy_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tabD = (char *) malloc(size + 16 + 1);
      char * tabS = (char *) malloc(size + 16 + 1);
      void * valD;
      unsigned int j;
   
   
      memset(tabD, 0xff, size + 16 + 1);
      memset(tabS, 0xff, size + 16 + 1);
      
      for(j = 0 ; j < size + 1 ; j++) {
         tabS[j + 8] = rand();
      }
      
      tabS[size + 8] = 0;
      
      valD = uc_strncpy(&tabD[8], &tabS[8], size + 1);
            
      if(valD != tabD + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %pxh != %pxh\n", __FUNCTION__, size, valD, tabD + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabD, tabS, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the beginning\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabD + 8 + size + 1, tabS + 8 + size + 1, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the end\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(strncmp(tabD + 8, tabS + 8, size + 1)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump((unsigned char *) tabD, size + 16 + 1);
         res = 1;
      }
      
   end:
      free(tabD);
      free(tabS);
   }
}

static void test_strncpy(void) {
   test_strncpy_sub_random();
}

static void test_strcat_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      if(size == 0)
         size = 1;
      unsigned int sizelow = rand() % size;
      char * tabG = (char *) malloc(size + 1 + 16);
      char * tabT = (char *) malloc(size + 1 + 16);
      char * high = (char *) malloc(size - sizelow + 1);
      char * valT;
      unsigned int j;
   
      memset(tabG, 0xff, size + 1 + 16);
      memset(tabT, 0xff, size + 1 + 16);
      memset(high, 0xff, size - sizelow + 1);
      
      for(j = 0 ; j < sizelow ; j++) {
         tabG[j + 8] = rand();
         tabT[j + 8] = tabG[j + 8];
      }
      
      for(j = 0 ; j < size - sizelow ; j++) {
         high[j] = rand();
      }
            
      tabG[sizelow + 8] = 0;
      tabT[sizelow + 8] = 0;
      high[size - sizelow] = 0;
      
      strcat(&tabG[8], high);
      valT = uc_strcat(&tabT[8], high);
                
      if(valT != tabT + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %ph != %ph\n", __FUNCTION__, size, valT, tabT + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG, tabT, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the beginning\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG + 8 + size + 1, tabT + 8 + size + 1, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the end\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(strncmp(tabG + 8, tabT + 8, size + 1)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump((unsigned char *) tabT, size + 16 + 1);
         res = 1;
      }
      
   end:
      free(tabG);
      free(tabT);
   }
}

static void test_strcat(void) {
   test_strcat_sub_random();
}

static void test_strncat_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      if(size == 0)
         size = 1;
      unsigned int sizelow = rand() % size;
      char * tabG = (char *) malloc(size + 1 + 16);
      char * tabT = (char *) malloc(size + 1 + 16);
      char * high = (char *) malloc(size - sizelow + 1);
      char * valT;
      unsigned int j;
   
      memset(tabG, 0xff, size + 1 + 16);
      memset(tabT, 0xff, size + 1 + 16);
      memset(high, 0xff, size - sizelow + 1);
      
      for(j = 0 ; j < sizelow ; j++) {
         tabG[j + 8] = rand();
         tabT[j + 8] = tabG[j + 8];
      }
      
      for(j = 0 ; j < size - sizelow ; j++) {
         high[j] = rand();
      }
            
      tabG[sizelow + 8] = 0;
      tabT[sizelow + 8] = 0;
      high[size - sizelow] = 0;
      
      strncat(&tabG[8], high, size - sizelow + 1);
      valT = uc_strncat(&tabT[8], high, size - sizelow + 1);
                
      if(valT != tabT + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %ph != %ph\n", __FUNCTION__, size, valT, tabT + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG, tabT, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the beginning\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG + 8 + size + 1, tabT + 8 + size + 1, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the end\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(strncmp(tabG + 8, tabT + 8, size + 1)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump((unsigned char *) tabT, size + 16 + 1);
         res = 1;
      }
      
   end:
      free(tabG);
      free(tabT);
   }
}

static void test_strncat(void) {
   test_strncat_sub_random();
}

static void test_strncat_s_sub_random(void) {
   unsigned int maxsize = 8192;
   unsigned int maxiter = 10000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      if(size == 0)
         size = 1;
      unsigned int sizelow = rand() % size;
      char * tabG = (char *) malloc(size + 1 + 16);
      char * tabT = (char *) malloc(size + 1 + 16);
      char * high = (char *) malloc(size - sizelow + 1);
      char * valT;
      unsigned int j;
   
      memset(tabG, 0xff, size + 1 + 16);
      memset(tabT, 0xff, size + 1 + 16);
      memset(high, 0xff, size - sizelow + 1);
      
      for(j = 0 ; j < sizelow ; j++) {
         tabG[j + 8] = rand();
         tabT[j + 8] = tabG[j + 8];
      }
      
      for(j = 0 ; j < size - sizelow ; j++) {
         high[j] = rand();
      }
            
      tabG[sizelow + 8] = 0;
      tabT[sizelow + 8] = 0;
      high[size - sizelow] = 0;
      
      strncat(&tabG[8], high, size - sizelow + 1);
      valT = uc_strncat_s(&tabT[8], size + 1, high, size - sizelow + 1);
                
      if(valT != tabT + 8) {
         printf("ERROR in [%s(%u)] : bad returned values %ph != %pxh\n", __FUNCTION__, size, valT, tabT + 8);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG, tabT, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the beginning\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(memcmp(tabG + 8 + size + 1, tabT + 8 + size + 1, 8)) {
          printf("ERROR in [%s(%u)] : overflow at the end\n", __FUNCTION__, size);
         res = 1;
         goto end;
      }
      
      if(strncmp(tabG + 8, tabT + 8, size + 1)) {
         printf("ERROR in [%s(%u)] : bad data\n", __FUNCTION__, size);
         hexdump((unsigned char *) tabT, size + 16 + 1);
         res = 1;
      }
      
   end:
      free(tabG);
      free(tabT);
   }
}

static void test_strncat_s_sub_custom(void) {
   {
      char test1[16];
      strncpy(test1, "12345678", sizeof(test1));
      uc_strncat_s(test1, sizeof(test1), "90abcdef", sizeof("90abcdef"));
      if(strcmp(test1, "1234567890abcde")) {
         printf("ERROR in [%s] : bad data : %s\n", __FUNCTION__, test1);
         res = 1;
      }
   }
   
   {
      char test1[16];
      strncpy(test1, "1234", sizeof(test1));
      uc_strncat_s(test1, sizeof(test1), "5678", sizeof("5678"));
      if(strcmp(test1, "12345678")) {
         printf("ERROR in [%s] : bad data : %s\n", __FUNCTION__, test1);
         res = 1;
      }
   }
   
   {
      char test1[16];
      strncpy(test1, "0123456789abcdef", sizeof(test1));
      uc_strncat_s(test1, sizeof(test1), "AABBCCDD", sizeof("AABBCCDD"));
      if(strcmp(test1, "0123456789abcde")) {
         printf("ERROR in [%s] : bad data : %s\n", __FUNCTION__, test1);
         res = 1;
      }
   }
   
   {
      char test1[16];
      strncpy(test1, "0123456789abcde", sizeof(test1));
      uc_strncat_s(test1, sizeof(test1), "AABBCCDD", sizeof("AABBCCDD"));
      if(strcmp(test1, "0123456789abcde")) {
         printf("ERROR in [%s] : bad data : %s\n", __FUNCTION__, test1);
         res = 1;
      }
   }
   
   {
      char test1[16];
      strncpy(test1, "0123456789abcd", sizeof(test1));
      uc_strncat_s(test1, sizeof(test1), "AABBCCDD", sizeof("AABBCCDD"));
      if(strcmp(test1, "0123456789abcdA")) {
         printf("ERROR in [%s] : bad data : %s\n", __FUNCTION__, test1);
         res = 1;
      }
   }
}

static void test_strncat_s(void) {
   test_strncat_s_sub_random();
   test_strncat_s_sub_custom();
}


static void test_strtoul_sub_random(void) {
   unsigned int maxsize = 12;
   unsigned int maxiter = 1000000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab = (char *) malloc(size + 1);
      char * endG;
      char * endT;
      unsigned long valG;
      unsigned long valT;
      unsigned int j;
   
      memset(tab, 0xff, size);
      
      for(j = 0 ; j < size ; j++) {
         tab[j] = rand();
      }
            
      tab[size] = 0;
      
      for(j = 0 ; j < 37 ; j++) {
         if(j == 1) {
            continue;
         }
         
         valG = strtoul(tab, &endG, j);
         valT = uc_strtoul(tab, &endT, j);
         
         
         if(valG != valT) {
            printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad returned values %lxh != %lxh\n", __FUNCTION__, size, j, tab, valG, valT);
            hexdump((unsigned char *) tab, size);
            res = 1;
         }
         
         if(endG != endT) {
            printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad end pointer values %ph != %ph\n", __FUNCTION__, size, j, tab, endG, endT);
            hexdump((unsigned char *) tab, size);
            res = 1;
         }
      }

      free(tab);
   }

}

static void test_strtoul_sub_custom(void) {
	//TODO
	unsigned int size = sizeof("\x41\x67\x5a\x67\x74\x65\x61\x53") - 1;
	char tab[] = "\x41\x67\x5a\x67\x74\x65\x61\x53";
   char * endG;
   char * endT;
   unsigned long valG;
   unsigned long valT;
   unsigned int j = 36;
	
	valG = strtoul(tab, &endG, j);
   valT = uc_strtoul(tab, &endT, j);
   
   
   if(valG != valT) {
      printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad returned values %lxh != %lxh\n", __FUNCTION__, size, j, tab, valG, valT);
      hexdump((unsigned char *) tab, size);
      res = 1;
   }
   
   if(endG != endT) {
      printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad end pointer values %ph != %ph\n", __FUNCTION__, size, j, tab, endG, endT);
      hexdump((unsigned char *) tab, size);
      res = 1;
   }

}

static void test_strtoul(void) {
   test_strtoul_sub_random();
   //test_strtoul_sub_custom();
}

static void test_strtoull_sub_random(void) {
   unsigned int maxsize = 20;
   unsigned int maxiter = 1000000;
   unsigned int i;
   
   for(i = 0 ; i < maxiter ; i++) {
      unsigned int size = rand() % maxsize;
      char * tab = (char *) malloc(size + 1);
      char * endG;
      char * endT;
      unsigned long long valG;
      unsigned long long valT;
      unsigned int j;
   
      memset(tab, 0xff, size);
      
      for(j = 0 ; j < size ; j++) {
         tab[j] = rand();
      }
            
      tab[size] = 0;
      
      for(j = 0 ; j < 37 ; j++) {
         if(j == 1) {
            continue;
         }
         
         valG = strtoull(tab, &endG, j);
         valT = uc_strtoull(tab, &endT, j);
         
         
         if(valG != valT) {
            printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad returned values %llxh != %llxh\n", __FUNCTION__, size, j, tab, valG, valT);
            hexdump((unsigned char *) tab, size);
            res = 1;
         }
         
         if(endG != endT) {
            printf("ERROR in [%s(%u), base = %u, @buf = %ph] : bad end pointer values %ph != %ph\n", __FUNCTION__, size, j, tab, endG, endT);
            hexdump((unsigned char *) tab, size);
            res = 1;
         }
      }

      free(tab);
   }

}

static void test_strtoull(void) {
   test_strtoull_sub_random();
}

static void test_vsnprintf_subtest(char * fmt, ...) {
	char buffer1[1024];
	int res1;
	char buffer2[1024];
	int res2;
	va_list ap;
		
	va_start(ap, fmt);
	
	res1 = vsnprintf(buffer1, sizeof(buffer1), fmt, ap);
	res2 = uc_vsnprintf(buffer2, sizeof(buffer2), fmt, ap);
	
	if((res1 != res2) || strncmp(buffer1, buffer2, sizeof(buffer1))) {
		printf("KO ..%s..\n", buffer1);
		printf("   !!%s!!\n", buffer2);
	} else {
		printf("OK ==%s==\n", buffer1);
	}
	
	va_end(ap);
}

static void test_vsnprintf_sub_custom(void) {
	test_vsnprintf_subtest("%x", 0x32);
	test_vsnprintf_subtest("%-8x", 0x32);
	test_vsnprintf_subtest("%-.8x", 0x32);
	test_vsnprintf_subtest("%-.8x", 0x32);
	test_vsnprintf_subtest("%-5.3x", 0x32);
	test_vsnprintf_subtest("%-3.5x", 0x32);
	test_vsnprintf_subtest("%-5.5x", 0x32);
	test_vsnprintf_subtest("%-05.5x", 0x32);
	test_vsnprintf_subtest("%-5.5x", 0x32);
	test_vsnprintf_subtest("%05.5x", 0x32);
	
	test_vsnprintf_subtest("%c", 0x31);
	test_vsnprintf_subtest("%-8c", 0x31);
	test_vsnprintf_subtest("%-.8c", 0x31);
	test_vsnprintf_subtest("%-.8c", 0x31);
	test_vsnprintf_subtest("%-5.3c", 0x31);
	test_vsnprintf_subtest("%-3.5c", 0x31);
	test_vsnprintf_subtest("%-5.5c", 0x31);
	test_vsnprintf_subtest("%-05.5c", 0x31);
	test_vsnprintf_subtest("%-5.5c", 0x31);
	test_vsnprintf_subtest("%05.5c", 0x31);
	
	test_vsnprintf_subtest("%d", 5);
	test_vsnprintf_subtest("%d", -5);
	test_vsnprintf_subtest("%+d", 5);
	test_vsnprintf_subtest("%+d", -5);
	test_vsnprintf_subtest("% d", 5);
	test_vsnprintf_subtest("% d", -5);
	test_vsnprintf_subtest("%+ d", 5);
	test_vsnprintf_subtest("%+ d", -5);
	test_vsnprintf_subtest("coucou %%,% 4.4u,%06.4u,%04.6u,%08d,%+ld,%#*llX,%#o,%s\n", 0x64, 0x64, 0x64, 0xfffffffe, 3, 20, 0x1122334455667788ULL, 8, "paf");
}

static void test_vsnprintf(void) {
	test_vsnprintf_sub_custom();
}

/*
int                UCCALL uc_vsnprintf (char * buf, size_t size, const char * fmt, va_list ap);*/

int main(void) {
   srand(GetTickCount());
   //srand(0);
   printf("Test memset\n");
   test_memset();
   printf("Test memcmp\n");
   test_memcmp();
   printf("Test memcpy\n");
   test_memcpy();
   printf("Test strlen\n");
   test_strlen();
   printf("Test strcmp\n");
   test_strcmp();
   printf("Test strncmp\n");
   test_strncmp();
   printf("Test strcpy\n");
   test_strcpy();
   printf("Test strncpy\n");
   test_strncpy();
   printf("Test strcat\n");
   test_strcat();
   printf("Test strncat\n");
   test_strncat();
   printf("Test strncat_s\n");
   test_strncat_s();
   printf("Test strtoul\n");
   test_strtoul();
   printf("Test strtoull\n");
   test_strtoull();
   printf("Test vsnprintf\n");
   test_vsnprintf();
   
   return res;
}

