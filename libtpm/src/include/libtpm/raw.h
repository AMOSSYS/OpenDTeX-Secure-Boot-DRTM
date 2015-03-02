#pragma once

#include <uc/types.h>
#include <uc/uc.h>

#define TPM_LOCALITY_BASE             0xfed40000
#define TPM_LOCALITY_SIZE             0x5000
/*#define NR_TPM_LOCALITY_PAGES         ((TPM_LOCALITY_1 - TPM_LOCALITY_0) >> PAGE_SHIFT)

#define TPM_LOCALITY_0                TPM_LOCALITY_BASE
#define TPM_LOCALITY_1                (TPM_LOCALITY_BASE | 0x1000)
#define TPM_LOCALITY_2                (TPM_LOCALITY_BASE | 0x2000)
#define TPM_LOCALITY_3                (TPM_LOCALITY_BASE | 0x3000)
#define TPM_LOCALITY_4                (TPM_LOCALITY_BASE | 0x4000)
#define TPM_LOCALITY_BASE_N(n)        (TPM_LOCALITY_BASE | ((n) << 12))*/
#define TPM_LOCALITY_BASE_N(base, n)  (((uint8_t *) base) + ((n) << 12))
#define TPM_NR_LOCALITIES             5

#define TPM_REG_ACCESS                 0x00
#define TPM_REG_STS                    0x18
#define TPM_VALIDATE_LOCALITY_TIME_OUT 0x100

bool     LIBTPMCALL tpmraw_is_ready(uint8_t * tpmbase, uint32_t locality);
void     LIBTPMCALL tpmraw_deactivate_locality(uint8_t * tpmbase, uint32_t locality);
uint32_t LIBTPMCALL tpmraw_write_cmd_fifo(uint8_t * tpmbase, uint32_t locality, uint8_t * in, uint32_t in_size, uint8_t * out, uint32_t * out_size);

