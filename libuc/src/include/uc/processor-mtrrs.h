#pragma once

#include <uc/uc.h>

/*  These are the region types  */
 #define MTRR_TYPE_UNCACHABLE 0
 #define MTRR_TYPE_WRCOMB     1
 #define MTRR_TYPE_WRTHROUGH  4
 #define MTRR_TYPE_WRPROT     5
 #define MTRR_TYPE_WRBACK     6
 #define MTRR_NUM_TYPES       7

enum fix_mtrr_t {
    MTRR_FIX64K_00000 = 0x250,
    MTRR_FIX16K_80000 = 0x258,
    MTRR_FIX16K_A0000 = 0x259,
    MTRR_FIX4K_C0000  = 0x268,
    MTRR_FIX4K_C8000  = 0x269,
    MTRR_FIX4K_D0000  = 0x26A,
    MTRR_FIX4K_D8000  = 0x26B,
    MTRR_FIX4K_E0000  = 0x26C,
    MTRR_FIX4K_E8000  = 0x26D,
    MTRR_FIX4K_F0000  = 0x26E,
    MTRR_FIX4K_F8000  = 0x26F
};

typedef union {
    uint64_t raw;
    uint8_t  type[8];
} mtrr_fix_types_t;

enum var_mtrr_t {
    MTRR_PHYS_BASE0_MSR = 0x200,
    MTRR_PHYS_MASK0_MSR = 0x201,
    MTRR_PHYS_BASE1_MSR = 0x202,
    MTRR_PHYS_MASK1_MSR = 0x203,
    MTRR_PHYS_BASE2_MSR = 0x204,
    MTRR_PHYS_MASK2_MSR = 0x205,
    MTRR_PHYS_BASE3_MSR = 0x206,
    MTRR_PHYS_MASK3_MSR = 0x207,
    MTRR_PHYS_BASE4_MSR = 0x208,
    MTRR_PHYS_MASK4_MSR = 0x209,
    MTRR_PHYS_BASE5_MSR = 0x20A,
    MTRR_PHYS_MASK5_MSR = 0x20B,
    MTRR_PHYS_BASE6_MSR = 0x20C,
    MTRR_PHYS_MASK6_MSR = 0x20D,
    MTRR_PHYS_BASE7_MSR = 0x20E,
    MTRR_PHYS_MASK7_MSR = 0x20F
};

typedef union {
    uint64_t    raw;
    struct {
        uint64_t vcnt        : 8;    /* num variable MTRR pairs */
        uint64_t fix         : 1;    /* fixed range MTRRs are supported */
        uint64_t reserved1   : 1;
        uint64_t wc          : 1;    /* write-combining mem type supported */
        uint64_t reserved2   : 53;
    };
} mtrr_cap_t;

typedef union {
    uint64_t    raw;
    struct {
        uint64_t type        : 8;
        uint64_t reserved1   : 2;
        uint64_t fe          : 1;    /* fixed MTRR enable */
        uint64_t e           : 1;    /* (all) MTRR enable */
        uint64_t reserved2   : 52;
    };
} mtrr_def_type_t;

typedef union {
    uint64_t    raw;
    struct {
        uint64_t type      : 8;
        uint64_t reserved1 : 4;
        uint64_t base      : 24;
        uint64_t reserved2 : 28;
    };
} mtrr_physbase_t;

typedef union {
    uint64_t    raw;
    struct {
        uint64_t reserved1 : 11;
        uint64_t v         : 1;      /* valid */
        uint64_t mask      : 24;
        uint64_t reserved2 : 28;
    };
} mtrr_physmask_t;

/* current procs only have 8, so this should hold us for a while */
#define MAX_VARIABLE_MTRRS      16

typedef struct {
   mtrr_def_type_t   mtrr_def_type;
   int               num_var_mtrrs;
   mtrr_physbase_t   mtrr_physbases[MAX_VARIABLE_MTRRS];
   mtrr_physmask_t   mtrr_physmasks[MAX_VARIABLE_MTRRS];
} mtrr_state_t;

bool  UCCALL set_mem_type(uint64_t base, uint32_t size, uint32_t mem_type);
void  UCCALL set_all_mtrrs(bool enable);
void  UCCALL save_mtrrs(mtrr_state_t * saved_state);
void  UCCALL restore_mtrrs(const mtrr_state_t * saved_state);
bool  UCCALL set_mtrrs_for_acmod(uint64_t sinit, uint64_t size);
bool  UCCALL validate_mtrrs(const mtrr_state_t * saved_state);

