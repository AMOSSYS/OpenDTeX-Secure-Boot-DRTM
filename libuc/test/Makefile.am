AM_CFLAGS = $(CFLAGS) $(BUILD_CFLAGS) -std=gnu99
AM_CFLAGS += -fno-dwarf2-cfi-asm
AM_CFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wno-unused-function -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CFLAGS += -I../src/include -L ../src -g -Wno-deprecated-declarations

AM_CCASFLAGS = $(CCASFLAGS) $(BUILD_CCASFLAGS) -std=gnu99 -D__ASSEMBLY__ -DASM_FILE -fPIE
AM_CCASFLAGS += -fno-dwarf2-cfi-asm
AM_CCASFLAGS += -Wall -W -Werror -Wundef -Wsign-compare -Wunused -Wunused-parameter -Wno-unused-function -Wshadow -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes
AM_CCASFLAGS += -fno-stack-protector -mno-stack-arg-probe -fno-builtin -ffreestanding -fno-common
AM_CCASFLAGS += -I../src/include -L ../src -g -Wno-deprecated-declarations

AM_LDFLAGS = $(LDFLAGS) $(BUILD_LDFLAGS)

AM_LDADD = -luc

noinst_PROGRAMS = testuc testucnoinline

testuc_SOURCES = 	testuc.c
testuc_CFLAGS = $(AM_CFLAGS) -O2
testuc_CCASFLAGS = $(AM_CCASFLAGS) -O2
testuc_LDFLAGS = $(AM_LDFLAGS)
testuc_LDADD = $(AM_LDADD)

testucnoinline_SOURCES = 	testuc.c
testucnoinline_CFLAGS = $(AM_CFLAGS) -fno-inline -O0
testucnoinline_CCASFLAGS = $(AM_CCASFLAGS) -fno-inline -O0
testucnoinline_LDFLAGS = $(AM_LDFLAGS)
testucnoinline_LDADD = $(AM_LDADD)

