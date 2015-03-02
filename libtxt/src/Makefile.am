AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99
AM_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -Iinclude -D__UC_MAINLIBC__
AM_CFLAGS += -D__LIBTXT_COMPILING__

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99 -D__ASSEMBLY__ -DASM_FILE
AM_CCASFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CCASFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -Iinclude -D__UC_MAINLIBC__
AM_CCASFLAGS += -D__LIBTXT_COMPILING__

SOURCE = acmod.c txt.c heap.c

lib_LIBRARIES = libtxt.a libtxt-pic.a

libtxt_a_SOURCES = $(SOURCE)
libtxt_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS)
libtxt_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS)

libtxt_pic_a_SOURCES = $(SOURCE)
libtxt_pic_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS) -fPIC
libtxt_pic_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS) -fPIC

libtxtincludedir = $(includedir)/txt
libtxtinclude_HEADERS = 	include/txt/txt.h include/txt/smx.h include/txt/txtregs.h include/txt/acmod.h include/txt/errorcode.h include/txt/heap.h include/txt/misc.h include/txt/mle.h

noinst_HEADERS = include/stuff.h
									
install-data-hook:
	test -L $(includedir)/txt/txt && rm $(includedir)/txt/txt && ln -s $(includedir)/txt/. $(includedir)/txt/txt || ln -s $(includedir)/txt/. $(includedir)/txt/txt
	
uninstall-local:
	test $(includedir)/txt && rm -rf $(includedir)/txt || true

