// -*- coding: utf-8 -*-

/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004
 *
 */
 
#include <uc/uc.h>
#include <libtpm/libtpm.h>
#include  <libtpm/tpmutils.h>
#include "core.h"
#include "crypto/sha.h"
 
TSS_RESULT LIBTPMCALL TCS_OIAP(UINT32 locality, TPM_AUTH * auth)
{
	UINT64 offset = 0;
	TSS_RESULT result;
	UINT32 paramSize;
	BYTE txBlob[TSS_TPM_TXBLOB_SIZE];

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }

	LOGDEBUG("Entering TCS_OIAP");

   result = tpm_rqu_build(TPM_ORD_OIAP, &offset, txBlob, NULL);
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
		result = tpm_rsp_parse(TPM_ORD_OIAP, txBlob, paramSize, &auth->AuthHandle, auth->NonceEven.nonce);
	}

	LOGDEBUG("Exiting TCS_OIAP : %x", result);
	
	return result;
}

TSS_RESULT LIBTPMCALL TCS_OSAP(UINT32 locality, TPM_AUTH * authOSAP, TPM_AUTH * auth, TPM_ENTITY_TYPE entityType, UINT32 entityValue)
{
	UINT64 offset = 0;
	TSS_RESULT result;
	UINT32 paramSize;
	BYTE txBlob[TSS_TPM_TXBLOB_SIZE];

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	LOGDEBUG("Entering TCS_OSAP");

   result = tpm_rqu_build(TPM_ORD_OSAP, &offset, txBlob, entityType, entityValue, authOSAP->NonceOdd.nonce);
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
		result = tpm_rsp_parse(TPM_ORD_OSAP, txBlob, paramSize, &auth->AuthHandle, auth->NonceEven.nonce, authOSAP->NonceEven.nonce);
	}
	
	LOGDEBUG("Exiting TCS_OSAP : %x", result);

	return result;
}

TSS_RESULT tcs_compute_auth(UINT32 locality, TPM_AUTH * auth, TPM_NONCE * h1, TPM_AUTHDATA * secret) {
  	struct HMACContext	ctx_hmac;
	TSS_RESULT           result;
	
	result = LIBTPM_GetRand(locality, auth->NonceOdd.nonce, sizeof(auth->NonceOdd.nonce));
	if(result != TSS_SUCCESS) {
	   LOGERROR("Can't randomize NonceOdd");
	   return result;
	}
	
	hmacReset(&ctx_hmac, SHA1, secret->authdata, sizeof(secret->authdata));
	hmacInput(&ctx_hmac, h1->nonce, sizeof(h1->nonce));
	hmacInput(&ctx_hmac, auth->NonceEven.nonce, sizeof(auth->NonceEven.nonce));
	hmacInput(&ctx_hmac, auth->NonceOdd.nonce, sizeof(auth->NonceOdd.nonce));
	hmacInput(&ctx_hmac, (const unsigned char *) &auth->fContinueAuthSession, sizeof(auth->fContinueAuthSession));
	hmacResult(&ctx_hmac, auth->HMAC.authdata);
	memset(&ctx_hmac, 0, sizeof(ctx_hmac));
	
	return TSS_SUCCESS;
}

TSS_RESULT tcs_check_auth(TPM_AUTH * auth, TPM_NONCE * h1, TPM_AUTHDATA * secret) {
	struct HMACContext	ctx_hmac;
	TPM_AUTHDATA		   resAuth;
	
	memset(&resAuth, 0, TPM_SHA1_160_HASH_LEN);
	
	hmacReset(&ctx_hmac, SHA1, secret->authdata, sizeof(secret->authdata));
	hmacInput(&ctx_hmac, h1->nonce, sizeof(h1->nonce));
	hmacInput(&ctx_hmac, auth->NonceEven.nonce, sizeof(auth->NonceEven.nonce));
	hmacInput(&ctx_hmac, auth->NonceOdd.nonce, sizeof(auth->NonceOdd.nonce));
	hmacInput(&ctx_hmac, (const unsigned char *) &auth->fContinueAuthSession, sizeof(auth->fContinueAuthSession));
	hmacResult(&ctx_hmac, (uint8_t *) &resAuth);
	memset(&ctx_hmac, 0, sizeof(ctx_hmac));


	if(memcmp((char *)&resAuth, (char *)auth->HMAC.authdata, sizeof(auth->HMAC.authdata))) {
		memset(&resAuth, 0, TPM_SHA1_160_HASH_LEN);
		LOGDEBUG("auth checking failed");
		return TCSERR(TSS_E_TSP_AUTHFAIL);
	}

	memset(&resAuth, 0, TPM_SHA1_160_HASH_LEN);
	
	return TSS_SUCCESS;
}

