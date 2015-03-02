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
#include "crypto/sha.h"

TSS_RESULT LIBTPMCALL TCS_Sign(
                           UINT32         locality, 
                           TPM_KEY_HANDLE hParent,
                           TPM_AUTHDATA * parentAuth,
                           UINT32         dataSize,
                           BYTE *         data,
                           TPM_AUTH *     auth,
                           UINT32 *       sigBlobSize,
                           BYTE **        sigBlobData) {
                           
   UINT64 			 	offset = 0;
	TSS_RESULT 	 		result;
	UINT32 		 		paramSize;
	BYTE 		 			txBlob[TSS_TPM_TXBLOB_SIZE];
	TPM_COMMAND_CODE 	ordinal = TPM_ORD_Sign;
	BYTE 		 			bigendian_ordinal[sizeof(ordinal)];
	BYTE 		 			bigendian_dataSize[sizeof(dataSize)];
	BYTE 		 			bigendian_result[sizeof(result)];
	TPM_NONCE 	 		h1;
	TPM_NONCE 	 		h1Check;
	struct USHAContext ctx_sha1;
	
	if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }

	LOGDEBUG("Entering TCS_Sign");

   // Generate H1
	USHAReset(&ctx_sha1, SHA1);
	UINT32ToArray(ordinal, bigendian_ordinal);
	UINT32ToArray(dataSize, bigendian_dataSize);
	USHAInput(&ctx_sha1, bigendian_ordinal, sizeof(bigendian_ordinal));
	USHAInput(&ctx_sha1, bigendian_dataSize, sizeof(bigendian_dataSize));
	USHAInput(&ctx_sha1, data, dataSize);
	USHAResult(&ctx_sha1, (uint8_t *) &h1);
	memset(&ctx_sha1, 0, sizeof(ctx_sha1));
	
	// Compute AUTH
	result = tcs_compute_auth(locality, auth, &h1, parentAuth);
	if(result) {
		return result;
	}
	
	// Communication with TPM
	result = tpm_rqu_build(TPM_ORD_Sign, &offset, txBlob, hParent, dataSize, data, auth);
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
		result = tpm_rsp_parse(TPM_ORD_Sign, txBlob, paramSize, sigBlobSize, sigBlobData, auth, NULL);
	}
	
	if(! result) {
		// Check auth value
		UINT32ToArray(result, bigendian_result);
		UINT32ToArray(*sigBlobSize, bigendian_dataSize);
		USHAReset(&ctx_sha1, SHA1);
		USHAInput(&ctx_sha1, bigendian_result, sizeof(bigendian_result));
		USHAInput(&ctx_sha1, bigendian_ordinal, sizeof(bigendian_ordinal));
		USHAInput(&ctx_sha1, bigendian_dataSize, sizeof(bigendian_dataSize));
		USHAInput(&ctx_sha1, *sigBlobData, *sigBlobSize);
		USHAResult(&ctx_sha1, (uint8_t *) &h1Check);
		memset(&ctx_sha1, 0, sizeof(ctx_sha1));
		
		result = tcs_check_auth(auth, &h1Check, parentAuth);
		if(result) {
		   free(*sigBlobData);
		   *sigBlobData = 0;
		}
	}

	LOGDEBUG("Exiting TCS_Sign : %x", result);

	return result;
}

