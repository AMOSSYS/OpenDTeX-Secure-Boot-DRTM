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
#include "libtpm/libtpm.h"
#include "libtpm/tpmutils.h"
#include "core.h"
#include "crypto/sha.h"

const TCS_CONTEXT_HANDLE InternalContext = 0x30000000;
const TSS_UUID SRK_UUID = TSS_UUID_SRK;

UINT16
Decode_UINT16(BYTE * in)
{
	UINT16 temp = 0;
	temp = (in[1] & 0xFF);
	temp |= (in[0] << 8);
	return temp;
}

void
UINT64ToArray(UINT64 i, BYTE * out)
{
	out[0] = (BYTE) ((i >> 56) & 0xFF);
	out[1] = (BYTE) ((i >> 48) & 0xFF);
	out[2] = (BYTE) ((i >> 40) & 0xFF);
	out[3] = (BYTE) ((i >> 32) & 0xFF);
	out[4] = (BYTE) ((i >> 24) & 0xFF);
	out[5] = (BYTE) ((i >> 16) & 0xFF);
	out[6] = (BYTE) ((i >> 8) & 0xFF);
	out[7] = (BYTE) (i & 0xFF);
}

void
UINT32ToArray(UINT32 i, BYTE * out)
{
	out[0] = (BYTE) ((i >> 24) & 0xFF);
	out[1] = (BYTE) ((i >> 16) & 0xFF);
	out[2] = (BYTE) ((i >> 8) & 0xFF);
	out[3] = (BYTE) (i & 0xFF);
}

void
UINT16ToArray(UINT16 i, BYTE * out)
{
	out[0] = (BYTE) ((i >> 8) & 0xFF);
	out[1] = (BYTE) (i & 0xFF);
}

UINT32
Decode_UINT32(BYTE * y)
{
	UINT32 x = 0;

	x = y[0];
	x = ((x << 8) | (y[1] & 0xFF));
	x = ((x << 8) | (y[2] & 0xFF));
	x = ((x << 8) | (y[3] & 0xFF));

	return x;
}

UINT64
Decode_UINT64(BYTE *y)
{
	UINT64 x = 0;

	x = y[0];
	x = ((x << 8) | (y[1] & 0xFF));
	x = ((x << 8) | (y[2] & 0xFF));
	x = ((x << 8) | (y[3] & 0xFF));
	x = ((x << 8) | (y[4] & 0xFF));
	x = ((x << 8) | (y[5] & 0xFF));
	x = ((x << 8) | (y[6] & 0xFF));
	x = ((x << 8) | (y[7] & 0xFF));

	return x;
}

void
LoadBlob_UINT64(UINT64 *offset, UINT64 in, BYTE * blob)
{
	if (blob)
		UINT64ToArray(in, &blob[*offset]);
	*offset += sizeof(UINT64);
}

void
LoadBlob_UINT32(UINT64 *offset, UINT32 in, BYTE * blob)
{
	if (blob)
		UINT32ToArray(in, &blob[*offset]);
	*offset += sizeof(UINT32);
}

void
LoadBlob_UINT16(UINT64 *offset, UINT16 in, BYTE * blob)
{
	if (blob)
		UINT16ToArray(in, &blob[*offset]);
	*offset += sizeof(UINT16);
}

void
UnloadBlob_UINT64(UINT64 *offset, UINT64 * out, BYTE * blob)
{
	if (out)
		*out = Decode_UINT64(&blob[*offset]);
	*offset += sizeof(UINT64);
}

void
UnloadBlob_UINT32(UINT64 *offset, UINT32 * out, BYTE * blob)
{
	if (out)
		*out = Decode_UINT32(&blob[*offset]);
	*offset += sizeof(UINT32);
}

void
UnloadBlob_UINT16(UINT64 *offset, UINT16 * out, BYTE * blob)
{
	if (out)
		*out = Decode_UINT16(&blob[*offset]);
	*offset += sizeof(UINT16);
}

void
LoadBlob_BYTE(UINT64 *offset, BYTE data, BYTE * blob)
{
	if (blob)
		blob[*offset] = data;
	(*offset)++;
}

