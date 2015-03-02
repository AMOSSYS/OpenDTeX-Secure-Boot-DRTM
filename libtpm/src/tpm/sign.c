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
 * \file createkey.c
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

TSS_RESULT LIBTPMCALL TPM_Sign(UINT32 locality, BYTE ** sigBlobData, UINT32 * sigBlobSize, char * parentName, char * pwdParent, UINT32 size_pwdParent, UINT32 dataSize, BYTE * data) {
	TSS_RESULT 		result;
	TPM_KEY_HANDLE	hParent;
	TPM_AUTH 		auth;
	TPM_AUTHDATA	hashSecret;
	TPM_DIGEST     hash;
	UINT64         offset = 0;
	BYTE           tpmsigninfo[sizeof(TPM_SIGN_INFO) + sizeof(hash.digest)];
	
	memset(&auth, 0, sizeof(auth));
	memset(hashSecret.authdata, 0, sizeof(hashSecret.authdata));
	
	if(sigBlobData == NULL) {
		LOGERROR("sigBlobData = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(sigBlobSize == NULL) {
		LOGERROR("sigBlobSize = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(data == NULL) {
		LOGERROR("data = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(dataSize == 0) {
		LOGERROR("dataSize = 0");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(parentName == NULL) {
		LOGERROR("parentName = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	result = TPM_GetKeyHandle(parentName, &hParent);
	if(result) {
		LOGERROR("%s not found", parentName);
		return result;
	}
	
	// Hash data and create TPM_SIGN_INFO structure
	{
	   //TPM_NONCE            nonce;
	   struct USHAContext   ctx_sha1;
	   

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, data, dataSize);
		USHAResult(&ctx_sha1, hash.digest);
	   
	   // Trousers can't verify TPM_SS_RSASSAPKCS1v15_INFO scheme
	   /*LIBTPM_GetRand(locality, nonce.nonce, sizeof(nonce.nonce));
	
	   LoadBlob_UINT16(&offset, TPM_TAG_SIGNINFO, tpmsigninfo);
	   LoadBlob(&offset, 4, tpmsigninfo, (BYTE *) "DTEX");
	   LoadBlob(&offset, sizeof(nonce.nonce), tpmsigninfo, nonce.nonce);
	   LoadBlob_UINT32(&offset, dataSize, tpmsigninfo);
	   LoadBlob(&offset, sizeof(hash.digest), tpmsigninfo, hash.digest);*/
	   LoadBlob(&offset, sizeof(hash.digest), tpmsigninfo, hash.digest);
	}
	
	// OIAP session
	result = TCS_OIAP(locality, &auth);
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_OIAP returns %x", result);
		return result;
	}
	
	if(pwdParent && size_pwdParent && pwdParent[0]) {
		struct USHAContext ctx_sha1;

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) pwdParent, strlen(pwdParent));
		USHAResult(&ctx_sha1, hashSecret.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}

	result = TCS_Sign(locality, hParent, &hashSecret, offset, tpmsigninfo, &auth, sigBlobSize, sigBlobData);
	
	// Burn data
	memset(&auth, 0, sizeof(auth));
	memset(hashSecret.authdata, 0, sizeof(hashSecret.authdata));
	
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_Sign returns %x", result);
		return result;
	}

	return result;
}

