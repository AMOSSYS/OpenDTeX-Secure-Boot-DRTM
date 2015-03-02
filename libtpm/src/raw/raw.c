// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organizations nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <uc/processor.h>
#include <libtpm/libtpm.h>
#include "core.h"

#pragma pack(push, 1)

// TPM_ACCESS_x
typedef union {
   uint8_t  _raw[1];                     // 1-byte reg

   struct {
      uint8_t  tpm_establishment   : 1;  // RO, 0=T/OS has been established before
      uint8_t  request_use         : 1;  // RW, 1=locality is requesting TPM use */
      uint8_t  pending_request     : 1;  // RO, 1=other locality is requesting TPM usage
      uint8_t  seize               : 1;  // WO, 1=seize locality
      uint8_t  been_seized         : 1;  // RW, 1=locality seized while active
      uint8_t  active_locality     : 1;  // RW, 1=locality is active
      uint8_t  reserved            : 1;
      uint8_t  tpm_reg_valid_sts   : 1;  // RO, 1=other bits are valid
   };
} tpm_reg_access_t;

// TPM_STS_x
typedef union {
   uint8_t _raw[3];                  // 3-byte reg

   struct {
      uint8_t  reserved1       : 1;
      uint8_t  response_retry  : 1;  // WO, 1=re-send response
      uint8_t  reserved2       : 1;
      uint8_t  expect          : 1;  // RO, 1=more data for command expected
      uint8_t  data_avail      : 1;  // RO, 0=no more data for response
      uint8_t  tpm_go          : 1;  // WO, 1=execute sent command
      uint8_t  command_ready   : 1;  // RW, 1=TPM ready to receive new cmd
      uint8_t  sts_valid       : 1;  // RO, 1=data_avail and expect bits are valid
      uint16_t burst_count     : 16; // RO, # read/writes bytes before wait
   };
} tpm_reg_sts_t;

// TPM_DATA_FIFO_x
#define TPM_REG_DATA_FIFO        0x24
typedef union {
   uint8_t  _raw[1];                 // 1-byte reg
} tpm_reg_data_fifo_t;

#pragma pack(pop)


#define readb(va)	(*(volatile uint8_t *) (va))
#define readw(va)	(*(volatile uint16_t *) (va))
#define readl(va)	(*(volatile uint32_t *) (va))

#define writeb(va, d)	(*(volatile uint8_t *) (va) = (d))
#define writew(va, d)	(*(volatile uint16_t *) (va) = (d))
#define writel(va, d)	(*(volatile uint32_t *) (va) = (d))

#define read_tpm_reg(tpmbase, locality, reg, pdata) _read_tpm_reg(tpmbase, locality, reg, (pdata)->_raw, sizeof(*(pdata)))
#define write_tpm_reg(tpmbase, locality, reg, pdata) _write_tpm_reg(tpmbase, locality, reg, (pdata)->_raw, sizeof(*(pdata)))

static void _read_tpm_reg(uint8_t * tpmbase, uint32_t locality, uint32_t reg, uint8_t *_raw, uint32_t size) {
   uint32_t i;
   
   for(i = 0 ; i < size ; i++) {
      _raw[i] = readb((TPM_LOCALITY_BASE_N(tpmbase, locality) + reg) + i);
   }
}

static void _write_tpm_reg(uint8_t * tpmbase, uint32_t locality, uint32_t reg, uint8_t *_raw, uint32_t size) {
   uint32_t i;
   
   for(i = 0 ; i < size ; i++) {
      writeb((TPM_LOCALITY_BASE_N(tpmbase, locality) + reg) + i, _raw[i]);
   }
}

/*
 * the following inline function reversely copy the bytes from 'in' to
 * 'out', the byte number to copy is given in count.
 */
#define reverse_copy(out, in, count) _reverse_copy((uint8_t *)(out), (uint8_t *)(in), count)

static inline void _reverse_copy(uint8_t * out, uint8_t * in, uint32_t count) {
   for(uint32_t i = 0 ; i < count ; i++ ) {
      out[i] = in[count - i - 1];
   }
}

static bool tpm_validate_locality(uint8_t * tpmbase, uint32_t locality) {
    uint32_t i;
    tpm_reg_access_t reg_acc;

   for(i = TPM_VALIDATE_LOCALITY_TIME_OUT ; i > 0; i--) {
      /*
      * TCG spec defines reg_acc.tpm_reg_valid_sts bit to indicate whether
      * other bits of access reg are valid.( but this bit will also be 1
      * while this locality is not available, so check seize bit too)
      * It also defines that reading reg_acc.seize should always return 0
      */
      
      read_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
      if(reg_acc.tpm_reg_valid_sts == 1 && reg_acc.seize == 0) {
         return true;
      }
      
      pause();
   }

   return false;
}