void
UnloadBlob_BYTE(UINT64 *offset, BYTE * dataOut, BYTE * blob)
{
	if (dataOut)
		*dataOut = blob[*offset];
	(*offset)++;
}

void
LoadBlob_BOOL(UINT64 *offset, TSS_BOOL data, BYTE * blob)
{
	if (blob)
		blob[*offset] = data;
	(*offset)++;
}

void
UnloadBlob_BOOL(UINT64 *offset, TSS_BOOL *dataOut, BYTE * blob)
{
	if (dataOut)
		*dataOut = blob[*offset];
	(*offset)++;
}

void
LoadBlob(UINT64 *offset, UINT32 size, BYTE *container, BYTE *object)
{
	if ((size == 0) || ((*offset + size) > TSS_TPM_TXBLOB_SIZE))
		return;

	if (container)
		memcpy(&container[*offset], object, size);
	(*offset) += (UINT64) size;
}

void
UnloadBlob(UINT64 *offset, UINT32 size, BYTE *container, BYTE *object)
{
	if ((size == 0) || ((*offset + size) > TSS_TPM_TXBLOB_SIZE))
		return;

	if (object)
		memcpy(object, &container[*offset], size);
	(*offset) += (UINT64) size;
}

void
LoadBlob_Header(UINT16 tag, UINT32 paramSize, UINT32 ordinal, BYTE * blob)
{

	UINT16ToArray(tag, &blob[0]);
	LOGDEBUG("Header Tag: %x", tag);
	UINT32ToArray(paramSize, &blob[2]);
	LOGDEBUG("Header ParamSize: %x", paramSize);
	UINT32ToArray(ordinal, &blob[6]);
	LOGDEBUG("Header Ordinal: %x", ordinal);
#if 0
	LogInfo("Blob's TPM Ordinal: 0x%x", ordinal);
#endif
}

#ifdef TSS_DEBUG
TSS_RESULT
LogUnloadBlob_Header(BYTE * blob, UINT32 * size, char *file, int line)
{
	TSS_RESULT result;

	UINT16 temp = Decode_UINT16(blob);
	LOGDEBUG("UnloadBlob_Tag: %x", (temp));
	*size = Decode_UINT32(&blob[2]);
	LOGDEBUG("UnloadBlob_Header, size: %x", *size);
	LOGDEBUG("UnloadBlob_Header, returnCode: %x", Decode_UINT32(&blob[6]));

	if ((result = Decode_UINT32(&blob[6]))) {
		LogTPMERR(result, file, line);
	}

	return result;
}
#else
TSS_RESULT
UnloadBlob_Header(BYTE * blob, UINT32 * size)
{
	UINT16 temp = Decode_UINT16(blob);
	LOGDEBUG("UnloadBlob_Tag: %x", (temp));
	*size = Decode_UINT32(&blob[2]);
	LOGDEBUG("UnloadBlob_Header, size: %x", *size);
	LOGDEBUG("UnloadBlob_Header, returnCode: %x", Decode_UINT32(&blob[6]));
	return Decode_UINT32(&blob[6]);
}
#endif

void
LoadBlob_Auth(UINT64 *offset, BYTE * blob, TPM_AUTH * auth)
{
	LoadBlob_UINT32(offset, auth->AuthHandle, blob);
	LoadBlob(offset, TCPA_NONCE_SIZE, blob, auth->NonceOdd.nonce);
	LoadBlob_BOOL(offset, auth->fContinueAuthSession, blob);
	LoadBlob(offset, TCPA_AUTHDATA_SIZE, blob, (BYTE *)&auth->HMAC);
}

void
UnloadBlob_Auth(UINT64 *offset, BYTE * blob, TPM_AUTH * auth)
{
	if (!auth) {
		UnloadBlob(offset, TCPA_NONCE_SIZE, blob, NULL);
		UnloadBlob_BOOL(offset, NULL, blob);
		UnloadBlob(offset, TCPA_DIGEST_SIZE, blob, NULL);

		return;
	}

	UnloadBlob(offset, TCPA_NONCE_SIZE, blob, auth->NonceEven.nonce);
	UnloadBlob_BOOL(offset, &auth->fContinueAuthSession, blob);
	UnloadBlob(offset, TCPA_DIGEST_SIZE, blob, (BYTE *)&auth->HMAC);
}

