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


#include <uc/uc.h>
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"

TSS_RESULT LIBTPMCALL TCS_FlushSpecific(UINT32 locality, TPM_KEY_HANDLE handle, TPM_RESOURCE_TYPE type) {
	UINT64 			 	offset = 0;
	TSS_RESULT 	 		result;
	UINT32 		 		paramSize;
	BYTE 		 			txBlob[TSS_TPM_TXBLOB_SIZE];

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	LOGDEBUG("Entering TCS_FlushSpecific");

	// Communication with TPM
	result = tpm_rqu_build(TPM_ORD_FlushSpecific, &offset, txBlob, handle, type);
	if(result) {
		LOGDEBUG("tpm_rqu_build returns %x", result);
		return result;
	}

   UnloadBlob_Header(txBlob, &paramSize);
	result = tpm_io(locality, txBlob, paramSize, txBlob, sizeof(txBlob));
	if(result) {
	   result = TDDLERR(result);
		LOGERROR("tpm_io returns %x", result);
		return result;
	}

	result = UnloadBlob_Header(txBlob, &paramSize);
	if (! result) {
		result = tpm_rsp_parse(TPM_ORD_FlushSpecific, txBlob, paramSize, NULL);
	}

	LOGDEBUG("Exiting TCS_FlushSpecific : %x", result);

	return result;
}