bool LIBTPMCALL tpmraw_is_ready(uint8_t * tpmbase, uint32_t locality) {
   tpm_reg_access_t reg_acc;

   if(! tpm_validate_locality(tpmbase, locality)) {
      return false;
   }

   /*
   * must ensure TPM_ACCESS_0.activeLocality bit is clear
   * (: locality is not active)
   */
   read_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
   if(reg_acc.active_locality != 0) {
      /* make inactive by writing a 1 */
      reg_acc.active_locality = 1;
      write_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
   }

   if(! tpm_validate_locality(tpmbase, locality)) {
      return false;
   }

   read_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
   if(reg_acc.active_locality != 0) {
     return false;
   }

   return true;
}

void LIBTPMCALL tpmraw_deactivate_locality(uint8_t * tpmbase, uint32_t locality) {
   tpm_reg_access_t reg_acc;

   reg_acc._raw[0] = 0;
   reg_acc.active_locality = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
}

#define TIMEOUT_UNIT    (0x100000 / 330) /* ~1ms, 1 tpm r/w need > 330ns */
#define TIMEOUT_A       750  /* 750ms */
#define TIMEOUT_B       2000 /* 2s */
#define TIMEOUT_C       750  /* 750ms */
#define TIMEOUT_D       750  /* 750ms */

typedef struct __packed {
   uint32_t timeout_a;
   uint32_t timeout_b;
   uint32_t timeout_c;
   uint32_t timeout_d;
} tpm_timeout_t;

static tpm_timeout_t g_timeout = { TIMEOUT_A, TIMEOUT_B, TIMEOUT_C, TIMEOUT_D };

#define TPM_ACTIVE_LOCALITY_TIME_OUT   (TIMEOUT_UNIT * g_timeout.timeout_a)  /* according to spec */
#define TPM_CMD_READY_TIME_OUT         (TIMEOUT_UNIT * g_timeout.timeout_b)  /* according to spec */
#define TPM_CMD_WRITE_TIME_OUT         (TIMEOUT_UNIT * g_timeout.timeout_d)  /* let it long enough */
#define TPM_DATA_AVAIL_TIME_OUT        (TIMEOUT_UNIT * g_timeout.timeout_c)  /* let it long enough */
#define TPM_RSP_READ_TIME_OUT          (TIMEOUT_UNIT * g_timeout.timeout_d)  /* let it long enough */

static uint32_t tpm_wait_cmd_ready(uint8_t * tpmbase, uint32_t locality) {
   uint32_t      i;
   tpm_reg_access_t  reg_acc;
   tpm_reg_sts_t     reg_sts;

   // ensure the contents of the ACCESS register are valid
   read_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);

   if( reg_acc.tpm_reg_valid_sts == 0) {
      LOGERROR("Access reg not valid\n");
      return TPM_E_FAIL;
   }

   // request access to the TPM from locality N
   reg_acc._raw[0] = 0;
   reg_acc.request_use = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);

   i = 0;
   do {
      read_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);
      if(reg_acc.active_locality == 1) {
         break;
      } else {
         pause();
      }

      i++;
   } while(i <= TPM_ACTIVE_LOCALITY_TIME_OUT);

   if(i > TPM_ACTIVE_LOCALITY_TIME_OUT) {
      LOGERROR("access reg request use timeout\n");
      return TPM_E_FAIL;
   }

   // ensure the TPM is ready to accept a command
   i = 0;
   do {
      // write 1 to TPM_STS_x.commandReady to let TPM enter ready state
      memset((void *)&reg_sts, 0, sizeof(reg_sts));
      reg_sts.command_ready = 1;
      write_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);
      pause();

      // then see if it has
      read_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);

      if(reg_sts.command_ready == 1) {
         break;
      } else {
         pause();
      }
      
      i++;
   } while(i <= TPM_CMD_READY_TIME_OUT);


   if(i > TPM_CMD_READY_TIME_OUT) {
      LOGERROR("status reg content: %02x %02x %02x\n", (uint32_t)reg_sts._raw[0], (uint32_t)reg_sts._raw[1], (uint32_t)reg_sts._raw[2]);
      LOGERROR("tpm timeout for command_ready\n");
      goto RelinquishControl;
   }
   
   return TPM_SUCCESS;

RelinquishControl:
   // deactivate current locality
   reg_acc._raw[0] = 0;
   reg_acc.active_locality = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);

   return TPM_E_FAIL;
}

#define CMD_HEAD_SIZE           10
#define RSP_HEAD_SIZE           10
#define CMD_SIZE_OFFSET         2
#define CMD_ORD_OFFSET          6
#define RSP_SIZE_OFFSET         2
#define RSP_RST_OFFSET          6

