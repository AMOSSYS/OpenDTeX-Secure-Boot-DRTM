// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS, Bertin
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


#include <uc/types.h>
#include <libtpm/libtpm.h>
#include <uvideo/uvideo.h>
#include <memory.h>


int LIBTPMCALL stub_printf(const char *format, ...);
void * LIBTPMCALL stub_malloc(size_t size);
void LIBTPMCALL stub_free(void *ptr);
unsigned int LIBTPMCALL stub_tpm_io(unsigned int locality, unsigned char * txBlobIn, unsigned int sizeIn, unsigned char * txBlobOut, unsigned int sizeOut);

int LIBTPMCALL stub_printf(const char *format, ...) {
	int res;
	va_list ap;
	
	va_start(ap, format);
	res = uvideo_vprintf(format, ap);
	va_end(ap);
	
	return res;
}

void * LIBTPMCALL stub_malloc(size_t size) {
	return malloc(size);
}
void LIBTPMCALL stub_free(void *ptr) {
	free(ptr);
}

#define TPM_TXBLOB_SIZE 4096

extern uint32_t tpm_write_cmd_fifo(uint32_t locality, uint8_t * in, uint32_t in_size, uint8_t * out, uint32_t * out_size);

unsigned int LIBTPMCALL stub_tpm_io(unsigned int locality, unsigned char * txBlobIn, unsigned int sizeIn, unsigned char * txBlobOut, unsigned int sizeOut) {
   return tpm_write_cmd_fifo(locality, txBlobIn, sizeIn, txBlobOut, &sizeOut);
}

