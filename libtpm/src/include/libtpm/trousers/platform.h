/*++

There are platform dependent and general defines.

--*/

#ifndef TSS_PLATFORM_H
#define TSS_PLATFORM_H


/* The default implementation is to use stdint.h, a part of the C99 standard.
 * Systems that don't support this are handled on a case-by-case basis.
 */

typedef unsigned char		BYTE;
typedef char					TSS_BOOL;
typedef unsigned short		UINT16;
typedef unsigned int			UINT32;
typedef unsigned long long	UINT64;

typedef unsigned short		TSS_UNICODE;
typedef void *					PVOID;


/* Include this so that applications that use names as defined in the
 * 1.1 TSS specification can still compile
 */
#include "compat11b.h"

#endif // TSS_PLATFORM_H