void
UnloadBlob_VERSION(UINT64 *offset, BYTE *blob, TPM_VERSION *out)
{
	if (!out) {
		*offset += (sizeof(BYTE) * 4);
		return;
	}

	UnloadBlob_BYTE(offset, &out->major, blob);
	UnloadBlob_BYTE(offset, &out->minor, blob);
	UnloadBlob_BYTE(offset, &out->revMajor, blob);
	UnloadBlob_BYTE(offset, &out->revMinor, blob);
}

void
LoadBlob_VERSION(UINT64 *offset, BYTE *blob, TPM_VERSION *ver)
{
	LoadBlob_BYTE(offset, ver->major, blob);
	LoadBlob_BYTE(offset, ver->minor, blob);
	LoadBlob_BYTE(offset, ver->revMajor, blob);
	LoadBlob_BYTE(offset, ver->revMinor, blob);
}

void
UnloadBlob_TCPA_VERSION(UINT64 *offset, BYTE *blob, TCPA_VERSION *out)
{
	if (!out) {
		*offset += (sizeof(BYTE) * 4);
		return;
	}

	UnloadBlob_BYTE(offset, &out->major, blob);
	UnloadBlob_BYTE(offset, &out->minor, blob);
	UnloadBlob_BYTE(offset, &out->revMajor, blob);
	UnloadBlob_BYTE(offset, &out->revMinor, blob);
}

void
LoadBlob_TCPA_VERSION(UINT64 *offset, BYTE *blob, TCPA_VERSION *ver)
{
	LoadBlob_BYTE(offset, ver->major, blob);
	LoadBlob_BYTE(offset, ver->minor, blob);
	LoadBlob_BYTE(offset, ver->revMajor, blob);
	LoadBlob_BYTE(offset, ver->revMinor, blob);
}

TSS_RESULT
UnloadBlob_TPM_RSA_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_RSA_KEY_PARMS *out, UINT32 sizeout)
{
	if (!out) {
		*offset += sizeout;
		return TSS_SUCCESS;
	}
   
   if(sizeout < 12) {
      LOGERROR("not enough bytes in TPM_RSA_KEY_PARMS structure.");
      return TCSERR(TSS_E_BAD_PARAMETER);
   }
   
	UnloadBlob_UINT32(offset, &out->keyLength, blob);
	UnloadBlob_UINT32(offset, &out->numPrimes, blob);
	UnloadBlob_UINT32(offset, &out->exponentSize, blob);
	
	if(sizeout != *offset + out->exponentSize) {
	   LOGERROR("incorrect size of TPM_RSA_KEY_PARMS structure.");
      return TCSERR(TSS_E_BAD_PARAMETER);
	}

   UnloadBlob(offset, out->exponentSize, blob, out->exponent);
   
   return TSS_SUCCESS;
}

TSS_RESULT
UnloadBlob_TPM_SYMMETRIC_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_SYMMETRIC_KEY_PARMS *out, UINT32 sizeout)
{
	if (!out) {
		*offset += sizeout;
		return TSS_SUCCESS;
	}
   
   if(sizeout < 12) {
      LOGERROR("not enough bytes in TPM_SYMMETRIC_KEY_PARMS structure.");
      return TCSERR(TSS_E_BAD_PARAMETER);
   }
   
	UnloadBlob_UINT32(offset, &out->keyLength, blob);
	UnloadBlob_UINT32(offset, &out->blockSize, blob);
	UnloadBlob_UINT32(offset, &out->ivSize, blob);
	
	if(sizeout != *offset + out->ivSize) {
	   LOGERROR("incorrect size of TPM_SYMMETRIC_KEY_PARMS structure.");
      return TCSERR(TSS_E_BAD_PARAMETER);
	}

   UnloadBlob(offset, out->ivSize, blob, out->IV);
   
   return TSS_SUCCESS;
}

