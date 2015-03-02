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
#include <core.h>

static bool              libtpm_init = false;

static pf_tpm_io         libtpm_tpm_io = NULL;
static pf_malloc         libtpm_malloc = NULL;
static pf_free           libtpm_free   = NULL;
static pf_printf         libtpm_printf = NULL;

static LIBTPM_VERBOSITY  libtpm_verbosity = LIBTPM_NOLOG;

static const char * levelname[LIBTPM_LOGMAX];

// Libtpm intialization functions
void LIBTPMCALL LIBTPM_Init(pf_tpm_io v_tpm_io, pf_malloc v_malloc, pf_free v_free, pf_printf v_printf) {
   libtpm_tpm_io = v_tpm_io;
   libtpm_malloc = v_malloc;
   libtpm_free   = v_free;
   libtpm_printf = v_printf;
   
   levelname[LIBTPM_NOLOG] = "NOLOG";
   levelname[LIBTPM_ERROR] = "ERROR";
   levelname[LIBTPM_DEBUG] = "DEBUG";

   if(libtpm_tpm_io && libtpm_malloc && libtpm_free && libtpm_printf) {
      libtpm_init = true;
      LOGDEBUG("Initialization done");
   }
}

bool LIBTPM_IsInit(void) {
   return libtpm_init;
}

pf_tpm_io LIBTPM_GetTPMIO(void) {
   return libtpm_tpm_io;
}

pf_malloc LIBTPM_GetMalloc(void) {
   return libtpm_malloc;
}

pf_free LIBTPM_GetFree(void) {
   return libtpm_free;
}

pf_printf LIBTPM_GetPrintf(void) {
   return libtpm_printf;
}

void LIBTPMCALL LIBTPM_SetLogVerbosity(LIBTPM_VERBOSITY v) {
   switch(v) {
   case LIBTPM_NOLOG:
   case LIBTPM_ERROR:
   case LIBTPM_DEBUG:
      libtpm_verbosity = v;
      break;
   default:
      break;
   }
}

void LIBTPM_Log(LIBTPM_VERBOSITY v, char * fmt, ...) {
   char     printfbuffer[512];
   int      res;
   va_list  ap;
   
   if(libtpm_printf == NULL) {
      return;
   }
   
   if(v > libtpm_verbosity) {
      return;
   }
   
   res = snprintf(printfbuffer, sizeof(printfbuffer), "[LIBTPM-%s]", levelname[v]);
   if(res < 0) {
      return;
   }
   
   va_start(ap, fmt);
   res = vsnprintf(&printfbuffer[res], sizeof(printfbuffer) - res, fmt, ap);
   if(res > 0) {
      libtpm_printf("%s", printfbuffer);
   }
   va_end(ap);
}

