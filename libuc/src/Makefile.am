AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99
AM_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -Iinclude
AM_CFLAGS += -D__UC_COMPILING__

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99 -D__ASSEMBLY__ -DASM_FILE
AM_CCASFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CCASFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -Iinclude
AM_CCASFLAGS += -D__UC_COMPILING__

SOURCE = string.c vsnprintf.c snprintf.c mem.c processor-mtrrs.c

lib_LIBRARIES = libuc.a libuc-pic.a

libuc_a_SOURCES = $(SOURCE)	
libuc_a_CFLAGS = $(AM_CFLAGS)
libuc_a_CCASFLAGS = $(AM_CCASFLAGS)

libuc_pic_a_SOURCES = $(SOURCE)
libuc_pic_a_CFLAGS = $(AM_CFLAGS) -fPIC
libuc_pic_a_CCASFLAGS = $(AM_CCASFLAGS) -fPIC

libucincludedir = $(includedir)/uc
libucinclude_HEADERS = 	include/uc/uc.h include/uc/uc_mem.h include/uc/uc_string.h \
                        include/uc/processor.h include/uc/processor-apic.h \
                        include/uc/processor-cpuid.h  include/uc/processor-msr.h \
                        include/uc/processor-mtrrs.h include/uc/types.h include/uc/listentry.h


noinst_HEADERS =  	      	

install-data-hook:
	test -L $(includedir)/uc/uc && rm $(includedir)/uc/uc && ln -s $(includedir)/uc/. $(includedir)/uc/uc || ln -s $(includedir)/uc/. $(includedir)/uc/uc

uninstall-local:
	test $(includedir)/uc && rm -rf $(includedir)/uc || true

