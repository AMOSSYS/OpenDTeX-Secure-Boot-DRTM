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


/**
 * \file unseal.c
 * \brief Functions to apply seal operation (conditional ciphering) to a data binary.
 * \author Goulven Guiheux
 *
 * 
 */

#include <uc/uc.h>
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"
#include "crypto/sha.h"

TSS_RESULT LIBTPMCALL TPM_Seal(UINT32 locality, BYTE ** sealedDataBlob, UINT32 * sealedDataBlobSize, BYTE * dataBlob, UINT32 dataBlobSize, char * parentName, char * parentPwd, UINT32 size_parentPwd, char * dataPwd, UINT32 size_dataPwd, BYTE * pcrInfo, UINT32 pcrInfoSize) {
	TSS_RESULT 		result;
	TPM_KEY_HANDLE	hParent;
	TPM_AUTH 		authOSAP;
	TPM_AUTH 		auth;
	TPM_AUTHDATA   hashOSAP;
	TPM_AUTHDATA	hashSecret;
	TPM_ENCAUTH    encDataAuth;
	
	if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	memset(&authOSAP, 0, sizeof(authOSAP));
	memset(&hashOSAP, 0, sizeof(hashOSAP));
	memset(&hashSecret, 0, sizeof(hashSecret));
	memset(&auth, 0, sizeof(auth));
	memset(&encDataAuth, 0, sizeof(encDataAuth));
	
	if(sealedDataBlob == NULL) {
		LOGERROR("sealedDataBlob = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(sealedDataBlobSize == NULL) {
		LOGERROR("sealedDataBlobSize = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(dataBlob == NULL) {
		LOGERROR("dataBlob = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}

	if(parentName == NULL) {
		LOGERROR("parentName = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	result = TPM_GetKeyHandle(parentName, &hParent);
	if(result) {
		LOGERROR("%s not found", parentName);
		return TCSERR(TCS_E_KEY_MISMATCH);
	}
	
	// OSAP session
	result = LIBTPM_GetRand(locality, authOSAP.NonceOdd.nonce, sizeof(authOSAP.NonceOdd.nonce));
	if(result) {
		return result;
	}
	result = TCS_OSAP(locality, &authOSAP, &auth, TPM_ET_KEYHANDLE, hParent);
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_OSAP returns %x", result);
		return result;
	}
	
	if(parentPwd && size_parentPwd && parentPwd[0]) {
		struct USHAContext ctx_sha1;

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) parentPwd, strlen(parentPwd));
		USHAResult(&ctx_sha1, hashSecret.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}

   // Compute the shared secret of OSAP session
	{
		struct HMACContext	ctx_hmac;
		
		hmacReset(&ctx_hmac, SHA1, hashSecret.authdata, sizeof(hashSecret.authdata));
		hmacInput(&ctx_hmac, authOSAP.NonceEven.nonce, sizeof(authOSAP.NonceEven.nonce));
		hmacInput(&ctx_hmac, authOSAP.NonceOdd.nonce, sizeof(authOSAP.NonceOdd.nonce));
		hmacResult(&ctx_hmac, hashOSAP.authdata);
		memset(&ctx_hmac, 0, sizeof(ctx_hmac));
	}
	
	if(dataPwd && size_dataPwd && dataPwd[0]) {
		struct USHAContext ctx_sha1;

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) dataPwd, strlen(dataPwd));
		USHAResult(&ctx_sha1, encDataAuth.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}
	
	// Encrypt encDataAuth with ADIP protocol (using XOR algorithm ... TODO add support of AES128-CTR (if supported by TPM))
	{
		struct USHAContext ctx_sha1;
		TPM_ENCAUTH    xorEncrypt;
		unsigned int   i;

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, hashOSAP.authdata, sizeof(hashOSAP.authdata));
		USHAInput(&ctx_sha1, auth.NonceEven.nonce, sizeof(auth.NonceEven.nonce));
		USHAResult(&ctx_sha1, xorEncrypt.authdata);
		
		for(i = 0 ; i < sizeof(encDataAuth.authdata) ; i++) {
		   encDataAuth.authdata[i] ^= xorEncrypt.authdata[i];
		}
		
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
		memset(xorEncrypt.authdata, 0, sizeof(xorEncrypt.authdata));
	}
	
	result = TCS_Seal(locality, hParent, &hashOSAP, &encDataAuth, pcrInfoSize, pcrInfo, dataBlobSize, dataBlob, &auth, sealedDataBlobSize, sealedDataBlob);

   // Burn data
	memset(&authOSAP, 0, sizeof(authOSAP));
	memset(&hashOSAP, 0, sizeof(hashOSAP));
	memset(&hashSecret, 0, sizeof(hashSecret));
	memset(&auth, 0, sizeof(auth));
	memset(&encDataAuth, 0, sizeof(encDataAuth));

   if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_Seal returns %x", result);
		return result;
	}
	
	return result;
}

