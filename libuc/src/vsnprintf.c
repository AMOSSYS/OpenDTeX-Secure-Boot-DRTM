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

/*TODO : Not totally compliant with C99
   Type not supported : 
      C : wide character
      e, E, g, G, f : double
      n : pointer to integer
      S : wide string
   Size prefixed not supported :
      w 
      I32, I64 : msdn prefix
      
   Bug when displaying negative integer with a '0' width value : "%08d", -2 --> 000000-2 instead of -0000002
*/

#define FLAGS_LEFT		1
#define FLAGS_SIGNED		2
#define FLAGS_ZERO		4
#define FLAGS_BLANK		8
#define FLAGS_DIESE		16
#define FLAGS_UPPER		32
#define FLAGS_WIDTH		64
#define FLAGS_PRECISION	128
#define FLAGS_SIZEPFX	256

#define TYPE_CHAR			1
#define TYPE_INTEGER		2
#define TYPE_DOUBLE		3
#define TYPE_STRING		4

typedef struct {
	unsigned int 	flags;
	unsigned int	width;
	unsigned int	precision;
	unsigned int	sizeprefix;
	unsigned int	type;
	unsigned int 	int_signed;
	unsigned int 	int_base;
} mod_t;

static int getflags(const char ** ptr, mod_t * mod) {
	mod->flags = 0;
	
	while(1) {
		switch(**ptr) {
		case '-':
			mod->flags	|= FLAGS_LEFT;
			(*ptr)++;
			break;
		case '+':
			mod->flags	|= FLAGS_SIGNED;
			(*ptr)++;
			break;
		case '0':
			mod->flags	|= FLAGS_ZERO;
			(*ptr)++;
			break;
		case ' ':
			mod->flags	|= FLAGS_BLANK;
			(*ptr)++;
			break;
		case '#':
			mod->flags	|= FLAGS_DIESE;
			(*ptr)++;
			break;
		case 0:
			return -1;
		default:
			goto end;
		}
	}
	
end:
	if((mod->flags & (FLAGS_LEFT | FLAGS_ZERO)) == (FLAGS_LEFT | FLAGS_ZERO)) {
		mod->flags &= ~FLAGS_ZERO;
	}
	
	if((mod->flags & (FLAGS_BLANK | FLAGS_SIGNED)) == (FLAGS_BLANK | FLAGS_SIGNED)) {
		mod->flags &= ~FLAGS_BLANK;
	}
	
	return 0;
}

#define getwidth(res, ptr, ap, mod) {                 \
   mod.width = 0;                                     \
	                                                   \
	if(*ptr == '*') {                                  \
		mod.flags |= FLAGS_WIDTH;                       \
		mod.width = va_arg(ap, int);                    \
		ptr++;                                          \
	} else {                                           \
		while(1) {                                      \
			if(*ptr == 0) {                              \
				res = -1;                                 \
			}                                            \
			                                             \
			if(('0' <= *ptr) && (*ptr <= '9')) {         \
				mod.flags |= FLAGS_WIDTH;                 \
				mod.width = mod.width * 10 + *ptr - '0';  \
				ptr++;                                    \
			} else {                                     \
				break;                                    \
			}                                            \
		}                                               \
	}                                                  \
	                                                   \
	res = 0;                                           \
}

#define getprecision(res, ptr, ap, mod) {                      \
   mod.precision = 0;                                          \
	                                                            \
	if(*ptr == '*') {                                           \
		mod.flags |= FLAGS_PRECISION;                            \
		mod.precision = va_arg(ap, int);                         \
		ptr++;                                                   \
	} else {                                                    \
		while(1) {                                               \
			if(*ptr == 0) {                                       \
				res = -1;                                          \
			}                                                     \
			                                                      \
			if(('0' <= *ptr) && (*ptr <= '9')) {                  \
				mod.flags |= FLAGS_PRECISION;                      \
				mod.precision = mod.precision * 10 + *ptr - '0';   \
				ptr++;                                             \
			} else {                                              \
				break;                                             \
			}                                                     \
		}                                                        \
	}                                                           \
	                                                            \
	res = 0;                                                    \
}

