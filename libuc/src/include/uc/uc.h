#pragma once

#include <stdarg.h>
#include <stddef.h>

#if ARCHI == 32
#define UCCALL __attribute__((stdcall, regparm(2)))
#define __type_mem unsigned int
#elif ARCHI == 64
#define UCCALL __attribute__((regparm(4)))
#define __type_mem unsigned long long
#else
#error ARCHI macro not defined
#endif

#if defined(__OPTIMIZE__) && ! defined(__NO_INLINE__) && ! defined(__UC_COMPILING__)
#define __UC_INLINE__ 1
#endif

#define __UC_CHECK__

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__  0x41424344UL
#endif
#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__     0x44434241UL
#endif
#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__           __ORDER_LITTLE_ENDIAN__
#endif

#include "uc_mem.h"
#include "uc_string.h"

//#define UC_LITTLE_ENDIAN 0x41424344UL 
//#define UC_BIG_ENDIAN    0x44434241UL
//#define UC_PDP_ENDIAN    0x42414443UL
//#define UC_ENDIAN_ORDER  ('ABCD') 

#if ! defined(__UC_COMPILING__)
#ifdef __UC_MAINLIBC__

#define memset    uc_memset
#define memcmp    uc_memcmp
#define memcpy    uc_memcpy
#define strlen    uc_strlen
#define strcmp    uc_strcmp
#define strncmp   uc_strncmp
#define strcpy    uc_strcpy
#define strncpy   uc_strncpy
#define strcat    uc_strcat
#define strncat   uc_strncat
#define strncat_s uc_strncat_s
#define strtoul   uc_strtoul
#define strtoull  uc_strtoull
#define vsnprintf uc_vsnprintf
#define snprintf  uc_snprintf

#endif
#endif

#undef __UC_CHECK__

