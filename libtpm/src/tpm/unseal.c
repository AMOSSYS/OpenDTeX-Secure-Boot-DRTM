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
 * \brief Functions to apply unseal operation (conditional deciphering) to a data binary.
 * \author Goulven Guiheux
 *
 * 
 */

#include <uc/uc.h>
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"
#include "crypto/sha.h"

TSS_RESULT LIBTPMCALL TPM_Unseal(UINT32 locality, BYTE ** dataBlob, UINT32 * dataBlobSize, BYTE * sealedBlob, UINT32 sealedBlobSize, char * keyName, char * pwdKey, UINT32 size_pwdKey, char * pwdData, UINT32 size_pwdData) {
	TSS_RESULT 		result;
	TPM_KEY_HANDLE	hKey;
	TPM_AUTH 		authOSAP;
	TPM_AUTH 		authKey;
	TPM_AUTH 		authData;
	TPM_AUTHDATA   hashOSAP;
	TPM_AUTHDATA   hashKey;
	TPM_AUTHDATA   hashData;
	
	if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	memset(&authOSAP, 0, sizeof(authOSAP));
	memset(&authKey, 0, sizeof(authKey));
	memset(&authData, 0, sizeof(authData));
	memset(hashOSAP.authdata, 0, sizeof(hashOSAP.authdata));
	memset(hashKey.authdata, 0, sizeof(hashKey.authdata));
	memset(hashData.authdata, 0, sizeof(hashData.authdata));
	
	if(dataBlob == NULL) {
		LOGERROR("dataBlob = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(dataBlobSize == NULL) {
		LOGERROR("dataBlobSize = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(sealedBlob == NULL) {
		LOGERROR("sealedBlob = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}

	if(keyName == NULL) {
		LOGERROR("keyName = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	result = TPM_GetKeyHandle(keyName, &hKey);
	if(result) {
		LOGERROR("%s not found", keyName);
		return TCSERR(TCS_E_KEY_MISMATCH);
	}
	
	// OSAP session for the key
	result = LIBTPM_GetRand(locality, authOSAP.NonceOdd.nonce, sizeof(authOSAP.NonceOdd.nonce));
	if(result) {
		return result;
	}
	result = TCS_OSAP(locality, &authOSAP, &authKey, TPM_ET_KEYHANDLE, hKey);
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_OSAP returns %x", result);
		return result;
	}
	
	// OIAP session for data
	result = TCS_OIAP(locality, &authData);
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_OIAP returns %x\n", result);
		return result;
	}
	
	
	if(pwdKey && size_pwdKey && pwdKey[0]) {
		struct USHAContext ctx_sha1;
		
		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) pwdKey, strlen(pwdKey));
		USHAResult(&ctx_sha1, hashKey.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}
	
	if(pwdData && size_pwdData && pwdData[0]) {
		struct USHAContext ctx_sha1;
		
		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) pwdData, strlen(pwdData));
		USHAResult(&ctx_sha1, hashData.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}

	// Compute the shared secret of OSAP session
	{
		struct HMACContext	ctx_hmac;
		
		hmacReset(&ctx_hmac, SHA1, hashKey.authdata, sizeof(hashKey.authdata));
		hmacInput(&ctx_hmac, authOSAP.NonceEven.nonce, sizeof(authOSAP.NonceEven.nonce));
		hmacInput(&ctx_hmac, authOSAP.NonceOdd.nonce, sizeof(authOSAP.NonceOdd.nonce));
		hmacResult(&ctx_hmac, hashOSAP.authdata);
		memset(&ctx_hmac, 0, sizeof(ctx_hmac));
	}

	result = TCS_Unseal(locality, hKey, &hashOSAP, sealedBlob, sealedBlobSize, &hashData, &authKey, &authData, dataBlob, dataBlobSize);
	
	// Burn data
	memset(hashData.authdata, 0, sizeof(hashData.authdata));
	memset(&authData, 0, sizeof(authData));
	memset(hashKey.authdata, 0, sizeof(hashKey.authdata));
	memset(&authKey, 0, sizeof(authKey));
	memset(hashOSAP.authdata, 0, sizeof(hashOSAP.authdata));
	memset(&authOSAP, 0, sizeof(authOSAP));
	
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_Unseal returns %x\n", result);
	}
	
	return result;
}

