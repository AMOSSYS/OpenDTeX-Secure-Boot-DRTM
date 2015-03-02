// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS, Bertin
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
#include <system.h>

/**********************************************************
* INTERRUPT
**********************************************************/

int cptInt = 0;

void interrupt_handler_gen(unsigned int num, unsigned int * stack) {
	
	if((0x20 <= num) && (num < 0x30)) {
		// Ackit IRQ
		if(num >= 0x28)
			outb(0xA0, 0x20);
		outb(0x20, 0x20);
	}
	
	cptInt++;
	
	num = *stack; // foo
}

/**********************************************************
* INIT
**********************************************************/

/**
 * Tloader system entry point
 */
 
void systeminit(void) {
	//8259 PIC setup in order to map IRQ0-15 to Interrupt vector 0x20 to 0x2f
	//http://www.xbdev.net/asm/protected_mode/tut_025/index.php
	//http://www.acm.uiuc.edu/sigops/roll_your_own/i386/irq.html
	//http://www.brokenthorn.com/Resources/OSDevPic.html
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	
	outb(0x21, 0x05);
	outb(0xA1, 0x01);
	
	// Mask all IRQs except IRQ1 for keyboard and IRQ12 for mouse ...
	outb(0x21, 0xff);
	outb(0xA1, 0xff);
}