static int getsizeprefix(const char ** ptr, mod_t * mod) {
	mod->sizeprefix = 0;
	
	switch(**ptr) {
	case 'h':
		mod->flags |= FLAGS_SIZEPFX;
		if((*ptr)[1] == 'h') {
			mod->sizeprefix = 1;
			(*ptr) += 2;
		} else {
			mod->sizeprefix = 2;
			(*ptr)++;
		}
		break;
		
	case 'l':
		mod->flags |= FLAGS_SIZEPFX;
		if((*ptr)[1] == 'l') {
			mod->sizeprefix = 8;
			(*ptr) += 2;
		} else {
			mod->sizeprefix = 4;
			(*ptr)++;
		}
		break;
		
	case 0:
		return -1;
		
	default:
		break;
	}
	
	return 0;
}

static int gettype(const char type, mod_t * mod) {
	switch(type) {
	case 'd':
	case 'i':
		mod->type = TYPE_INTEGER;
		mod->int_signed = 1;
		mod->int_base = 10;
		if(! (mod->flags & FLAGS_SIZEPFX))
			mod->sizeprefix = 4;
		break;
		
	case 'u':
		mod->type = TYPE_INTEGER;
		mod->int_signed = 0;
		mod->int_base = 10;
		if(! (mod->flags & FLAGS_SIZEPFX))
			mod->sizeprefix = 4;
		break;

	case 'o':
		mod->type = TYPE_INTEGER;
		mod->int_signed = 0;
		mod->int_base = 8;
		if(! (mod->flags & FLAGS_SIZEPFX))
			mod->sizeprefix = 4;
		break;

	case 'X':
		mod->flags |= FLAGS_UPPER;
	case 'x':
		mod->type = TYPE_INTEGER;
		mod->int_signed = 0;
		mod->int_base = 16;
		if(! (mod->flags & FLAGS_SIZEPFX))
			mod->sizeprefix = 4;
		break;

	case 'p':
		mod->type = TYPE_INTEGER;
		mod->int_signed = 0;
		mod->int_base = 16;
		if(! (mod->flags & FLAGS_SIZEPFX))
			mod->sizeprefix = ARCHI / 8;
		break;

	case 'c':
		mod->type = TYPE_CHAR;
		mod->sizeprefix = 1;
		break;

	case 's':
		mod->type = TYPE_STRING;
		mod->sizeprefix = 0;
		break;
		
	default:
		return -1;
	}
	
	return 0;
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

static const char lowerdigits[] = "0123456789abcdef";
static const char upperdigits[] = "0123456789ABCDEF";

static const unsigned long long truncatemask[9] = {
	0,
	0xffULL,
	0xffffULL,
	0xffffffULL,
	0xffffffffULL,
	0xffffffffffULL,
	0xffffffffffffULL,
	0xffffffffffffffULL,
	0xffffffffffffffffULL
};

static const unsigned long long signedmax[9] = {
	0,
	0x80ULL,
	0x8000ULL,
	0x800000ULL,
	0x80000000ULL,
	0x8000000000ULL,
	0x800000000000ULL,
	0x80000000000000ULL,
	0x8000000000000000ULL
};

#define integer2string32(res, tempbuf, ap, mod) { \
   int negative = 0;                            \
	int i;                                       \
	unsigned long long val;                      \
                                                \
   res = 0;                                     \
                                                \
	if((mod.int_base < 2) || (mod.int_base > 16)) { \
	   res = -1;                                    \
		goto end_integer2string;                     \
	}                                               \
                                                   \
   /* Get the value */                             \
	if(mod.sizeprefix == 8) {                       \
		val = va_arg(ap, unsigned long long);        \
	} else {                                        \
		val = va_arg(ap, int);                       \
	}                                               \
                                                   \
   /* Truncate with the good size */               \
	val &= truncatemask[mod.sizeprefix];            \
                                                   \
	/* Truncate with the good size
	Adjust if negative signed decimal value */      \
	if(mod.int_signed && (mod.int_base == 10)) {    \
		if(val >= signedmax[mod.sizeprefix]) {       \
			val = (0ULL - val) & truncatemask[mod.sizeprefix]; \
			negative = 1;                             \
		}                                            \
	}                                               \
                                                   \
	/* Euclidian division */                        \
	do {                                            \
		unsigned int rem = 0;                        \
		if(div64(val, mod.int_base, &val, &rem) < 0) {  \
			res = -1;                                    \
   		goto end_integer2string;                     \
		}                                            \
                                                   \
		tempbuf[res] = (mod.flags & FLAGS_UPPER) ? upperdigits[rem] : lowerdigits[rem];  \
		res++;                                       \
	} while(val);                                   \
	                                                \
	/* Put some prefix if necessary */              \
	if((mod.int_base == 10) && mod.int_signed) {    \
		if(negative) {                               \
			tempbuf[res] = '-';                       \
			res++;                                    \
		} else if(mod.flags & FLAGS_SIGNED) {        \
			tempbuf[res] = '+';                       \
			res++;                                    \
		} else if(mod.flags & FLAGS_BLANK) {         \
			tempbuf[res] = ' ';                       \
			res++;                                    \
		}                                            \
	} else if(mod.flags & FLAGS_DIESE) {            \
		if(mod.int_base == 8) {                      \
			tempbuf[res] = '0';                       \
			res++;                                    \
		} else if(mod.int_base == 16) {              \
			tempbuf[res] = (mod.flags & FLAGS_UPPER) ? 'X' : 'x'; \
			tempbuf[res + 1] = '0';                   \
			res += 2;                                 \
		}                                            \
	}                                               \
                                                   \
	tempbuf[res] = 0;                               \
                                                   \
	/* Reverse */                                   \
	for(i = 0 ; i < res / 2 ; i++) {                \
		char c;                                      \
                                                   \
		c = tempbuf[i];                              \
		tempbuf[i] = tempbuf[res - i - 1];           \
		tempbuf[res - i - 1] = c;                    \
	}                                               \
                                                   \
end_integer2string:                                \
   ;                                               \
}