TSS_RESULT
UnloadBlob_KEY_PARMS(UINT64 *offset, BYTE *blob, TCPA_KEY_PARMS *keyParms)
{
	if (!keyParms) {
		UINT32 parmSize;

		UnloadBlob_UINT32(offset, NULL, blob);
		UnloadBlob_UINT16(offset, NULL, blob);
		UnloadBlob_UINT16(offset, NULL, blob);
		UnloadBlob_UINT32(offset, &parmSize, blob);

		if (parmSize > 0)
			UnloadBlob(offset, parmSize, blob, NULL);

		return TSS_SUCCESS;
	}

	UnloadBlob_UINT32(offset, &keyParms->algorithmID, blob);
	UnloadBlob_UINT16(offset, &keyParms->encScheme, blob);
	UnloadBlob_UINT16(offset, &keyParms->sigScheme, blob);
	UnloadBlob_UINT32(offset, &keyParms->parmSize, blob);

	if (keyParms->parmSize == 0)
		keyParms->parms = NULL;
	else {
	   TSS_RESULT rc;
	   
		keyParms->parms = malloc(keyParms->parmSize);
		if (keyParms->parms == NULL) {
			LOGERROR("malloc of %u bytes failed.", keyParms->parmSize);
			keyParms->parmSize = 0;
			return TCSERR(TSS_E_OUTOFMEMORY);
		}
		
		if(keyParms->algorithmID == TPM_ALG_RSA) {
	      rc = UnloadBlob_TPM_RSA_KEY_PARMS(offset, blob, (TPM_RSA_KEY_PARMS *) keyParms->parms, keyParms->parmSize);
	      if(rc != TSS_SUCCESS) {
	         return rc;
	      }
	   } else if(keyParms->algorithmID == TPM_ALG_AES) {
	      rc = UnloadBlob_TPM_SYMMETRIC_KEY_PARMS(offset, blob, (TPM_SYMMETRIC_KEY_PARMS *) keyParms->parms, keyParms->parmSize);
	      if(rc != TSS_SUCCESS) {
	         return rc;
	      }
	   }
	   
		//UnloadBlob(offset, keyParms->parmSize, blob, keyParms->parms);
	}

	return TSS_SUCCESS;
}

void
UnloadBlob_KEY_FLAGS(UINT64 *offset, BYTE *blob, TCPA_KEY_FLAGS *flags)
{
	if (!flags) {
		UnloadBlob_UINT32(offset, NULL, blob);

		return;
	}

	UnloadBlob_UINT32(offset, flags, blob);
}

