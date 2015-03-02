#pragma once

#include <uc/types.h>

#if ARCHI == 32
#define LIBTXTCALL __attribute__((stdcall, regparm(2)))
#define __type_mem unsigned int
#elif ARCHI == 64
#define LIBTXTCALL __attribute__((regparm(4)))
#define __type_mem unsigned long long
#else
#error ARCHI macro not defined
#endif

#define __LIBTXT_CHECK__

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "misc.h"
#include "smx.h"
#include "txtregs.h"
#include "mle.h"
#include "heap.h"
#include "acmod.h"
#include "errorcode.h"

#undef __LIBTXT_CHECK__