#define integer2string64(res, tempbuf, ap, mod) { \
   int negative = 0;                            \
	int i;                                       \
	unsigned long long val;                      \
                                                \
   res = 0;                                     \
                                                \
	if((mod.int_base < 2) || (mod.int_base > 16)) { \
	   res = -1;                                    \
		goto end_integer2string;                     \
	}                                               \
                                                   \
   /* Get the value */                             \
	val = va_arg(ap, unsigned long long);           \
                                                   \
   /* Truncate with the good size */               \
	val &= truncatemask[mod.sizeprefix];            \
                                                   \
	/* Truncate with the good size
	Adjust if negative signed decimal value */      \
	if(mod.int_signed && (mod.int_base == 10)) {    \
		if(val >= signedmax[mod.sizeprefix]) {       \
			val = (0ULL - val) & truncatemask[mod.sizeprefix]; \
			negative = 1;                             \
		}                                            \
	}                                               \
                                                   \
	/* Euclidian division */                        \
	do {                                            \
		unsigned int rem = 0;                        \
		if(div64(val, mod.int_base, &val, &rem) < 0) {  \
			res = -1;                                    \
   		goto end_integer2string;                     \
		}                                            \
                                                   \
		tempbuf[res] = (mod.flags & FLAGS_UPPER) ? upperdigits[rem] : lowerdigits[rem];  \
		res++;                                       \
	} while(val);                                   \
	                                                \
	/* Put some prefix if necessary */              \
	if((mod.int_base == 10) && mod.int_signed) {    \
		if(negative) {                               \
			tempbuf[res] = '-';                       \
			res++;                                    \
		} else if(mod.flags & FLAGS_SIGNED) {        \
			tempbuf[res] = '+';                       \
			res++;                                    \
		} else if(mod.flags & FLAGS_BLANK) {         \
			tempbuf[res] = ' ';                       \
			res++;                                    \
		}                                            \
	} else if(mod.flags & FLAGS_DIESE) {            \
		if(mod.int_base == 8) {                      \
			tempbuf[res] = '0';                       \
			res++;                                    \
		} else if(mod.int_base == 16) {              \
			tempbuf[res] = (mod.flags & FLAGS_UPPER) ? 'X' : 'x'; \
			tempbuf[res + 1] = '0';                   \
			res += 2;                                 \
		}                                            \
	}                                               \
                                                   \
	tempbuf[res] = 0;                               \
                                                   \
	/* Reverse */                                   \
	for(i = 0 ; i < res / 2 ; i++) {                \
		char c;                                      \
                                                   \
		c = tempbuf[i];                              \
		tempbuf[i] = tempbuf[res - i - 1];           \
		tempbuf[res - i - 1] = c;                    \
	}                                               \
                                                   \
end_integer2string:                                \
   ;                                               \
}

#if ARCHI == 32
#define integer2string integer2string32
#elif ARCHI == 64
#define integer2string integer2string64
#endif

