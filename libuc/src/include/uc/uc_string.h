#pragma once

#ifndef __UC_CHECK__
#error "Use uc.h header"
#endif
 
#ifdef __UC_INLINE__

static inline size_t uc_strlen(const char * s) {
   const char * end = s;
   
   while(*end) {
      end++;
   }
   
   return end - s;
}

static inline int uc_strcmp(const char * s1, const char * s2) {
   if(s1 == s2) {
      return 0;
   }
   
   while(1) {
      unsigned char val1 = (unsigned char) *s1++;
      unsigned char val2 = (unsigned char) *s2++;
      
      if(val1 != val2) {
         return val1 < val2 ? -1 : 1;
      }
      
      if(val1 == '\0') {
	      break;
	   }
   };

   return 0;
}

static inline int uc_strncmp(const char * s1, const char * s2, size_t n) {  
   if((n == 0) || (s1 == s2)) {
      return 0;
   }

   while(n > 0) {
      unsigned char val1 = (unsigned char) *s1++;
      unsigned char val2 = (unsigned char) *s2++;
      
      if(val1 != val2) {
         return val1 < val2 ? -1 : 1l;
      }
      
      if(val1 == '\0') {
         break;
      }

      n--;
    }

   return 0;
}

 __attribute__ ((deprecated)) static inline char * uc_strcpy (char * dst, const char * src) {
   char * pdst = dst;
   const char * psrc = src;
   
   while(*psrc) {
      *pdst = *psrc;
      pdst++;
      psrc++;
   }
   
   *pdst = '\0';
   
   return dst;
}

static inline char * uc_strncpy(char * dst, const char * src, size_t n) {
   char * pdst = dst;
   const char * psrc = src;

   if(n == 0) {
      return dst;
   }

   while(*psrc && n) {
      *pdst = *psrc;
      pdst++;
      psrc++;
      n--;
   }
   
   if(n)
      *pdst = '\0';
   else
      pdst[-1] = '\0';
   
   return dst;
}

__attribute__ ((deprecated)) static inline char * uc_strcat(char * dst, const char * src) {
   char * pdst = dst;
   const char * psrc = src;
   
   while(*pdst) {
      pdst++;
   }
   
   while(*psrc) {
      *pdst = *psrc;
      pdst++;
      psrc++;
   }
   
   *pdst = '\0';
   
   return dst;
}

__attribute__ ((deprecated)) static inline char * uc_strncat(char * dst, const char * src, size_t n) {
   char * pdst = dst;
   const char * psrc = src;
   
   if(n == 0) {
      return dst;
   }
   
   while(*pdst) {
      pdst++;
   }
   
   while(*psrc && n) {
      *pdst = *psrc;
      pdst++;
      psrc++;
      n--;
   }
   
   if(n)
      *pdst = '\0';
   else
      pdst[-1] = '\0';
   
   return dst;
}

static inline char * uc_strncat_s(char * dst, size_t n, const char * src, size_t count) {
   char * pdst = dst;
   const char * psrc = src;
   
   if(n == 0) {
      return dst;
   }
   
   while(*pdst && n) {
      pdst++;
      n--;
   }
   
   if(n == 0) {
      pdst[-1] = '\0';
      return dst;
   }
   
   if(n <= count) {
      count = n - 1;
   }
   
   while(*psrc && count) {
      *pdst = *psrc;
      pdst++;
      psrc++;
      count--;
   }

   *pdst = '\0';
   
   return dst;
}

#else 

size_t             UCCALL uc_strlen(const char * str);
int                UCCALL uc_strcmp(const char * s1, const char * s2);
int                UCCALL uc_strncmp(const char * s1, const char * s2, size_t n);
char *             UCCALL uc_strcpy(char * dst, const char * src)                               __attribute__ ((deprecated));
char *             UCCALL uc_strncpy(char * dst, const char * src, size_t n);
char *             UCCALL uc_strcat(char * dst, const char * src)                               __attribute__ ((deprecated));
char *             UCCALL uc_strncat(char * dst, const char * src, size_t n)                    __attribute__ ((deprecated));
char *             UCCALL uc_strncat_s(char * dst, size_t n, const char * src, size_t count);

#endif

unsigned long      UCCALL uc_strtoul(const char * nptr, char ** endptr, int base);
unsigned long long UCCALL uc_strtoull(const char * nptr, char ** endptr, int base);
int                UCCALL uc_vsnprintf(char * buf, size_t size, const char * fmt, va_list ap);
int                UCCALL uc_snprintf(char * buf, size_t size, const char * fmt, ...);



