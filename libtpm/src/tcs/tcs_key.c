// -*- coding: utf-8 -*-

/*
 * Licensed Materials - Property of IBM
 *
 * trousers - An open source TCG Software Stack
 *
 * (C) Copyright International Business Machines Corp. 2004-2006
 *
 */

#include <libtpm/libtpm.h>
#include <libtpm/tpmutils.h>
#include "core.h"
#include "crypto/sha.h"

void
LoadBlob_TPM_RSA_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_RSA_KEY_PARMS *rsaKeyParms)
{
   if(rsaKeyParms) {
      LoadBlob_UINT32(offset, rsaKeyParms->keyLength, blob);
      LoadBlob_UINT32(offset, rsaKeyParms->numPrimes, blob);
      LoadBlob_UINT32(offset, rsaKeyParms->exponentSize, blob);
      LoadBlob(offset, rsaKeyParms->exponentSize, blob, rsaKeyParms->exponent);
   }
}

void
LoadBlob_TPM_SYMMETRIC_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_SYMMETRIC_KEY_PARMS *symKeyParms)
{
   if(symKeyParms) {
      LoadBlob_UINT32(offset, symKeyParms->keyLength, blob);
      LoadBlob_UINT32(offset, symKeyParms->blockSize, blob);
      LoadBlob_UINT32(offset, symKeyParms->ivSize, blob);
      LoadBlob(offset, symKeyParms->ivSize, blob, symKeyParms->IV);
   }
}

void
LoadBlob_KEY_PARMS(UINT64 *offset, BYTE *blob, TCPA_KEY_PARMS *keyParms)
{
   UINT64 save;
   
	LoadBlob_UINT32(offset, keyParms->algorithmID, blob);
	LoadBlob_UINT16(offset, keyParms->encScheme, blob);
	LoadBlob_UINT16(offset, keyParms->sigScheme, blob);
	save = *offset;
	LoadBlob_UINT32(offset, keyParms->parmSize, blob);
	
	if(keyParms->algorithmID == TPM_ALG_RSA) {
	   LoadBlob_TPM_RSA_KEY_PARMS(offset, blob, (TPM_RSA_KEY_PARMS *) keyParms->parms);
	   keyParms->parmSize = *offset - save - 4;
	   LoadBlob_UINT32(&save, keyParms->parmSize, blob);
	} else if(keyParms->algorithmID == TPM_ALG_AES) {
	   LoadBlob_TPM_SYMMETRIC_KEY_PARMS(offset, blob, (TPM_SYMMETRIC_KEY_PARMS *) keyParms->parms);
	   keyParms->parmSize = *offset - save - 4;
	   LoadBlob_UINT32(&save, keyParms->parmSize, blob);
	}
}

TSS_RESULT
UnloadBlob_STORE_PUBKEY(UINT64 *offset, BYTE *blob, TCPA_STORE_PUBKEY *store)
{
	if (!store) {
		UINT32 keyLength;

		UnloadBlob_UINT32(offset, &keyLength, blob);

		if (keyLength > 0)
			UnloadBlob(offset, keyLength, blob, NULL);

		return TSS_SUCCESS;
	}

	UnloadBlob_UINT32(offset, &store->keyLength, blob);

	if(store->keyLength == 0) {
		store->key = NULL;
		LOGDEBUG("Unloading a public key of size 0!");
	} else {
		store->key = (BYTE *)malloc(store->keyLength);
		if (store->key == NULL) {
			LOGERROR("malloc of %u bytes failed.", store->keyLength);
			store->keyLength = 0;
			return TCSERR(TSS_E_OUTOFMEMORY);
		}

		UnloadBlob(offset, store->keyLength, blob, store->key);
	}

	return TSS_SUCCESS;
}

void
LoadBlob_STORE_PUBKEY(UINT64 *offset, BYTE * blob, TCPA_STORE_PUBKEY * store)
{
	LoadBlob_UINT32(offset, store->keyLength, blob);
	LoadBlob(offset, store->keyLength, blob, store->key);
}