TSS_RESULT
UnloadBlob_CERTIFY_INFO(UINT64 *offset, BYTE *blob, TCPA_CERTIFY_INFO *certify)
{
	TSS_RESULT rc;

	if (!certify) {
		TPM_VERSION version;
		UINT32 size;

		UnloadBlob_VERSION(offset, blob, &version);
		UnloadBlob_UINT16(offset, NULL, blob);
		UnloadBlob_KEY_FLAGS(offset, blob, NULL);
		UnloadBlob_BOOL(offset, NULL, blob);

		if ((rc = UnloadBlob_KEY_PARMS(offset, blob, NULL)))
			return rc;

		UnloadBlob(offset, TCPA_DIGEST_SIZE, blob, NULL);
		UnloadBlob(offset, TCPA_NONCE_SIZE, blob, NULL);
		UnloadBlob_BOOL(offset, NULL, blob);
		UnloadBlob_UINT32(offset, &size, blob);

		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);

		if (Decode_UINT16((BYTE *) &version) == TPM_TAG_CERTIFY_INFO2){
			/* This is a TPM_CERTIFY_INFO2 structure. */
			/* Read migrationAuthority. */
			UnloadBlob_UINT32(offset, &size, blob);
			if (size > 0)
				UnloadBlob(offset, size, blob, NULL);
		}

		return TSS_SUCCESS;
	}

	UnloadBlob_VERSION(offset, blob, (TPM_VERSION *)&certify->version);
	UnloadBlob_UINT16(offset, &certify->keyUsage, blob);
	UnloadBlob_KEY_FLAGS(offset, blob, &certify->keyFlags);
	UnloadBlob_BOOL(offset, (TSS_BOOL *)&certify->authDataUsage, blob);

	if ((rc = UnloadBlob_KEY_PARMS(offset, blob, &certify->algorithmParms)))
		return rc;

	UnloadBlob(offset, TCPA_DIGEST_SIZE, blob, certify->pubkeyDigest.digest);
	UnloadBlob(offset, TCPA_NONCE_SIZE, blob, certify->data.nonce);
	UnloadBlob_BOOL(offset, (TSS_BOOL *)&certify->parentPCRStatus, blob);
	UnloadBlob_UINT32(offset, &certify->PCRInfoSize, blob);

	if (certify->PCRInfoSize > 0) {
		certify->PCRInfo = (BYTE *)malloc(certify->PCRInfoSize);
		if (certify->PCRInfo == NULL) {
			LOGERROR("malloc of %u bytes failed.", certify->PCRInfoSize);
			certify->PCRInfoSize = 0;
			free(certify->algorithmParms.parms);
			certify->algorithmParms.parms = NULL;
			certify->algorithmParms.parmSize = 0;
			return TCSERR(TSS_E_OUTOFMEMORY);
		}
		UnloadBlob(offset, certify->PCRInfoSize, blob, certify->PCRInfo);
	} else {
		certify->PCRInfo = NULL;
	}

	if (Decode_UINT16((BYTE *) &certify->version) == TPM_TAG_CERTIFY_INFO2){
		/* This is a TPM_CERTIFY_INFO2 structure. */
		/* Read migrationAuthority. */
		UINT32 size;
		UnloadBlob_UINT32(offset, &size, blob);
		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);
	}

	return TSS_SUCCESS;
}

TSS_RESULT
UnloadBlob_KEY_HANDLE_LIST(UINT64 *offset, BYTE *blob, TCPA_KEY_HANDLE_LIST *list)
{
	UINT16 i;

	if (!list) {
		UINT16 size;

		UnloadBlob_UINT16(offset, &size, blob);

		*offset += (size * sizeof(UINT32));

		return TSS_SUCCESS;
	}

	UnloadBlob_UINT16(offset, &list->loaded, blob);
	if (list->loaded == 0) {
		list->handle = NULL;
		return TSS_SUCCESS;
	}

	list->handle = malloc(list->loaded * sizeof (UINT32));
        if (list->handle == NULL) {
		LOGERROR("malloc of %zd bytes failed.", list->loaded * sizeof (UINT32));
		list->loaded = 0;
                return TCSERR(TSS_E_OUTOFMEMORY);
        }

	for (i = 0; i < list->loaded; i++)
		UnloadBlob_UINT32(offset, &list->handle[i], blob);

	return TSS_SUCCESS;
}

void
LoadBlob_DIGEST(UINT64 *offset, BYTE *blob, TPM_DIGEST *digest)
{
	LoadBlob(offset, TPM_SHA1_160_HASH_LEN, blob, digest->digest);
}

void
UnloadBlob_DIGEST(UINT64 *offset, BYTE *blob, TPM_DIGEST *digest)
{
	UnloadBlob(offset, TPM_SHA1_160_HASH_LEN, blob, digest->digest);
}

void
LoadBlob_NONCE(UINT64 *offset, BYTE *blob, TPM_NONCE *nonce)
{
	LoadBlob(offset, TCPA_NONCE_SIZE, blob, nonce->nonce);
}

void
UnloadBlob_NONCE(UINT64 *offset, BYTE *blob, TPM_NONCE *nonce)
{
	UnloadBlob(offset, TCPA_NONCE_SIZE, blob, nonce->nonce);
}

void
LoadBlob_AUTHDATA(UINT64 *offset, BYTE *blob, TPM_AUTHDATA *authdata)
{
	LoadBlob(offset, TPM_SHA1_160_HASH_LEN, blob, authdata->authdata);
}

