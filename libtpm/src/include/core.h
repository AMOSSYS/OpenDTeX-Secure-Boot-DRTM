#pragma once 

#include <libtpm/libtpm.h>

#define printf LIBTPM_GetPrintf()
#define malloc LIBTPM_GetMalloc()
#define free   LIBTPM_GetFree()
#define tpm_io LIBTPM_GetTPMIO()

bool        LIBTPM_IsInit(void);
pf_tpm_io   LIBTPM_GetTPMIO(void);
pf_malloc   LIBTPM_GetMalloc(void);
pf_free     LIBTPM_GetFree(void);
pf_printf   LIBTPM_GetPrintf(void);
void        LIBTPM_Log(LIBTPM_VERBOSITY v, char * fmt, ...);


#define LOGERROR(fmt, ...) LIBTPM_Log(LIBTPM_ERROR, "[%s:%u][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGDEBUG(fmt, ...) LIBTPM_Log(LIBTPM_DEBUG, "[%s:%u][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)

#define TSS_TPM_TXBLOB_HDR_LEN		(sizeof(UINT16) + (2 * sizeof(UINT32)))
#define TSS_TPM_RSP_BLOB_AUTH_LEN	(sizeof(TPM_NONCE) + sizeof(TPM_DIGEST) + sizeof(TPM_BOOL))
#define TSS_TPM_TXBLOB_SIZE 			4096

