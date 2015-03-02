#ifndef _LIBTCG_H
#define _LIBTCG_H

#define burn(x, len)						\
	memset((char *) x, 0, len);		\
	memset((char *) x, 0xff, len);	\
	memset((char *) x, 0, len);

#define MAXSIZE 100
typedef struct _TCG_KEY TCG_KEY;

struct _TCG_KEY {
	TCG_KEY 	*kParent;
	TCG_KEY 	*kChildren;
	TCG_KEY 	*kBrother;
	TSS_HKEY	hKey;
	TSS_HPOLICY	hPolicy;
	TSS_HPCRS 	hPcrs;
	long int 	depth;
	int		isWellKnown;
	int		flagsPCR;
	char		name[MAXSIZE];
};

#define NBMAX_PCR 24

extern TCG_KEY 		kSRK;
extern TCG_KEY		*currentKey;
extern TSS_HCONTEXT 	hContext;
extern TSS_HTPM 	hTpm;
extern BYTE 		wellKnown[TCPA_SHA1_160_HASH_LEN];
extern TSS_UUID 	SRK_UUID;


void dump(BYTE *p, UINT64 len);
void tspiError(const char *a_szName, TSS_RESULT a_iResult);
long getTaille(FILE *f);
int parsePCR(char *list, int *flagsPCR);
int InsertKey(long int depth, char *name, int isWellKnown, int flagsPCR);
void RecPrintArchitecture(TCG_KEY *kKey);
int CreatePCR(TSS_HPCRS *hPcrs, int flagsPCR);
int LoadKeyFromDisk(TCG_KEY *kKey);

#endif

