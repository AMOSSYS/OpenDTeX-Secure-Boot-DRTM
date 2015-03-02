#pragma once 

#include <libtpm/libtpm.h>

int            LIBTPMCALL stub_printf(const char *format, ...);
void *         LIBTPMCALL stub_malloc(size_t size);
void           LIBTPMCALL stub_free(void *ptr);
unsigned int   LIBTPMCALL stub_tpm_io(unsigned int locality, unsigned char * txBlobIn, unsigned int sizeIn, unsigned char * txBlobOut, unsigned int sizeOut);