void
UnloadBlob_AUTHDATA(UINT64 *offset, BYTE *blob, TPM_AUTHDATA *authdata)
{
	UnloadBlob(offset, TPM_SHA1_160_HASH_LEN, blob, authdata->authdata);
}

TSS_RESULT
UnloadBlob_PCR_INFO_SHORT(UINT64 *offset, BYTE *blob, TPM_PCR_INFO_SHORT *pcrInfoOut)
{
	TSS_RESULT result;
	BYTE locAtRelease;
	TPM_DIGEST digest;

	LOGDEBUG("UnloadBlob_PCR_INFO_SHORT.");
	/* Only adjust the offset until the end of this data type */
	if (!pcrInfoOut) {
		if ((result = UnloadBlob_PCR_SELECTION(offset, blob, NULL)))
			return result;
		/* What should go to &pcrInfoOut->localityAtRelease */
		UnloadBlob_BYTE(offset, NULL, blob);
		/* What should go to &pcrInfoOut->digestAtRelease */
		UnloadBlob_DIGEST(offset, blob, NULL);
		return TSS_SUCCESS;
	}

	/* Normal retrieve or TPM_PCR_INFO_SHORT (not used yet, kept for
	 * integrity purposes.
	 * TPM_PCR_SELECTION pcrSelection
	 * TPM_LOCALITY_SELECTION localityAtRelease
	 * TPM_COMPOSITE_HASH digestAtRelease
	 *  */
	if ((result = UnloadBlob_PCR_SELECTION(offset, blob, &pcrInfoOut->pcrSelection)))
		return result;

	UnloadBlob_BYTE(offset, &locAtRelease, blob);
	pcrInfoOut->localityAtRelease = locAtRelease;
	UnloadBlob_DIGEST(offset, blob, &digest);
	pcrInfoOut->digestAtRelease = digest;

	return TSS_SUCCESS;
}

TSS_RESULT
UnloadBlob_PCR_SELECTION(UINT64 *offset, BYTE *blob, TCPA_PCR_SELECTION *pcr)
{
	if (!pcr) {
		UINT16 size;

		UnloadBlob_UINT16(offset, &size, blob);

		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);

		return TSS_SUCCESS;
	}

	UnloadBlob_UINT16(offset, &pcr->sizeOfSelect, blob);
	pcr->pcrSelect = malloc(pcr->sizeOfSelect);
        if (pcr->pcrSelect == NULL) {
		LOGERROR("malloc of %hu bytes failed.", pcr->sizeOfSelect);
		pcr->sizeOfSelect = 0;
                return TCSERR(TSS_E_OUTOFMEMORY);
        }
	UnloadBlob(offset, pcr->sizeOfSelect, blob, pcr->pcrSelect);

	return TSS_SUCCESS;
}

void
LoadBlob_PCR_SELECTION(UINT64 *offset, BYTE * blob, TCPA_PCR_SELECTION pcr)
{
	LoadBlob_UINT16(offset, pcr.sizeOfSelect, blob);
	LoadBlob(offset, pcr.sizeOfSelect, blob, pcr.pcrSelect);
}

TSS_RESULT
UnloadBlob_PCR_COMPOSITE(UINT64 *offset, BYTE *blob, TCPA_PCR_COMPOSITE *out)
{
	TSS_RESULT rc;

	if (!out) {
		UINT32 size;

		if ((rc = UnloadBlob_PCR_SELECTION(offset, blob, NULL)))
			return rc;

		UnloadBlob_UINT32(offset, &size, blob);
		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);

		return TSS_SUCCESS;
	}

	if ((rc = UnloadBlob_PCR_SELECTION(offset, blob, &out->select)))
		return rc;

	UnloadBlob_UINT32(offset, &out->valueSize, blob);
	out->pcrValue = malloc(out->valueSize);
        if (out->pcrValue == NULL) {
		LOGERROR("malloc of %u bytes failed.", out->valueSize);
		out->valueSize = 0;
                return TCSERR(TSS_E_OUTOFMEMORY);
        }
	UnloadBlob(offset, out->valueSize, blob, (BYTE *) out->pcrValue);

	return TSS_SUCCESS;
}

