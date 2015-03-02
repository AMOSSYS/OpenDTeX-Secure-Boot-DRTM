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


#include <stdio.h>
#include <string.h>
#include "aes.h"

extern char AES_ENCRYPT_ECB_MCT256_KEY[400][32];
extern char AES_ENCRYPT_ECB_MCT256_PLAINTEXT[400][16];
extern char AES_ENCRYPT_ECB_MCT256_CIPHERTEXT[400][16];

extern char AES_DECRYPT_ECB_MCT256_KEY[400][32];
extern char AES_DECRYPT_ECB_MCT256_CIPHERTEXT[400][16];
extern char AES_DECRYPT_ECB_MCT256_PLAINTEXT[400][16];

#define xor256(a,b,c) \
	((int *)a)[0]=((int *)b)[0]^((int *)c)[0];	\
	((int *)a)[1]=((int *)b)[1]^((int *)c)[1];	\
	((int *)a)[2]=((int *)b)[2]^((int *)c)[2];	\
	((int *)a)[3]=((int *)b)[3]^((int *)c)[3];	\
	((int *)a)[4]=((int *)b)[4]^((int *)c)[4];	\
	((int *)a)[5]=((int *)b)[5]^((int *)c)[5];	\
	((int *)a)[6]=((int *)b)[6]^((int *)c)[6];	\
	((int *)a)[7]=((int *)b)[7]^((int *)c)[7];

int aes_encrypt_ecb() {
	rijndael_ctx 	aesctx;
	unsigned char K[32];
	unsigned char KS[32];
	unsigned char M[16];
	unsigned char C[16];
	unsigned char TMP[16];
	int i, j;
	
	memset(K, 0, 32);
	memset(KS, 0, 32);
	memset(M, 0, 16);
	memset(C, 0, 16);
	memset(TMP, 0, 16);
	
	for(i = 0 ; i < 400 ; i++) {
		if(memcmp(M, AES_ENCRYPT_ECB_MCT256_PLAINTEXT[i], 16) != 0) {
			printf("(%s) : Plain error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		if(memcmp(K, AES_ENCRYPT_ECB_MCT256_KEY[i], 32) != 0) {
			printf("(%s) : Key error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		memcpy(KS, K, 32);
		rijndael_set_key(&aesctx, KS, 256);
		
		for(j = 0 ; j < 10000 ; j++) {
			memcpy(TMP, C, 16);
			rijndael_encrypt(&aesctx, M, C);
			memcpy(M, C, 16);
		}
		
		if(memcmp(C, AES_ENCRYPT_ECB_MCT256_CIPHERTEXT[i], 16) != 0) {
			printf("(%s) : Cipher error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		memcpy(K, TMP, 16);
		memcpy(&K[16], C, 16);
		xor256(K, K, KS)
	}
	
	return 1;
}

int aes_decrypt_ecb() {
	rijndael_ctx 	aesctx;
	unsigned char K[32];
	unsigned char KS[32];
	unsigned char M[16];
	unsigned char C[16];
	unsigned char TMP[16];
	int i, j;
	
	memset(K, 0, 32);
	memset(KS, 0, 32);
	memset(M, 0, 16);
	memset(C, 0, 16);
	memset(TMP, 0, 16);
	
	for(i = 0 ; i < 400 ; i++) {
		if(memcmp(C, AES_DECRYPT_ECB_MCT256_CIPHERTEXT[i], 16) != 0) {
			printf("(%s) : Cipher error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		if(memcmp(K, AES_DECRYPT_ECB_MCT256_KEY[i], 32) != 0) {
			printf("(%s) : Key error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		memcpy(KS, K, 32);
		rijndael_set_key(&aesctx, KS, 256);
		
		for(j = 0 ; j < 10000 ; j++) {
			memcpy(TMP, M, 16);
			rijndael_decrypt(&aesctx, C, M);
			memcpy(C, M, 16);
		}
		
		if(memcmp(M, AES_DECRYPT_ECB_MCT256_PLAINTEXT[i], 16) != 0) {
			printf("(%s) : Plain error [%d]\n", __FUNCTION__, i);
			return 0;
		}
		
		memcpy(K, TMP, 16);
		memcpy(&K[16], M, 16);
		xor256(K, K, KS)
	}
	
	return 1;
}

int main() {
	if(aes_encrypt_ecb())
		printf("aes_encrypt_ecb OK\n");
	if(aes_decrypt_ecb())
		printf("aes_decrypt_ecb OK\n");
	return 0;
}


