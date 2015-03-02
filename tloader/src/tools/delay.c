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


#include <uc/types.h>
#include <uc/processor.h>
#include <utils.h>

static bool g_calibrated = false;
static uint64_t g_ticks_per_millisec;

#define TIMER_FREQ	1193182
#define TIMER_DIV(hz)	((TIMER_FREQ+(hz)/2)/(hz))

static void wait_tsc_uip(void)
{
    do {
        outb(0x43, 0xe8);
        pause();
    } while ( !(inb(0x42) & 0x80) );
    do {
        outb(0x43, 0xe8);
        pause();
    } while ( inb(0x42) & 0x80 );
}

static void calibrate_tsc(void)
{
    if ( g_calibrated )
        return;

    /* disable speeker */
    uint8_t val = inb(0x61);
    val = ((val & ~0x2) | 0x1);
    outb(0x61, val);

    /* 0xb6 - counter2, low then high byte write */
    /* mode 3, binary */
    outb(0x43, 0xb6);

    /* 0x4a9 - divisor to get 1ms period time */
    /* 1.19318 MHz / 1193 = 1000.15Hz */
    uint16_t latch = TIMER_DIV(1000);
    outb(0x42, latch & 0xff);
    outb(0x42, latch >> 8);

    /* 0xe8 - read back command, don't get count */
    /* get status, counter2 select */
    do {
        outb(0x43, 0xe8);
        pause();
    } while ( inb(0x42) & 0x40 );

    wait_tsc_uip();

    /* get starting TSC val */
    uint64_t start = rdtsc();

    wait_tsc_uip();

    uint64_t end = rdtsc();

    /* # ticks in 1 millisecond */
    g_ticks_per_millisec = end - start;

    /* restore timer 1 programming */
    outb(0x43, 0x54);
    outb(0x41, 0x12);

    g_calibrated = true;
}

void delay(int millisecs)
{
    if ( millisecs <= 0 )
        return;

    calibrate_tsc();

    uint64_t rtc = rdtsc();

    uint64_t end_ticks = rtc + millisecs * g_ticks_per_millisec;
    while ( rtc < end_ticks ) {
        pause();
        rtc = rdtsc();
    }
}

