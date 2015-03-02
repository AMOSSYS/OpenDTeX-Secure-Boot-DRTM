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
 * \file store_key.c
 * \brief Functions to handle key storage.
 * \author Goulven Guiheux
 *
 * 
 */

#include <uc/uc.h>
#include <uc/listentry.h>
#include <libtpm/libtpm.h>
#include "core.h"

TSS_RESULT STOREKEY_AddKey(UINT32 locality, TPM_KEY_HANDLE handle, char * name);
TSS_RESULT STOREKEY_RemoveKey(TPM_KEY_HANDLE handle);

typedef struct {
	LIST_ENTRY 	entry;
	LOADED_KEY	key;
} BLOB_KEY;

static LIST_ENTRY keys;

TSS_RESULT LIBTPMCALL TPM_InitStoredKey(void) {
   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
      
	InitializeListHead(&keys);
	
	LOGDEBUG("Key storage intialized");
	
	return TSS_SUCCESS;
}

TSS_RESULT LIBTPMCALL TPM_ClearStoredKey(void) {
	PLIST_ENTRY entry;

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }

   LOGDEBUG("Clearing the key storage ...");

	entry = keys.Flink;

	while(entry != &keys) {
		BLOB_KEY * 	keyblob;
		TSS_RESULT 	result;
	
		keyblob = CONTAINING_RECORD(entry, BLOB_KEY, entry);
		
		LOGDEBUG("Flush %s", keyblob->key.name);
		
		result = TCS_FlushSpecific(keyblob->key.locality, keyblob->key.handle, TPM_RT_KEY);
		if(result != TSS_SUCCESS) {
			LOGERROR("TCS_FlushSpecific returns %x\n", result);
			entry = entry->Flink;
		} else {
			entry = entry->Flink;
			RemoveEntryList(entry->Blink);
			free(keyblob);
		}
	}
	
	return TSS_SUCCESS;
}

TSS_RESULT LIBTPMCALL TPM_ListStoredKey(void) {
	PLIST_ENTRY entry;
	
	if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
	printf("Loaded keys :\n");
	
	for(entry = keys.Flink ; entry != &keys ; entry = entry->Flink) {
		BLOB_KEY * keyblob = CONTAINING_RECORD(entry, BLOB_KEY, entry);
		
		printf("\t%08x (%u) : %s\n", keyblob->key.handle, keyblob->key.locality, keyblob->key.name);
	}
	
	return TSS_SUCCESS;
}

TSS_RESULT LIBTPMCALL TPM_GetKeyHandle(char * name, TPM_KEY_HANDLE * handle) {
	PLIST_ENTRY entry;

   if(LIBTPM_IsInit() == false) {
      LOGERROR("libtpm not initialized");
      return TCSERR(TSS_E_INTERNAL_ERROR);
   }
   
   if(name == NULL) {
		LOGERROR("name = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
   
   if(handle == NULL) {
		LOGERROR("handle = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}   
   
	if(strncmp(name, "SRK", strlen("SRK")) == 0) {
	   *handle = TPM_KH_SRK;
	   LOGDEBUG("Get SRK handle (%x)", *handle);
		return TSS_SUCCESS;
	}
	
	for(entry = keys.Flink ; entry != &keys ; entry = entry->Flink) {
		BLOB_KEY * keyblob = CONTAINING_RECORD(entry, BLOB_KEY, entry);
		 
		if(strncmp(keyblob->key.name, name, sizeof(keyblob->key.name)) == 0) {		   
		   *handle = keyblob->key.handle;
		   LOGDEBUG("Get %s handle (%x)", keyblob->key.name, *handle);
			return TSS_SUCCESS;
		}
	}

   *handle = 0;

	return TCSERR(TCS_E_KEY_MISMATCH);
}

TSS_RESULT LIBTPMCALL TPM_GetKeyName(TPM_KEY_HANDLE handle, char * name, UINT32 size_name) {
	PLIST_ENTRY entry;

   if(name == NULL) {
		LOGERROR("name = null pointer");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}
   
   if(size_name == 0) {
		LOGERROR("size_name = 0");
		return TCSERR(TCS_E_BAD_PARAMETER);
	}   

	if(handle == TPM_KH_SRK) {
	   strncpy(name, "SRK", size_name);
	   LOGDEBUG("Get key name (%x) --> SRK", handle);
		return TSS_SUCCESS;
	}
	
	for(entry = keys.Flink ; entry != &keys ; entry = entry->Flink) {
		BLOB_KEY * keyblob = CONTAINING_RECORD(entry, BLOB_KEY, entry);
		
		if(keyblob->key.handle == handle) {
		   strncpy(name, keyblob->key.name, size_name);
		   LOGDEBUG("Get key name (%x) --> %s", handle, name);
			return TSS_SUCCESS;
		}
	}
	
	strncpy(name, "", size_name);

	return TCSERR(TCS_E_KEY_MISMATCH);
}

TSS_RESULT STOREKEY_AddKey(UINT32 locality, TPM_KEY_HANDLE handle, char * name) {
	BLOB_KEY * toInsert;
	
	toInsert = (BLOB_KEY *) malloc(sizeof(BLOB_KEY));
	if(toInsert == NULL) {
	   LOGERROR("Not enought memory : can't create a BLOB_KEY in the key storage");
	   return TCSERR(TCS_E_OUTOFMEMORY);
	}
	
	toInsert->key.handle = handle;
	strncpy(toInsert->key.name, name, sizeof(toInsert->key.name));
	toInsert->key.locality = locality;
	
	InsertHeadList(&keys, &toInsert->entry);
	
	LOGDEBUG("Key %s (%x) stored", name, handle);
	
	return TSS_SUCCESS;
}

TSS_RESULT STOREKEY_RemoveKey(TPM_KEY_HANDLE handle) {
	PLIST_ENTRY entry;
	
	for(entry = keys.Flink ; entry != &keys ; entry = entry->Flink) {
		BLOB_KEY * keyblob = CONTAINING_RECORD(entry, BLOB_KEY, entry);
		
		if(keyblob->key.handle == handle) {
		   LOGDEBUG("Key %s (%x, %u) flushed", keyblob->key.name, handle, keyblob->key.locality);
			RemoveEntryList(entry);
			free(keyblob);
			return TSS_SUCCESS;
		}
	}
	
	LOGERROR("Key %x not found ...", handle);
	
	return TCSERR(TCS_E_KEY_MISMATCH);
}

