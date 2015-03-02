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

//TODO : to optimize ...
size_t UCCALL uc_strlen(const char * str) {
   const char * end = str;
   
   while(*end) {
      end++;
   }
   
   return end - str;
}

int UCCALL uc_strcmp(const char * s1, const char * s2) {
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

int UCCALL uc_strncmp(const char * s1, const char * s2, size_t n) {
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

char * UCCALL uc_strcpy(char * dst, const char * src) {
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

char * UCCALL uc_strncpy(char * dst, const char * src, size_t n) {
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

char * UCCALL uc_strcat(char * dst, const char * src) {
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

char * UCCALL uc_strncat(char * dst, const char * src, size_t n) {
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

char * UCCALL uc_strncat_s(char * dst, size_t n, const char * src, size_t count) {
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

#define _SP 1

static unsigned char char_type[256] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, _SP, _SP, _SP, _SP, _SP, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   _SP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define ISSPACE(x) ((char_type[(unsigned char) x] & _SP) != 0)

static unsigned char stroul_val[256] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
      0,    1,    2,    3,    4,    5,    6,    7,    8,    9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24, 
     25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24, 
     25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35, 0xff, 0xff, 0xff, 0xff, 0xff,  
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned long UCCALL uc_strtoul(const char * nptr, char ** endptr, int base) {
   const char *   s = nptr;
   int            neg = 0;
   int            nodigit = 1;
   unsigned long  val = 0;
   unsigned long  limval;
   unsigned long  limoff;
   char *         tmpptr;
   
   tmpptr = (char *) nptr;
   
   if(! ((0 <= base ) && (base <= 36))) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(base == 1) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   // skip space
   while(ISSPACE(*s) && *s) {
      s++;
   }
   
   if(*s == 0) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(*s == '-') {
		neg = 1;
		s++;
	} else if(*s == '+') {
		s++;
   }
   
   if(*s == 0) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(base == 16) {
      if(s[0] == '0') {
         if((s[1] == 'x') || (s[1] == 'X')) {
            tmpptr = (char *) &s[1];
            s += 2;
         }
      }
   }
   
   if(base == 0) {
      if(*s == '0') {
         s++;
         
         tmpptr = (char *) s;
         
         if(*s == 0) {
            if(endptr) {
               *endptr = tmpptr;
            }
            return val;
         }
      
         if((*s == 'x') || (*s == 'X')) {
            base = 16;
            s++;
         } else {
            base = 8;
         }
      } else {
         base = 10;
      }
   }
   
   limval = (unsigned long) -1 / (unsigned long) base;
   limoff = (unsigned long) -1 % (unsigned long) base;
   
   while(*s) {
      unsigned long temp = (unsigned long) stroul_val[(unsigned char) *s];
      if(temp >= (unsigned int) base) {
        break;
      }
      
      nodigit = 0;
      
      if(val > limval) {
         val = -1;
      } else if((val == limval) && (temp > limoff)) {
         val = (unsigned long) -1;
      } else {  
         val *= base;
         val += temp;
      }
      
      s++;
   }
   
   if(! nodigit) {
      if(val != (unsigned long) -1) {
         if(neg) {
            val = -val;   
         }
      }
      tmpptr = (char *) s;
   }
   
   if(endptr) {
      *endptr = tmpptr;
   }

   return val;
}

static int div64(unsigned long long num, unsigned int base, unsigned long long * quot, unsigned int * rem) {
	unsigned int high;
   unsigned int low;

	/* check exceptions */
	if((quot == 0) || (rem == 0) || (base == 0)) {
		return -1;
	}

	high = num >> 32;
	low = (unsigned int) num;

	if(high == 0) {
		*quot = low / base;
		*rem = low % base;
	} else {
		unsigned long long hquo = high / base;
		unsigned int hrem = high % base;
		unsigned int lquo;
		
      /*
      * use "divl" instead of "/" to avoid the link error
      * undefined reference to `__udivdi3'
      */
      __asm__ __volatile__ ( "divl %4;"
                            : "=a"(lquo), "=d"(*rem)
                            : "a"(low), "d"(hrem), "r"(base));
      *quot = (hquo << 32) + lquo;
        
		/*unsigned long long temp = hrem;
		temp <<= 32;
		temp += low;

		lquo = temp / base;
		*rem = temp % base;
		*quot = (hquo << 32) + lquo;*/
	}

	return 0;
}

unsigned long long UCCALL uc_strtoull(const char * nptr, char ** endptr, int base) {
   const char *       s = nptr;
   int                neg = 0;
   int                nodigit = 1;
   unsigned long long val = 0;
   unsigned long long limval;
   unsigned int       limoff;
   char *             tmpptr;
   
   tmpptr = (char *) nptr;
   
   if(! ((0 <= base ) && (base <= 36))) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(base == 1) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   // skip space
   while(ISSPACE(*s) && *s) {
      s++;
   }
   
   if(*s == 0) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(*s == '-') {
		neg = 1;
		s++;
	} else if(*s == '+') {
		s++;
   }
   
   if(*s == 0) {
      if(endptr) {
         *endptr = tmpptr;
      }
      return val;
   }
   
   if(base == 16) {
      if(s[0] == '0') {
         if((s[1] == 'x') || (s[1] == 'X')) {
            tmpptr = (char *) &s[1];
            s += 2;
         }
      }
   }
   
   if(base == 0) {
      if(*s == '0') {
         s++;
            
         tmpptr = (char *) s;

         if(*s == 0) {
            if(endptr) {
               *endptr = tmpptr;
            }
            return val;
         }
      
         if((*s == 'x') || (*s == 'X')) {
            base = 16;
            s++;
         } else {
            base = 8;
         }
      } else {
         base = 10;
      }
   }
   
   div64((unsigned long long) -1, base, &limval, &limoff);
      
   while(*s) {
      unsigned long temp = stroul_val[(unsigned char) *s];
      if(temp >= (unsigned int) base) {
        break;
      }
      
      nodigit = 0;
      
      if(val > limval) {
         val = -1;
      } else if((val == limval) && (temp > limoff)) {
         val = -1;
      } else {
         val *= base;
         val += temp;
      }
      
      s++;
   }
   
   if(! nodigit) {
      if(val != (unsigned long long) -1) {
         if(neg) {
            val = -val;   
         }
      }
      tmpptr = (char *) s;
   }
   
   if(endptr) {
      *endptr = tmpptr;
   }

   return val;
}


