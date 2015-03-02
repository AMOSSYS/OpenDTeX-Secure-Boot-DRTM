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
 * \file loadkey.c
 * \brief Functions to load a key into the TPM volatile memory under a specific parent.
 * \author Goulven Guiheux
 *
 * 
 */

#include <uc/uc.h>
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"
#include "crypto/sha.h"

TSS_RESULT STOREKEY_AddKey(UINT32 locality, TPM_KEY_HANDLE handle, char * name);

TSS_RESULT LIBTPMCALL TPM_LoadKey(UINT32 locality, BYTE * keyBlob, UINT32 keyBlobLen, char * keyName, char * parentName, char * password, UINT32 sizepwd) {
	TSS_RESULT 		result;
	TPM_AUTH 		auth;
	TPM_KEY_HANDLE	hParent;
	TPM_KEY_HANDLE	hKey;
	TPM_AUTHDATA	hashSecret;
	
	memset(hashSecret.authdata, 0, sizeof(hashSecret.authdata));
	memset(&auth, 0, sizeof(auth));
	
	if(keyBlob == NULL) {
		LOGERROR("keyBlob = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
	
	if(keyName == NULL) {
		LOGERROR("keyName = null pointer");
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
	
	result = TPM_GetKeyHandle(keyName, &hKey);
	if(result == TSS_SUCCESS) {
		LOGERROR("%s already loaded", keyName);
		return TCSERR(TCS_E_KEY_MISMATCH);
	}

	// OIAP session
	result = TCS_OIAP(locality, &auth);
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_OIAP returns %x", result);
		return result;
	}
	
	if(password && sizepwd && password[0]) {
		struct USHAContext ctx_sha1;

		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, (uint8_t *) password, strlen(password));
		USHAResult(&ctx_sha1, hashSecret.authdata);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	}

	result = TCS_LoadKey2(locality, hParent, &hashSecret, keyBlob, keyBlobLen, &auth, &hKey);
	
	// Burn data
	memset(&auth, 0, sizeof(auth));
	memset(hashSecret.authdata, 0, sizeof(hashSecret.authdata));
	
	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_LoadKey2 returns %x", result);
		return result;
	}

	return STOREKEY_AddKey(locality, hKey, keyName);
}

