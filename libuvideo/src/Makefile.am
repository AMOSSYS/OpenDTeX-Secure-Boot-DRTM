AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99
AM_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -Iinclude
AM_CFLAGS += -D__UVIDEO_COMPILING__

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -O2 -nostdinc -isystem `$(CC) -print-file-name=include` -std=gnu99 -D__ASSEMBLY__ -DASM_FILE
AM_CCASFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -fno-dwarf2-cfi-asm
AM_CCASFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -Iinclude
AM_CCASFLAGS += -D__UVIDEO_COMPILING__

SOURCE = video.c  video_fonts.c  video_picture.c vga.c

lib_LIBRARIES = libuvideo.a libuvideo-pic.a

libuvideo_a_SOURCES = $(SOURCE)
libuvideo_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS)
libuvideo_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS)

libuvideo_pic_a_SOURCES = $(SOURCE)
libuvideo_pic_a_CFLAGS = $(AM_CFLAGS) $(LIBUC_CFLAGS) -fPIE
libuvideo_pic_a_CCASFLAGS = $(AM_CCASFLAGS) $(LIBUC_CFLAGS) -fPIE

libuvideoincludedir = $(includedir)/uvideo
libuvideoinclude_HEADERS = 	include/uvideo/uvideo.h include/uvideo/vbe.h include/uvideo/vga.h

noinst_HEADERS =  	      	
									
install-data-hook:
	test -L $(includedir)/uvideo/uvideo && rm $(includedir)/uvideo/uvideo && ln -s $(includedir)/uvideo/. $(includedir)/uvideo/uvideo || ln -s $(includedir)/uvideo/. $(includedir)/uvideo/uvideo
	
uninstall-local:
	test $(includedir)/uvideo && rm -rf $(includedir)/uvideo || true