TSS_RESULT
UnloadBlob_TSS_KEY(UINT64 *offset, BYTE *blob, TSS_KEY *key)
{
	TSS_RESULT rc;

	if (!key) {
		UINT32 size;

		/* TPM_KEY's ver and TPM_KEY12's tag/file are
		   the same size, so... */
		UnloadBlob_VERSION(offset, blob, NULL);
		UnloadBlob_UINT16(offset, NULL, blob);
		UnloadBlob_KEY_FLAGS(offset, blob, NULL);
		UnloadBlob_BOOL(offset, NULL, blob);
		if ((rc = UnloadBlob_KEY_PARMS(offset, blob, NULL)))
			return rc;
		UnloadBlob_UINT32(offset, &size, blob);

		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);

		if ((rc = UnloadBlob_STORE_PUBKEY(offset, blob, NULL)))
			return rc;

		UnloadBlob_UINT32(offset, &size, blob);

		if (size > 0)
			UnloadBlob(offset, size, blob, NULL);

		return TSS_SUCCESS;
	}

	if (key->hdr.key12.tag == TPM_TAG_KEY12) {
		UnloadBlob_UINT16(offset, &key->hdr.key12.tag, blob);
		UnloadBlob_UINT16(offset, &key->hdr.key12.fill, blob);
	} else
		UnloadBlob_TCPA_VERSION(offset, blob, &key->hdr.key11.ver);
	UnloadBlob_UINT16(offset, &key->keyUsage, blob);
	UnloadBlob_KEY_FLAGS(offset, blob, &key->keyFlags);
	UnloadBlob_BOOL(offset, (TSS_BOOL *)&key->authDataUsage, blob);
	if ((rc = UnloadBlob_KEY_PARMS(offset, blob, &key->algorithmParms)))
		return rc;
	UnloadBlob_UINT32(offset, &key->PCRInfoSize, blob);

	if (key->PCRInfoSize == 0)
		key->PCRInfo = NULL;
	else {
		key->PCRInfo = malloc(key->PCRInfoSize);
		if (key->PCRInfo == NULL) {
			LOGERROR("malloc of %u bytes failed.", key->PCRInfoSize);
			key->PCRInfoSize = 0;
			free(key->algorithmParms.parms);
			key->algorithmParms.parms = NULL;
			key->algorithmParms.parmSize = 0;
			return TCSERR(TSS_E_OUTOFMEMORY);
		}
		UnloadBlob(offset, key->PCRInfoSize, blob, key->PCRInfo);
	}

	if ((rc = UnloadBlob_STORE_PUBKEY(offset, blob, &key->pubKey))) {
		free(key->PCRInfo);
		key->PCRInfo = NULL;
		key->PCRInfoSize = 0;
		free(key->algorithmParms.parms);
		key->algorithmParms.parms = NULL;
		key->algorithmParms.parmSize = 0;
		return rc;
	}
	UnloadBlob_UINT32(offset, &key->encSize, blob);

	if (key->encSize == 0)
		key->encData = NULL;
	else {
		key->encData = (BYTE *)malloc(key->encSize);
		if (key->encData == NULL) {
			LOGERROR("malloc of %d bytes failed.", key->encSize);
			key->encSize = 0;
			free(key->algorithmParms.parms);
			key->algorithmParms.parms = NULL;
			key->algorithmParms.parmSize = 0;
			free(key->PCRInfo);
			key->PCRInfo = NULL;
			key->PCRInfoSize = 0;
			free(key->pubKey.key);
			key->pubKey.key = NULL;
			key->pubKey.keyLength = 0;
			return TCSERR(TSS_E_OUTOFMEMORY);
		}
		UnloadBlob(offset, key->encSize, blob, key->encData);
	}

	return TSS_SUCCESS;
}

void
LoadBlob_TSS_KEY(UINT64 *offset, BYTE * blob, TSS_KEY * key)
{
	if (key->hdr.key12.tag == TPM_TAG_KEY12) {
		LoadBlob_UINT16(offset, key->hdr.key12.tag, blob);
		LoadBlob_UINT16(offset, key->hdr.key12.fill, blob);
	} else
		LoadBlob_TCPA_VERSION(offset, blob, &key->hdr.key11.ver);
	LoadBlob_UINT16(offset, key->keyUsage, blob);
	LoadBlob_KEY_FLAGS(offset, blob, &key->keyFlags);
	LoadBlob_BOOL(offset, key->authDataUsage, blob);
	LoadBlob_KEY_PARMS(offset, blob, &key->algorithmParms);
	LoadBlob_UINT32(offset, key->PCRInfoSize, blob);
	LoadBlob(offset, key->PCRInfoSize, blob, key->PCRInfo);
	LoadBlob_STORE_PUBKEY(offset, blob, &key->pubKey);
	LoadBlob_UINT32(offset, key->encSize, blob);
	LoadBlob(offset, key->encSize, blob, key->encData);
}

void
LoadBlob_PUBKEY(UINT64 *offset, BYTE * blob, TCPA_PUBKEY * key)
{
	LoadBlob_KEY_PARMS(offset, blob, &(key->algorithmParms));
	LoadBlob_STORE_PUBKEY(offset, blob, &(key->pubKey));
}

TSS_RESULT
UnloadBlob_PUBKEY(UINT64 *offset, BYTE *blob, TCPA_PUBKEY *key)
{
	TSS_RESULT rc;

	if (!key) {
		if ((rc = UnloadBlob_KEY_PARMS(offset, blob, NULL)))
			return rc;
		return UnloadBlob_STORE_PUBKEY(offset, blob, NULL);
	}

	if ((rc = UnloadBlob_KEY_PARMS(offset, blob, &key->algorithmParms)))
		return rc;
	if ((rc = UnloadBlob_STORE_PUBKEY(offset, blob, &key->pubKey))) {
		free(key->algorithmParms.parms);
		key->algorithmParms.parms = NULL;
		key->algorithmParms.parmSize = 0;
	}

	return rc;
}

void
LoadBlob_KEY_FLAGS(UINT64 *offset, BYTE * blob, TCPA_KEY_FLAGS * flags)
{
	LoadBlob_UINT32(offset, *flags, blob);
}

