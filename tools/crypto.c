// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2013, AMOSSYS
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

/*
 * File contributors:
 *   - Goulven Guiheux (AMOSSYS)
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "crypto.h"

/******************************************************************************************
AES 256 / CBC / PKCS5PADDING
******************************************************************************************/

#define XOR128(dst, src)												\
	((unsigned int *) dst)[0] ^= ((unsigned int *) src)[0];	\
	((unsigned int *) dst)[1] ^= ((unsigned int *) src)[1];	\
	((unsigned int *) dst)[2] ^= ((unsigned int *) src)[2];	\
	((unsigned int *) dst)[3] ^= ((unsigned int *) src)[3];

int init_AESCBC(AESCBC_ctx *ctx, unsigned char* key, int len_key) {
	if(((int) ctx && (int) key && len_key) == 0) {
		printf("%s : Initialization of AES fails\n", __FUNCTION__);
		return 1;
	}

	memset(ctx->tmp, 0, AES_BLOCK_SIZE);
	
	if(rijndael_set_key(&ctx->aesctx, key, len_key * 8)) {
		burn(ctx, sizeof(AESCBC_ctx))
		printf("%s : Initialization of AES fails\n", __FUNCTION__);
		return 1;
	}

	return 0;
}

void encrypt_AESCBC(AESCBC_ctx *ctx, unsigned char* dst, unsigned char* src, int len) {
	int cpt = 0;

	if(len % AES_BLOCK_SIZE) {
		printf("%s : Bad alignment !\n", __FUNCTION__);
	}

	while(cpt < len) {
		XOR128(ctx->tmp, src)
		rijndael_encrypt(&ctx->aesctx, ctx->tmp, dst);
		memcpy(ctx->tmp, dst, AES_BLOCK_SIZE);

		src += AES_BLOCK_SIZE;
		dst += AES_BLOCK_SIZE;
		cpt += AES_BLOCK_SIZE;
	}	
}

void decrypt_AESCBC(AESCBC_ctx *ctx, unsigned char* dst, unsigned char* src, int len) {
	int cpt = 0;

	if(len % AES_BLOCK_SIZE) {
		printf("%s : Bad alignment !\n", __FUNCTION__);
	}

	while(cpt < len) {
		rijndael_decrypt(&ctx->aesctx, src, dst);
		XOR128(dst, ctx->tmp)
		memcpy(ctx->tmp, src, AES_BLOCK_SIZE);

		src += AES_BLOCK_SIZE;
		dst += AES_BLOCK_SIZE;
		cpt += AES_BLOCK_SIZE;
	}	
}

void close_AESCBC(AESCBC_ctx *ctx) {
	burn(ctx, sizeof(AESCBC_ctx))
}
