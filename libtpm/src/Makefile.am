AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99
AM_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -Iinclude -D__UC_MAINLIBC__
AM_CFLAGS += -D__TPM_COMPILING__ -DTSS_BUILD_NV=1 -DTSS_BUILD_DELEGATION=1

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99 -D__ASSEMBLY__ -DASM_FILE
AM_CCASFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CCASFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -Iinclude -D__UC_MAINLIBC__
AM_CCASFLAGS += -D__TPM_COMPILING__ -DTSS_BUILD_NV=1 -DTSS_BUILD_DELEGATION=1

SOURCE = tcs/tcsi_auth.c tcs/tcsi_pcr.c tcs/tcs_key.c tcs/tcs_random.c tcs/tcsi_key.c tcs/tcsi_seal.c tcs/tcs_pbg.c \
			tcs/tcs_utils.c tcs/tcsi_flush.c tcs/tcsi_random.c tcs/tcsi_sign.c \
			tpm/store_key.c tpm/loadkey.c tpm/unloadkey.c tpm/unseal.c tpm/createkey.c tpm/seal.c \
			tpm/sign.c \
			crypto/hkdf.c crypto/hmac.c crypto/sha1.c crypto/sha224-256.c crypto/sha384-512.c crypto/usha.c \
			core/core.c raw/raw.c

lib_LIBRARIES = libtpm.a libtpm-pic.a

libtpm_a_SOURCES = $(SOURCE)
libtpm_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS)
libtpm_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS)

libtpm_pic_a_SOURCES = $(SOURCE)
libtpm_pic_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS) -fPIE
libtpm_pic_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS) -fPIE

libtpmincludedir = $(includedir)/libtpm
libtpminclude_HEADERS = 	include/libtpm/listkey.h \
									include/libtpm/libtpm.h \
									include/libtpm/types.h \
									include/libtpm/raw.h \
									include/libtpm/tpmutils.h

libtpmtrousersincludedir = $(includedir)/libtpm/trousers
libtpmtrousersinclude_HEADERS = 	include/libtpm/trousers/platform.h \
											include/libtpm/trousers/tss_structs.h \
											include/libtpm/trousers/compat11b.h \
											include/libtpm/trousers/tcs_typedef.h \
											include/libtpm/trousers/tss_defines.h \
											include/libtpm/trousers/tpm.h \
											include/libtpm/trousers/tss_error_basics.h \
											include/libtpm/trousers/tpm_ordinal.h \
											include/libtpm/trousers/tpm_error.h \
											include/libtpm/trousers/tcs_defines.h \
											include/libtpm/trousers/tss_error.h \
											include/libtpm/trousers/trousers_types.h \
											include/libtpm/trousers/tcs_structs.h \
											include/libtpm/trousers/tss_typedef.h \
											include/libtpm/trousers/tcs_error.h

noinst_HEADERS =  					include/core.h \
											include/crypto/sha.h \
											include/crypto/sha-private.h
									
install-data-hook:
	test -L $(includedir)/libtpm/libtpm && rm $(includedir)/libtpm/libtpm && ln -s $(includedir)/libtpm/. $(includedir)/libtpm/libtpm || ln -s $(includedir)/libtpm/. $(includedir)/libtpm/libtpm
	
uninstall-local:
	test $(includedir)/libtpm && rm -rf $(includedir)/libtpm || true
	

