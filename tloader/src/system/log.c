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
#include <uvideo/uvideo.h>
#include <log.h>

static TLOADER_VERBOSITY loglevel = TLOADER_NOLOG;

void SetLogVerbosity(TLOADER_VERBOSITY v) {
   switch(v) {
   case TLOADER_NOLOG:
   case TLOADER_ERROR:
   case TLOADER_DEBUG:
      loglevel = v;
      break;
   default:
      break;
   }
}

static const char * levelname[] = {
   "NOLOG",
   "ERROR",
   "DEBUG"
};

void Log(TLOADER_VERBOSITY v, char * fmt, ...) {
   char     printfbuffer[512];
   int      res;
   va_list  ap;
   
   if(v > loglevel) {
      return;
   }
   
   res = snprintf(printfbuffer, sizeof(printfbuffer), "[TLOADER-%s]", levelname[v]);
   if(res < 0) {
      return;
   }
   
   va_start(ap, fmt);
   res = vsnprintf(&printfbuffer[res], sizeof(printfbuffer) - res, fmt, ap);
   if(res > 0) {
      uvideo_printf("%s", printfbuffer);
   }
   va_end(ap);
}

