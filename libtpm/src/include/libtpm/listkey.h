#pragma once

#define NAMEKEY 256

typedef struct {
	TPM_KEY_HANDLE handle;
	char 				name[NAMEKEY];
	UINT32         locality;
} LOADED_KEY;