/*static int integer2string(char tempbuf[256], va_list * ap, mod_t * mod) {
	int negative = 0;
	int cpt = 0;
	int i;
	unsigned long long val;

	if((mod->int_base < 2) || (mod->int_base > 16)) {
		return -1;
	}

	// Get the value
	#if ARCHI == 32
	if(mod->sizeprefix == 8) {
		val = va_arg(*ap, unsigned long long);
		//val += va_arg(*ap, unsigned long long) << 32;
	} else {
		val = va_arg(*ap, int);
	}
	#elif ARCHI == 64
	val = va_arg(*ap, unsigned long long);
	#endif


	// Truncate with the good size
	val &= truncatemask[mod->sizeprefix];

	// Truncate with the good size Adjust if negative signed decimal value
	if(mod->int_signed && (mod->int_base == 10)) {
		if(val >= signedmax[mod->sizeprefix]) {
			val = (0ULL - val) & truncatemask[mod->sizeprefix];
			negative = 1;
		}
	}

	// Euclidian division
	do {
		unsigned int rem = 0;
		if(div64(val, mod->int_base, &val, &rem) < 0) {
			return -1;
		}

		tempbuf[cpt] = (mod->flags & FLAGS_UPPER) ? upperdigits[rem] : lowerdigits[rem];
		cpt++;
	} while(val);
	
	// Put some prefix if necessary
	if((mod->int_base == 10) && mod->int_signed) {
		if(negative) {
			tempbuf[cpt] = '-';
			cpt++;
		} else if(mod->flags & FLAGS_SIGNED) {
			tempbuf[cpt] = '+';
			cpt++;
		} else if(mod->flags & FLAGS_BLANK) {
			tempbuf[cpt] = ' ';
			cpt++;
		}
	} else if(mod->flags & FLAGS_DIESE) {
		if(mod->int_base == 8) {
			tempbuf[cpt] = '0';
			cpt++;
		} else if(mod->int_base == 16) {
			tempbuf[cpt] = (mod->flags & FLAGS_UPPER) ? 'X' : 'x';
			tempbuf[cpt + 1] = '0';
			cpt += 2;
		}
	}

	tempbuf[cpt] = 0;

	// Reverse
	for(i = 0 ; i < cpt / 2 ; i++) {
		char c;

		c = tempbuf[i];
		tempbuf[i] = tempbuf[cpt - i - 1];
		tempbuf[cpt - i - 1] = c;
	}

	return cpt;
}*/

static int printstringbuffer(char ** out, size_t * cpt, size_t sizemax, char * buffer, size_t size_buffer, mod_t * mod) {
	size_t i;
	size_t width_right;
	size_t width_left;
	unsigned int precision;
	char charwidth = ' ';

	if(mod->type == TYPE_CHAR) {
		if(mod->flags & FLAGS_PRECISION) {
			mod->flags &= ~FLAGS_PRECISION;
			mod->precision = 0;
		}
	}

	if((mod->type == TYPE_INTEGER) || (mod->type == TYPE_CHAR)) {
		if((mod->flags & FLAGS_PRECISION) && (mod->precision > size_buffer)) {
			precision = mod->precision - size_buffer;
		} else {
			precision = 0;
		}

		if((mod->flags & FLAGS_WIDTH) && (mod->width > size_buffer + precision)) {
			if(mod->flags & FLAGS_LEFT) {
				width_right = mod->width - size_buffer - precision;
				width_left = 0;
			} else {
				width_right = 0;
				width_left = mod->width - size_buffer - precision;
			}
		} else {
			width_right = 0;
			width_left = 0;
		}

		if((mod->flags & FLAGS_ZERO) && (mod->type != TYPE_CHAR)) {
			charwidth = '0';
		}
	} else {
	   //TODO : others cases
	   precision = 0;
		width_right = 0;
	   width_left = 0;
	}
	
	// Write left padding
	for(i = 0 ; (i < width_left) && (*cpt < sizemax) ; i++, (*out)++, (*cpt)++) {
		**out = charwidth;
	}

	// Write precision
	for(i = 0 ; (i < precision) && (*cpt < sizemax) ; i++, (*out)++, (*cpt)++) {
		**out = '0';
	}

	// Write buffer
	for(i = 0 ; (i < size_buffer) && (*cpt < sizemax) ; i++, (*out)++, (*cpt)++) {
		**out = buffer[i];
	}

	// Write right padding
	for(i = 0 ; (i < width_right) && (*cpt < sizemax) ; i++, (*out)++, (*cpt)++) {
		**out = charwidth;
	}

	/*
	flags & FLAGS_LEFT
	flags & FLAGS_ZERO
	flags & FLAGS_WIDTH
	flags & FLAGS_PRECISION
		 
	#define TYPE_CHAR			1
	#define TYPE_INTEGER		2
	#define TYPE_DOUBLE		3
	#define TYPE_STRING		4
	*/
	
	
	/*for(i = 0 ; (i < size_buffer) && (*cpt < sizemax) ; i++, (*out)++, (*cpt)++) {
		**out = buffer[i];
	}*/
	
	return 0;
}

