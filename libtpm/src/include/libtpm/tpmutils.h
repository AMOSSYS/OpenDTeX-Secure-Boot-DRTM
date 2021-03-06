#pragma once

TSS_RESULT  tcs_compute_auth(UINT32 locality, TPM_AUTH * auth, TPM_NONCE * h1, TPM_AUTHDATA * secret);
TSS_RESULT	tcs_check_auth(TPM_AUTH * auth, TPM_NONCE * h1, TPM_AUTHDATA * secret);
TSS_RESULT	tpm_rsp_parse(TPM_COMMAND_CODE ordinal, BYTE *b, UINT32 len, ...);
TSS_RESULT	tpm_rqu_build(TPM_COMMAND_CODE ordinal, UINT64 *outOffset, BYTE *out_blob, ...);

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

void UINT64ToArray(UINT64, BYTE *);
void UINT32ToArray(UINT32, BYTE *);
void UINT16ToArray(UINT16, BYTE *);
UINT64 Decode_UINT64(BYTE *);
UINT32 Decode_UINT32(BYTE *);
UINT16 Decode_UINT16(BYTE *);
void LoadBlob_UINT64(UINT64 *, UINT64, BYTE *);
void LoadBlob_UINT32(UINT64 *, UINT32, BYTE *);
void LoadBlob_UINT16(UINT64 *, UINT16, BYTE *);
void UnloadBlob_UINT64(UINT64 *, UINT64 *, BYTE *);
void UnloadBlob_UINT32(UINT64 *, UINT32 *, BYTE *);
void UnloadBlob_UINT16(UINT64 *, UINT16 *, BYTE *);
void LoadBlob_BYTE(UINT64 *, BYTE, BYTE *);
void UnloadBlob_BYTE(UINT64 *, BYTE *, BYTE *);
void LoadBlob_BOOL(UINT64 *, TSS_BOOL, BYTE *);
void UnloadBlob_BOOL(UINT64 *, TSS_BOOL *, BYTE *);
void LoadBlob(UINT64 *, UINT32, BYTE *, BYTE *);
void UnloadBlob(UINT64 *, UINT32, BYTE *, BYTE *);
void LoadBlob_Header(UINT16, UINT32, UINT32, BYTE *);
TSS_RESULT UnloadBlob_Header(BYTE *, UINT32 *);
TSS_RESULT UnloadBlob_MIGRATIONKEYAUTH(UINT64 *, BYTE *, TCPA_MIGRATIONKEYAUTH *);
void LoadBlob_Auth(UINT64 *, BYTE *, TPM_AUTH *);
void UnloadBlob_Auth(UINT64 *, BYTE *, TPM_AUTH *);
void LoadBlob_TPM_RSA_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_RSA_KEY_PARMS *rsaKeyParms);
void LoadBlob_TPM_SYMMETRIC_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_SYMMETRIC_KEY_PARMS *symKeyParms);
void LoadBlob_KEY_PARMS(UINT64 *, BYTE *, TCPA_KEY_PARMS *);
TSS_RESULT UnloadBlob_TPM_RSA_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_RSA_KEY_PARMS *out, UINT32 sizeout);
TSS_RESULT UnloadBlob_TPM_SYMMETRIC_KEY_PARMS(UINT64 *offset, BYTE *blob, TPM_SYMMETRIC_KEY_PARMS *out, UINT32 sizeout);
TSS_RESULT UnloadBlob_KEY_PARMS(UINT64 *, BYTE *, TCPA_KEY_PARMS *);
TSS_RESULT UnloadBlob_STORE_PUBKEY(UINT64 *, BYTE *, TCPA_STORE_PUBKEY *);
void LoadBlob_STORE_PUBKEY(UINT64 *, BYTE *, TCPA_STORE_PUBKEY *);
void UnloadBlob_VERSION(UINT64 *, BYTE *, TPM_VERSION *);
void LoadBlob_VERSION(UINT64 *, BYTE *, TPM_VERSION *);
void UnloadBlob_TCPA_VERSION(UINT64 *, BYTE *, TCPA_VERSION *);
void LoadBlob_TCPA_VERSION(UINT64 *, BYTE *, TCPA_VERSION *);
TSS_RESULT UnloadBlob_TSS_KEY(UINT64 *, BYTE *, TSS_KEY *);
void LoadBlob_TSS_KEY(UINT64 *, BYTE *, TSS_KEY *);
void LoadBlob_PUBKEY(UINT64 *, BYTE *, TCPA_PUBKEY *);
TSS_RESULT UnloadBlob_PUBKEY(UINT64 *, BYTE *, TCPA_PUBKEY *);
void LoadBlob_SYMMETRIC_KEY(UINT64 *, BYTE *, TCPA_SYMMETRIC_KEY *);
TSS_RESULT UnloadBlob_SYMMETRIC_KEY(UINT64 *, BYTE *, TCPA_SYMMETRIC_KEY *);
TSS_RESULT UnloadBlob_PCR_SELECTION(UINT64 *, BYTE *, TCPA_PCR_SELECTION *);
void LoadBlob_PCR_SELECTION(UINT64 *, BYTE *, TCPA_PCR_SELECTION);
TSS_RESULT UnloadBlob_PCR_COMPOSITE(UINT64 *, BYTE *, TCPA_PCR_COMPOSITE *);
void LoadBlob_PCR_INFO(UINT64 *, BYTE *, TCPA_PCR_INFO *);
TSS_RESULT UnloadBlob_PCR_INFO(UINT64 *, BYTE *, TCPA_PCR_INFO *);
TSS_RESULT UnloadBlob_STORED_DATA(UINT64 *, BYTE *, TCPA_STORED_DATA *);
void LoadBlob_STORED_DATA(UINT64 *, BYTE *, TCPA_STORED_DATA *);
void LoadBlob_KEY_FLAGS(UINT64 *, BYTE *, TCPA_KEY_FLAGS *);
void UnloadBlob_KEY_FLAGS(UINT64 *, BYTE *, TCPA_KEY_FLAGS *);
TSS_RESULT UnloadBlob_CERTIFY_INFO(UINT64 *, BYTE *, TCPA_CERTIFY_INFO *);
TSS_RESULT UnloadBlob_KEY_HANDLE_LIST(UINT64 *, BYTE *, TCPA_KEY_HANDLE_LIST *);
void LoadBlob_UUID(UINT64 *, BYTE *, TSS_UUID);
void UnloadBlob_UUID(UINT64 *, BYTE *, TSS_UUID *);
void LoadBlob_COUNTER_VALUE(UINT64 *, BYTE *, TPM_COUNTER_VALUE *);
void UnloadBlob_COUNTER_VALUE(UINT64 *, BYTE *, TPM_COUNTER_VALUE *);
void LoadBlob_DIGEST(UINT64 *, BYTE *, TPM_DIGEST *);
void UnloadBlob_DIGEST(UINT64 *, BYTE *, TPM_DIGEST *);
void LoadBlob_NONCE(UINT64 *, BYTE *, TPM_NONCE *);
void UnloadBlob_NONCE(UINT64 *, BYTE *, TPM_NONCE *);
void LoadBlob_AUTHDATA(UINT64 *, BYTE *, TPM_AUTHDATA *);
void UnloadBlob_AUTHDATA(UINT64 *, BYTE *, TPM_AUTHDATA *);
#define LoadBlob_ENCAUTH(a, b, c)	LoadBlob_AUTHDATA(a, b, c)
#define UnloadBlob_ENCAUTH(a, b, c)	UnloadBlob_AUTHDATA(a, b, c)

void UnloadBlob_CURRENT_TICKS(UINT64 *, BYTE *, TPM_CURRENT_TICKS *);
TSS_RESULT UnloadBlob_PCR_INFO_SHORT(UINT64 *, BYTE *, TPM_PCR_INFO_SHORT *);

void LoadBlob_TPM_PCR_INFO_LONG(UINT64 * offset, BYTE * blob, TPM_LOCALITY_SELECTION locAtC, TPM_LOCALITY_SELECTION locAtR, UINT32 maskPcrAtC, TPM_PCRVALUE * pcrAtC, UINT32 maskPcrAtR, TPM_PCRVALUE * pcrAtR);

