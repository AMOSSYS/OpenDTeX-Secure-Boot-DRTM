// -*- coding: utf-8 -*-

/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004-2007
 *
 */
 
#include <uc/uc.h>
#include <libtpm/libtpm.h>
#include <libtpm/tpmutils.h>
#include "core.h"

TSS_RESULT LIBTPMCALL TCS_PcrRead(UINT32 locality, TPM_PCRINDEX pcrNum, TPM_PCRVALUE * outDigest)
{
	UINT64 offset = 0;
	TSS_RESULT result;
	UINT32 paramSize;
	BYTE txBlob[TSS_TPM_TXBLOB_SIZE];

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	LOGDEBUG("Entering TCS_PcrRead");

	//TODO GGX
	/* PCRs are numbered 0 - (NUM_PCRS - 1), thus the >= */
	//if (pcrNum >= tpm_metrics.num_pcrs)
	//	return TCSERR(TSS_E_BAD_PARAMETER);

   result = tpm_rqu_build(TPM_ORD_PcrRead, &offset, txBlob, pcrNum, NULL);
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
	if(! result) {
		result = tpm_rsp_parse(TPM_ORD_PcrRead, txBlob, paramSize, NULL, outDigest->digest);
	}
	
	LOGDEBUG("Exiting TCS_PcrRead : %x", result);
	
	return result;
}

TSS_RESULT LIBTPMCALL TCS_Extend(UINT32 locality, TPM_PCRINDEX pcrNum, TPM_PCRVALUE * inDigest, TPM_PCRVALUE * outDigest)
{
	UINT64 offset = 0;
	TSS_RESULT result;
	UINT32 paramSize;
	BYTE txBlob[TSS_TPM_TXBLOB_SIZE];

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	LOGDEBUG("Entering TCS_Extend");

	//TODO GGX
	/* PCRs are numbered 0 - (NUM_PCRS - 1), thus the >= */
	//if (pcrNum >= tpm_metrics.num_pcrs)
	//	return TCSERR(TSS_E_BAD_PARAMETER);

   result = tpm_rqu_build(TPM_ORD_Extend, &offset, txBlob, pcrNum, TPM_DIGEST_SIZE, inDigest->digest, NULL, NULL);
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
	if(! result) {
		result = tpm_rsp_parse(TPM_ORD_Extend, txBlob, paramSize, NULL, outDigest->digest);
	}
	
	LOGDEBUG("Exiting TCS_Extend : %x", result);
	
	return result;
}

