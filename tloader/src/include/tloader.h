#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef __UNUSED__
#define __UNUSED__ __attribute__ ((__unused__))
#endif

#define ENTRY(name)                             \
  .globl name;                                  \
  .align 16,0x90;                               \
  name:

#define VARIABLE(x) ENTRY(x)

# define EXT_C(sym)     sym
#define  LOCAL(sym)     L_ ## sym
#define  ABS(sym)       L_ ## sym + REALCODE_ADDR


#define TLOADER_BASE_ADDR	0x06200000
#define TLOADER_START		(TLOADER_BASE_ADDR + 0x1000)

#define STACK_SIZE		(16 * 1024)

#define cs_sel      (1<<3)
#define ds_sel      (2<<3)
#define cs16_sel    (4<<3)
#define ds16_sel    (5<<3)

#define __data     __attribute__ ((__section__ (".data")))
#define __text     __attribute__ ((__section__ (".text")))

#ifndef __packed
#define __packed   __attribute__ ((packed))
#endif

#define inline        __inline__
#define always_inline __inline__ __attribute__ ((always_inline))


#define TLOADER_LOCALITY      2
#define PCR_TLOADER_CODE      21
#define PCR_TLOADER_CONFSEC   22

#endif

