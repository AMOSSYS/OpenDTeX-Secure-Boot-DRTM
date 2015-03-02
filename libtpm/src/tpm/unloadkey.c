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
 * \file unloadkey.c
 * \brief Functions to unload a key from the TPM volatile memory.
 * \author Goulven Guiheux
 *
 * 
 */

#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"

extern TSS_RESULT STOREKEY_RemoveKey(TPM_KEY_HANDLE handle);

TSS_RESULT LIBTPMCALL TPM_UnloadKey(UINT32 locality, char * keyName) {
	TSS_RESULT 		result;
	TPM_KEY_HANDLE	hKey;
	
	result = TPM_GetKeyHandle(keyName, &hKey);
	if(result) {
		LOGERROR("%s not found", keyName);
		return TCSERR(TCS_E_KEY_MISMATCH);
	}

	result = TCS_FlushSpecific(locality, hKey, TPM_RT_KEY);

	if(result != TSS_SUCCESS) {
		LOGDEBUG("TCS_FlushSpecific returns %x\n", result);
		return result;
	}

	return STOREKEY_RemoveKey(hKey);
}

