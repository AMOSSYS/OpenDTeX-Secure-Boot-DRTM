#ifndef CRYPTO_H
#define CRYPTO_H

#include "aes.h"

#define burn(x, len)						\
	memset((char *) x, 0, len);		\
	memset((char *) x, 0xff, len);	\
	memset((char *) x, 0, len);

typedef struct {
	rijndael_ctx 	aesctx;
	unsigned char 	tmp[AES_BLOCK_SIZE];
} AESCBC_ctx;

int init_AESCBC(AESCBC_ctx *ctx, unsigned char* key, int len_key);
void encrypt_AESCBC(AESCBC_ctx *ctx, unsigned char* dst, unsigned char* src, int len);
void decrypt_AESCBC(AESCBC_ctx *ctx, unsigned char* dst, unsigned char* src, int len);
void close_AESCBC(AESCBC_ctx *ctx);

#endif