int UCCALL uc_vsnprintf(char * buf, size_t size, const char * fmt, va_list ap) {
	const char *	ptr = fmt;
	char * 			out = buf;
	size_t			cpt = 0;
	mod_t				mod;
	char				tempbuf[256];
	int   			tempsize;
	char *			str;
	
	while(*ptr) {
	   int res;
	   
		if(*ptr != '%') {
			*out = *ptr;
			out++;
			ptr++;
			cpt++;
			goto endloop;
		}
			
		ptr++;
		
		// case '%%'
		if(*ptr == '%') {
			*out = '%';
			out++;
			ptr++;
			cpt++;
			goto endloop;
		}
		
		// Parse % format
		
		// Get FLAGS
		if(getflags(&ptr, &mod) < 0) {
			goto end;
		}
		
		// Get WIDTH
		getwidth(res, ptr, ap, mod);
		if(res < 0) {
			goto end;
		}
		
		// Get PRECISION
		if(*ptr == '.') {
			ptr++;

         getprecision(res, ptr, ap, mod);
			if(res < 0) {
				goto end;
			}
		}
		
		// Get SIZEPREFIX
		if(getsizeprefix(&ptr, &mod) < 0) {
			goto end;
		}
		
		if(gettype(*ptr, &mod) < 0) {
			goto end;
		}
		
		ptr++;
		
		switch(mod.type) {
		case TYPE_INTEGER:
		   integer2string(tempsize, tempbuf, ap, mod);
			//tempsize = integer2string(tempbuf, &ap, &mod);
			//TODO tempsize < 0 --> error
			printstringbuffer(&out, &cpt, size - 1, tempbuf, tempsize, &mod);
			break;
		
		case TYPE_CHAR:
			tempbuf[0] = (char) va_arg(ap, int);
			tempbuf[1] = 0;
			tempsize = 1;
			printstringbuffer(&out, &cpt, size - 1, tempbuf, (size_t) tempsize, &mod);
			break;
			
		case TYPE_STRING:
			str = va_arg(ap, char *);
			printstringbuffer(&out, &cpt, size - 1, str, uc_strlen(str), &mod);
			break;
		default:
			goto end;
		}
		
	

		/*case 'n':
			printf("Format %c (%x)\n", *ptr, va_arg(ap, int));
			if(flags)
				printf(" flags      : %08x\n", flags);
			if(width)
				printf(" width      : %u\n", width);
			if(precision)
				printf(" precision  : %u\n", precision);
			if(sizeprefix)
				printf(" sizeprefix : %u\n", sizeprefix);
			ptr++;
			break;*/
			
	endloop:
		if(cpt >= size - 1) {
			break;
		}
	}
	
end:
	buf[cpt] = 0;
	
	return cpt;
}

/*

int main() {
	MessageBoxA(0, "paf", "paf", 0);
	
	

	res = snprintf2(buffer, sizeof(buffer), "coucou %%,%.4u,%08d,%+ld,%#*llX,%#o,%s\n", 0x64, 0xfffffffe, 3, 20, 0x1122334455667788ULL, 8, "paf");
	printf("Result (%u) : %s\n", res, buffer);
	res = snprintf2(buffer, sizeof(buffer), "%hhx %hx %x %llx %x\n", 0x11223344, 0x11223344, 0x11223344, 0x1122334455667788ULL, 0x11223344);
	printf("Result (%u) : %s\n", res, buffer);
	res = snprintf2(buffer, sizeof(buffer), "%hhd %hd %d %lld %d\n", -2000000000, -2000000000, -2000000000, -2000000000ULL, -2000000000);
	printf("Result (%u) : %s\n", res, buffer);
	res = snprintf2(buffer, sizeof(buffer), "%hhd %hd %d %lld %d\n", -2000000, -2000000, -2000000, -2000000ULL, -2000000);
	printf("Result (%u) : %s\n", res, buffer);
	res = snprintf2(buffer, sizeof(buffer), "%c %c\n", 'c', 0x3030);
	printf("Result (%u) : %s\n", res, buffer);
	return 0;
}*/