uint32_t LIBTPMCALL tpmraw_write_cmd_fifo(uint8_t * tpmbase, uint32_t locality, uint8_t * in, uint32_t in_size, uint8_t * out, uint32_t * out_size) {
   uint32_t      i, rsp_size, offset, ret;
   uint16_t    row_size;
   tpm_reg_access_t  reg_acc;
   tpm_reg_sts_t     reg_sts;

   if(locality >= TPM_NR_LOCALITIES) {
      LOGERROR("Invalid locality : %u", locality);
      return TPM_E_BAD_PARAMETER;
   }
   
   if(in == NULL || out == NULL || out_size == NULL) {
      LOGERROR("Invalid paramet");
      return TPM_E_BAD_PARAMETER;
   }
   
   if(in_size < CMD_HEAD_SIZE || *out_size < RSP_HEAD_SIZE) {
      LOGERROR("in/out buf size must be larger than 10 bytes\n");
      return TPM_E_BAD_PARAMETER;
   }

   if(! tpm_validate_locality(tpmbase, locality)) {
      LOGERROR("Locality %u is not open\n", locality);
      return TPM_E_FAIL;
   }

   ret = tpm_wait_cmd_ready(tpmbase, locality);
   if(ret != TPM_SUCCESS) {
      return ret;
   }

   // write the command to the TPM FIFO
   offset = 0;
   do {
      i = 0;
      do {
         read_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);
         // find out how many bytes the TPM can accept in a row
         row_size = reg_sts.burst_count;
         if(row_size > 0) {
            break;
         } else {
            pause();
         }
         
         i++;
      } while (i <= TPM_CMD_WRITE_TIME_OUT);
      
      if(i > TPM_CMD_WRITE_TIME_OUT) {
         LOGERROR("write cmd timeout\n");
         ret = TPM_E_FAIL;
         goto RelinquishControl;
      }

      for( ; row_size > 0 && offset < in_size; row_size--, offset++) {
         write_tpm_reg(tpmbase, locality, TPM_REG_DATA_FIFO, (tpm_reg_data_fifo_t *) &in[offset]);
      }
   } while(offset < in_size);

   i = 0;
   do {
      read_tpm_reg(tpmbase, locality,TPM_REG_STS, &reg_sts);
      if( reg_sts.sts_valid == 1 && reg_sts.expect == 0) {
         break;
      } else {
         pause();
      }
      i++;
   } while(i <= TPM_DATA_AVAIL_TIME_OUT);

   if(i > TPM_DATA_AVAIL_TIME_OUT) {
      LOGERROR("wait for expect becoming 0 timeout\n");
      ret = TPM_E_FAIL;
      goto RelinquishControl;
   }

   // command has been written to the TPM, it is time to execute it.
   memset(&reg_sts, 0,  sizeof(reg_sts));
   reg_sts.tpm_go = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);

   // check for data available
   i = 0;
   do {
      read_tpm_reg(tpmbase, locality,TPM_REG_STS, &reg_sts);

      if(reg_sts.sts_valid == 1 && reg_sts.data_avail == 1) {
         break;
      } else {
         pause();
      }
      i++;
   } while(i <= TPM_DATA_AVAIL_TIME_OUT);
   
   if(i > TPM_DATA_AVAIL_TIME_OUT) {
      LOGERROR("wait for data available timeout\n");
      ret = TPM_E_FAIL;
      goto RelinquishControl;
   }

   rsp_size = 0;
   offset = 0;
   do {
      // find out how many bytes the TPM returned in a row
      i = 0;
      do {
         read_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);
         row_size = reg_sts.burst_count;
         if(row_size > 0) {
            break;
         } else {
            pause();
         }
         i++;
      } while(i <= TPM_RSP_READ_TIME_OUT);
      
      if(i > TPM_RSP_READ_TIME_OUT) {
         LOGERROR("read rsp timeout\n");
         ret = TPM_E_FAIL;
         goto RelinquishControl;
      }

      for( ; row_size > 0 && offset < *out_size; row_size--, offset++) {
         if(offset < *out_size) {
            read_tpm_reg(tpmbase, locality, TPM_REG_DATA_FIFO, (tpm_reg_data_fifo_t *)&out[offset]);
         } else {
            // discard the responded bytes exceeding out buf size
            tpm_reg_data_fifo_t discard;
            read_tpm_reg(tpmbase, locality, TPM_REG_DATA_FIFO, (tpm_reg_data_fifo_t *)&discard);
         }

         // get outgoing data size
         if(offset == RSP_RST_OFFSET - 1) {
            reverse_copy(&rsp_size, &out[RSP_SIZE_OFFSET], sizeof(rsp_size));
         }
      }
   } while(offset < RSP_RST_OFFSET || (offset < rsp_size && offset < *out_size));

   *out_size = (*out_size > rsp_size) ? rsp_size : *out_size;

   // out buffer contains the complete outgoing data, get return code
   reverse_copy(&ret, &out[RSP_RST_OFFSET], sizeof(ret));

   memset(&reg_sts, 0, sizeof(reg_sts));
   reg_sts.command_ready = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_STS, &reg_sts);

RelinquishControl:
   // deactivate current locality
   reg_acc._raw[0] = 0;
   reg_acc.active_locality = 1;
   write_tpm_reg(tpmbase, locality, TPM_REG_ACCESS, &reg_acc);

   return ret;
}

