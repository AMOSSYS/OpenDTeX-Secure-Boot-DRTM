AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -O0 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99
AM_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -falign-jumps=1 -falign-loops=1 -falign-functions=1 -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -I./include -D__UC_MAINLIBC__

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -O0 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99 -D__ASSEMBLY__ -DASM_FILE
AM_CCASFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm 
AM_CCASFLAGS += -Wall -W -Werror -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -falign-jumps=1 -falign-loops=1 -falign-functions=1 -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -I./include -D__UC_MAINLIBC__

#-Wundef 

AM_LDFLAGS = $(LDFLAGS) $(BUILD_LDFLAGS) -nostdlib
AM_LDADD = 

BUILT_SOURCES =
CLEANFILES =

tloader.lds: tloader.lds.x include/tloader.h
	cpp -P -E -Ui386 $(AM_CCASFLAGS) -o $@ $<

BUILT_SOURCES += tloader.lds
CLEANFILES += tloader.lds

sbin_PROGRAMS = tloader
tloader_SOURCES = 	main.c   \
                     system/boot.S system/kbd.c system/system.c system/memory.c system/log.c \
                     crypto/aes_cbc.c crypto/aes_core.c crypto/sha1.c   \
                     commands/command.c commands/command_loadkey.c commands/command_banner.c \
                     commands/command_sealedkernel32.c commands/command_kernel.c commands/load.S  \
                     commands/command_kernel32.c commands/command_initrd32.c commands/kernel32.S commands/bootos.c \
                     tools/stub.c tools/utils.c tools/com.c          \
                     tools/delay.c tpm/tpm.c                               \
                     tloader.lds.x
                     

tloader_CCASFLAGS = $(AM_CCASFLAGS) $(LIBTXT_CFLAGS) $(LIBTPM_CFLAGS) $(LIBUC_CFLAGS) $(LIBUVIDEO_CFLAGS)
tloader_CFLAGS = $(AM_CFLAGS) $(LIBTXT_CFLAGS) $(LIBTPM_CFLAGS) $(LIBUVIDEO_CFLAGS) $(LIBUC_CFLAGS)
tloader_LDFLAGS = $(AM_LDFLAGS) -T tloader.lds
tloader_LDADD = $(AM_LDADD) $(LIBTPM_LIBS) $(LIBUVIDEO_LIBS) $(LIBUC_LIBS)