void LoadBlob_TPM_PCR_INFO_LONG(UINT64 * offset, BYTE * blob, TPM_LOCALITY_SELECTION locAtC, TPM_LOCALITY_SELECTION locAtR, UINT32 maskPcrAtC, TPM_PCRVALUE * pcrAtC, UINT32 maskPcrAtR, TPM_PCRVALUE * pcrAtR) {
   UINT32               i;
   UINT32               cptC = 0;
   UINT32               cptR = 0;
   UINT16               sizeOfSelectAtC = 0;
   UINT16               sizeOfSelectAtR = 0;
   struct USHAContext   ctx_sha1;
   BYTE                 data[10];
   TPM_PCRVALUE         tpmDigest;
   UINT64               index;
   
   for(i = 0 ; i < 32 ; i++) {
      cptC += (((maskPcrAtC >> i) & 1) == 1);
   }
   if(maskPcrAtC & 0xff) sizeOfSelectAtC = 1;
   if(maskPcrAtC & 0xff00) sizeOfSelectAtC = 2;
   if(maskPcrAtC & 0xff0000) sizeOfSelectAtC = 3;
   if(maskPcrAtC & 0xff000000) sizeOfSelectAtC = 4;

   for(i = 0 ; i < 32 ; i++) {
      cptR += (((maskPcrAtR >> i) & 1) == 1);
   }
   if(maskPcrAtR & 0xff) sizeOfSelectAtR = 1;
   if(maskPcrAtR & 0xff00) sizeOfSelectAtR = 2;
   if(maskPcrAtR & 0xff0000) sizeOfSelectAtR = 3;
   if(maskPcrAtR & 0xff000000) sizeOfSelectAtR = 4;
   
   LoadBlob_UINT16(offset, TPM_TAG_PCR_INFO_LONG, blob);
   LoadBlob_BYTE(offset, locAtC, blob);
   LoadBlob_BYTE(offset, locAtR, blob);
   
   LoadBlob_UINT16(offset, sizeOfSelectAtC, blob);
   LoadBlob(offset, sizeOfSelectAtC, blob, (BYTE *) &maskPcrAtC);
   LoadBlob_UINT16(offset, sizeOfSelectAtR, blob);
   LoadBlob(offset, sizeOfSelectAtR, blob, (BYTE *) &maskPcrAtR);

   // digest at creation
   index = 0;
   LoadBlob_UINT16(&index, sizeOfSelectAtC, data);
   LoadBlob(&index, sizeOfSelectAtC, data, (BYTE *) &maskPcrAtC);
   LoadBlob_UINT32(&index, cptC * TPM_SHA1_160_HASH_LEN, data);
   USHAReset(&ctx_sha1, SHA1);
   USHAInput(&ctx_sha1, data, index);
   for(i = 0 ; i < cptC ; i++) {
      USHAInput(&ctx_sha1, pcrAtC[i].digest, TPM_SHA1_160_HASH_LEN);
   }
   USHAResult(&ctx_sha1, tpmDigest.digest);
   
   LoadBlob(offset, sizeof(tpmDigest.digest), blob, tpmDigest.digest);
   
   // digest at release
   index = 0;
   LoadBlob_UINT16(&index, sizeOfSelectAtR, data);
   LoadBlob(&index, sizeOfSelectAtR, data, (BYTE *) &maskPcrAtR);
   LoadBlob_UINT32(&index, cptR * TPM_SHA1_160_HASH_LEN, data);
   USHAReset(&ctx_sha1, SHA1);
   USHAInput(&ctx_sha1, data, index);
   for(i = 0 ; i < cptR ; i++) {
      USHAInput(&ctx_sha1, pcrAtR[i].digest, TPM_SHA1_160_HASH_LEN);
   }
   USHAResult(&ctx_sha1, tpmDigest.digest);
   
   LoadBlob(offset, sizeof(tpmDigest.digest), blob, tpmDigest.digest);
}

