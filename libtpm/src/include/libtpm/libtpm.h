#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#if ARCHI == 32
#define LIBTPMCALL __attribute__((stdcall, regparm(2)))
#define __type_mem unsigned int
#elif ARCHI == 64
#define LIBTPMCALL __attribute__((regparm(4)))
#define __type_mem unsigned long long
#else
#error ARCHI macro not defined
#endif

#define __TPM_CHECK__

#include <libtpm/types.h>
#include <libtpm/raw.h>

typedef int          (LIBTPMCALL * pf_printf)(const char * format, ...);
typedef void *       (LIBTPMCALL * pf_malloc)(size_t size);
typedef void         (LIBTPMCALL * pf_free)(void *ptr);
typedef unsigned int (LIBTPMCALL * pf_tpm_io)(unsigned int locality, unsigned char * txBlobIn, unsigned int sizeIn, unsigned char * txBlobOut, unsigned int sizeOut);

typedef enum {
   LIBTPM_NOLOG = 0,
   LIBTPM_ERROR,
   LIBTPM_DEBUG,
   LIBTPM_LOGMAX
} LIBTPM_VERBOSITY;

// Libtpm intialization functions
void           LIBTPMCALL  LIBTPM_Init(pf_tpm_io v_tpm_io, pf_malloc v_malloc, pf_free v_free, pf_printf v_printf);
void           LIBTPMCALL  LIBTPM_SetLogVerbosity(LIBTPM_VERBOSITY v);

// Low level TPM functions
TSS_RESULT		LIBTPMCALL	TCS_PcrRead(UINT32 locality, TPM_PCRINDEX pcrNum, TPM_PCRVALUE * outDigest);
TSS_RESULT		LIBTPMCALL	TCS_Extend(UINT32 locality, TPM_PCRINDEX pcrNum, TPM_PCRVALUE * inDigest, TPM_PCRVALUE * outDigest);
TSS_RESULT		LIBTPMCALL	TCS_OIAP(UINT32 locality, TPM_AUTH * auth);
TSS_RESULT		LIBTPMCALL	TCS_OSAP(UINT32 locality, TPM_AUTH * authOSAP, TPM_AUTH * auth, TPM_ENTITY_TYPE entityType, UINT32 entityValue);
TSS_RESULT     LIBTPMCALL  TCS_CreateWrapKey(UINT32 locality, TPM_KEY_HANDLE hParent, TPM_AUTHDATA * authData, TPM_ENCAUTH * keyUsageAuth, TPM_ENCAUTH * keyMigrationAuth, UINT32 keyInfoSize, BYTE * keyInfo, TPM_AUTH * auth, UINT32 * keyDataSize, BYTE ** keyData);
TSS_RESULT		LIBTPMCALL	TCS_LoadKey2(UINT32 locality, TPM_KEY_HANDLE hParent, TPM_AUTHDATA * authData, BYTE * keyBlob, UINT32 keyBlobLen, TPM_AUTH * auth, TPM_KEY_HANDLE * hKey);
TSS_RESULT     LIBTPMCALL  TCS_Seal(UINT32 locality, TPM_KEY_HANDLE hParent, TPM_AUTHDATA * parentAuth, TPM_ENCAUTH * encDataAuth, UINT32 pcrInfoSize, BYTE * pcrInfo, UINT32 dataSize, BYTE * data, TPM_AUTH * auth, UINT32 * sealedDataSize, BYTE ** sealedData);
TSS_RESULT		LIBTPMCALL	TCS_Unseal(UINT32 locality, TPM_KEY_HANDLE hParent, TPM_AUTHDATA * parentSecret, BYTE * sealedData, UINT32 sealedDataSize, TPM_AUTHDATA * dataSecret, TPM_AUTH * parentAuth, TPM_AUTH * dataAuth, BYTE ** data, UINT32 * dataSize);
TSS_RESULT		LIBTPMCALL	TCS_FlushSpecific(UINT32 locality, TPM_KEY_HANDLE handle, TPM_RESOURCE_TYPE type);
TSS_RESULT     LIBTPMCALL  TCS_GetRandom(UINT32 locality, BYTE * buffer, UINT32 size);
TSS_RESULT     LIBTPMCALL  TCS_StirRandom(UINT32 locality, BYTE * buffer, UINT32 size);
TSS_RESULT     LIBTPMCALL  TCS_Sign(UINT32 locality, TPM_KEY_HANDLE hParent, TPM_AUTHDATA * parentAuth, UINT32 dataSize, BYTE * data, TPM_AUTH * auth, UINT32 * sigBlobSize, BYTE ** sigBlobData);

// High and friendly level TPM functions
TSS_RESULT 		LIBTPMCALL	TPM_InitStoredKey(void);
TSS_RESULT 		LIBTPMCALL	TPM_ClearStoredKey(void);
TSS_RESULT 		LIBTPMCALL	TPM_ListStoredKey(void);
TSS_RESULT  	LIBTPMCALL	TPM_GetKeyHandle(char * name, TPM_KEY_HANDLE * handle);
TSS_RESULT		LIBTPMCALL	TPM_GetKeyName(TPM_KEY_HANDLE handle, char * name, UINT32 size_name);

TSS_RESULT     LIBTPMCALL  TPM_CreateKey(UINT32 locality, BYTE ** keyBlob, UINT32 * keyBlobSize, char * parentName, char * pwdParent, UINT32 size_pwdParent, char * pwdKey, UINT32 size_pwdKey, char * pwdMigrationKey, UINT32 size_pwdMigrationKey, BYTE * keyInfo, UINT32 keyInfoSize);
TSS_RESULT 		LIBTPMCALL	TPM_LoadKey(UINT32 locality, BYTE * keyBlob, UINT32 keyBlobLen, char * keyName, char * parentName, char * password, UINT32 sizepwd);
TSS_RESULT 		LIBTPMCALL	TPM_UnloadKey(UINT32 locality, char * keyName);
TSS_RESULT     LIBTPMCALL  TPM_Seal(UINT32 locality, BYTE ** sealedDataBlob, UINT32 * sealedDataBlobSize, BYTE * dataBlob, UINT32 dataBlobSize, char * parentName, char * parentPwd, UINT32 size_parentPwd, char * dataPwd, UINT32 size_dataPwd, BYTE * pcrInfo, UINT32 pcrInfoSize);
TSS_RESULT 		LIBTPMCALL	TPM_Unseal(UINT32 locality, BYTE ** dataBlob, UINT32 * dataBlobSize, BYTE * sealedBlob, UINT32 sealedBlobSize, char * keyName, char * pwdKey, UINT32 size_pwdKey, char * pwdData, UINT32 size_pwdData);
TSS_RESULT     LIBTPMCALL  TPM_Sign(UINT32 locality, BYTE ** sigBlobData, UINT32 * sigBlobSize, char * parentName, char * pwdParent, UINT32 size_pwdParent, UINT32 dataSize, BYTE * data);

TSS_RESULT     LIBTPMCALL  LIBTPM_GetRand(UINT32 locality, BYTE * buf, UINT32 len);

#undef __TPM_CHECK__

