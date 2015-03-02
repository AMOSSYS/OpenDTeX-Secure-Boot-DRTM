#pragma once

#ifndef __UC_CHECK__
#error "Use uc.h header"
#endif

#ifdef __UC_INLINE__

static inline void * uc_memset(void * s, int c, size_t n) {
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

static inline int uc_memcmp(const void * s1, const void * s2, size_t n) {
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

static inline void * uc_memcpy(void * dst, const void * src, size_t n) {
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

#else 

void *             UCCALL uc_memset(void * s, int c, size_t n);
int                UCCALL uc_memcmp(const void * s1, const void * s2, size_t n);
void *             UCCALL uc_memcpy(void * dst, const void * src, size_t n);

#endif

