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
#include <uvideo/vga.h>

typedef enum {
	VGA_SAVE_FONT0	= 1,  /* save/restore plane 2 fonts	  */
	VGA_SAVE_FONT1	= 2,  /* save/restore plane 3 fonts   */
	VGA_SAVE_TEXT	= 4,  /* save/restore plane 0/1 fonts */
	VGA_SAVE_FONTS	= 7,  /* save/restore all fonts	  */
	VGA_SAVE_MODE 	= 8,  /* save/restore video mode 	  */
	VGA_SAVE_CMAP	= 16, /* save/restore color map/DAC   */
	VGA_SAVE_GRAPH	= 32,	/* save/restore graphic data */
	VGA_SAVE_ALL	= 0xff
} VGA_SAVEMODE;

typedef struct {
	unsigned char	misc;
	unsigned char	fctl;
	unsigned char 	seq[VGA_SEQ_C];
	unsigned char 	crt[VGA_CRT_C];
	unsigned char 	gfx[VGA_GFX_C];
	unsigned char 	att[VGA_ATT_C];
	
	unsigned char	seq_index;
	unsigned char	crt_index;
	unsigned char	gfx_index;
	
	unsigned char	cmap[VGA_PEL_C][3];
	union {
		struct {
			unsigned char	font0[4 * 8192];
			unsigned char	font1[8 * 8192];
			unsigned char	text[2 * 8192];
		};
		unsigned char graphic[8 * 8192];
	};
	unsigned char *   vgabuffer;
	unsigned int      vgawidth;
	unsigned int      vgaheight;
} vgastate;

static bool vga_mode = false;

unsigned char UVIDEOCALL vga_seq_read(unsigned char reg) {
	outb(VGA_SEQ_I, reg);
	return inb(VGA_SEQ_D);
}

void UVIDEOCALL vga_seq_write(unsigned char reg, unsigned char val) {
	outb(VGA_SEQ_I, reg);
	outb(VGA_SEQ_D, val);
}

void UVIDEOCALL vga_seq_dump(unsigned char dump[VGA_SEQ_C]) {
	int i;

	for(i = 0 ; i < VGA_SEQ_C ; i++) {
		outb(VGA_SEQ_I, i);
		dump[i] = inb(VGA_SEQ_D);
	}
}

unsigned char UVIDEOCALL vga_gfx_read(unsigned char reg) {
	outb(VGA_GFX_I, reg);
	return inb(VGA_GFX_D);
}

void UVIDEOCALL vga_gfx_write(unsigned char reg, unsigned char val) {
	outb(VGA_GFX_I, reg);
	outb(VGA_GFX_D, val);
}

void UVIDEOCALL vga_gfx_dump(unsigned char dump[VGA_GFX_C]) {
	int i;

	for(i = 0 ; i < VGA_GFX_C ; i++) {
		outb(VGA_GFX_I, i);
		dump[i] = inb(VGA_GFX_D);
	}
}

unsigned char UVIDEOCALL vga_crt_read(unsigned char reg) {
	outb(VGA_CRT_I, reg);
	return inb(VGA_CRT_D);
}

void UVIDEOCALL vga_crt_write(unsigned char reg, unsigned char val) {
	outb(VGA_CRT_I, reg);
	outb(VGA_CRT_D, val);
}

void UVIDEOCALL vga_crt_dump(unsigned char dump[VGA_CRT_C]) {
	int i;

	for(i = 0 ; i < VGA_CRT_C ; i++) {
		outb(VGA_CRT_I, i);
		dump[i] = inb(VGA_CRT_D);
	}
}

unsigned char UVIDEOCALL vga_att_read(unsigned char reg) {
	unsigned short status_reg;
	unsigned char val;

	if(inb(VGA_MISC_R) & 1) {
		status_reg = VGA_IS1_RC;
	} else {
		status_reg = VGA_IS1_RM;
	}

	// reset flip-flop first ... 
	inb(status_reg);
	outb(VGA_ATT_IW, 0);

	inb(status_reg);
	outb(VGA_ATT_IW, reg);
	val = inb(VGA_ATT_DR);
	
	// ... end access to attributes registers
	inb(status_reg);
	outb(VGA_ATT_IW, 0x20);

	return val;
}

void UVIDEOCALL vga_att_write(unsigned char reg, unsigned char val) {
	unsigned short status_reg;

	if(inb(VGA_MISC_R) & 1) {
		status_reg = VGA_IS1_RC;
	} else {
		status_reg = VGA_IS1_RM;
	}

	// reset flip-flop first ... 
	inb(status_reg);
	outb(VGA_ATT_IW, 0);

	inb(status_reg);
	outb(VGA_ATT_IW, reg);
	outb(VGA_ATT_IW, val);
	
	// ... end access to attributes registers
	inb(status_reg);
	outb(VGA_ATT_IW, 0x20);
}

void UVIDEOCALL vga_att_dump(unsigned char dump[VGA_ATT_C]) {
	unsigned short status_reg;
	int i;

	if(inb(VGA_MISC_R) & 1) {
		status_reg = VGA_IS1_RC;
	} else {
		status_reg = VGA_IS1_RM;
	}

	// reset flip-flop first ... 
	inb(status_reg);
	outb(VGA_ATT_IW, 0);

	for(i = 0 ; i < VGA_CRT_C ; i++) {
		inb(status_reg);
		outb(VGA_ATT_IW, i);
		dump[i] = inb(VGA_ATT_DR);
	}

	// ... end access to attributes registers
	inb(status_reg);
	outb(VGA_ATT_IW, 0x20);
}

void UVIDEOCALL vga_att_conf(const unsigned char conf[VGA_ATT_C]) {
	unsigned short status_reg;
	int i;

	if(inb(VGA_MISC_R) & 1) {
		status_reg = VGA_IS1_RC;
	} else {
		status_reg = VGA_IS1_RM;
	}

	// reset flip-flop first ... 
	inb(status_reg);
	outb(VGA_ATT_IW, 0);

	for(i = 0 ; i < VGA_CRT_C ; i++) {
		inb(status_reg);
		outb(VGA_ATT_IW, i);
		outb(VGA_ATT_IW, conf[i]);
	}

	// ... end access to attributes registers
	inb(status_reg);
	outb(VGA_ATT_IW, 0x20);
}

void UVIDEOCALL vga_pel_dump(unsigned char dump[VGA_PEL_C][3]) {
	int i;

	outb(VGA_PEL_MSK, 0xff);
	outb(VGA_PEL_IR, 0);

	for(i = 0 ; i < VGA_PEL_C ; i++) {
		dump[i][0] = inb(VGA_PEL_D);	//RED
		dump[i][1] = inb(VGA_PEL_D);	//GREEN
		dump[i][2] = inb(VGA_PEL_D);	//BLUE
	}
}

void UVIDEOCALL vga_pel_conf(const unsigned char conf[VGA_PEL_C][3]) {
	int i;

	outb(VGA_PEL_MSK, 0xff);
	outb(VGA_PEL_IW, 0);

	for(i = 0 ; i < VGA_PEL_C ; i++) {
		outb(VGA_PEL_D, conf[i][0]);	//RED
		outb(VGA_PEL_D, conf[i][1]);	//GREEN
		outb(VGA_PEL_D, conf[i][2]);	//BLUE
	}
}

void UVIDEOCALL vga_pel_get(unsigned char (* dump)[3], unsigned char size, unsigned char offset) {
	unsigned char i;

	if(offset + size < offset) {
		//Overflow ...
		return;
	}

	outb(VGA_PEL_MSK, 0xff);
	outb(VGA_PEL_IR, offset);

	for(i = 0 ; i < size ; i++) {
		dump[i][0] = inb(VGA_PEL_D);	//RED
		dump[i][1] = inb(VGA_PEL_D);	//GREEN
		dump[i][2] = inb(VGA_PEL_D);	//BLUE
	}
}

void UVIDEOCALL vga_pel_set(unsigned char (* conf)[3], unsigned char size, unsigned char offset) {
	unsigned char i;

	if(offset + size < offset) {
		//Overflow ...
		return;
	}

	outb(VGA_PEL_MSK, 0xff);
	outb(VGA_PEL_IW, offset);

	for(i = 0 ; i < size ; i++) {
		outb(VGA_PEL_D, conf[i][0]);	//RED
		outb(VGA_PEL_D, conf[i][1]);	//GREEN
		outb(VGA_PEL_D, conf[i][2]);	//BLUE
	}
}

static void save_vgastate(vgastate * state, VGA_SAVEMODE mode) {
	unsigned int i;
	
	// Save color map
	if(mode & VGA_SAVE_CMAP) {
		vga_pel_dump(state->cmap);
	}
	
	// Save mode
	if(mode & VGA_SAVE_MODE) {
		state->misc = inb(VGA_MISC_R);
		state->fctl = inb(VGA_FCT_R);
		state->seq_index = inb(VGA_SEQ_I);
		state->crt_index = inb(VGA_CRT_I);
		state->gfx_index = inb(VGA_GFX_I);
	
		vga_seq_dump(state->seq);
		vga_crt_dump(state->crt);
		vga_gfx_dump(state->gfx);
		vga_att_dump(state->att);
	}

	// Save fonts if text mode
	if((! (state->att[VGA_ATT_MODE] & 1)) && (mode & VGA_SAVE_FONTS)) {
		// Blank screen
		vga_seq_write(VGA_SEQ_RESET, 1);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] | 0x20);
		vga_seq_write(VGA_SEQ_RESET, 3);

		// Save font at plane 2
		if(mode & VGA_SAVE_FONT0) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 4);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 2);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 4 * 8192 ; i++)  {
				state->font0[i] = *((unsigned char *) VGA_GFX_BUFFER + i);
			}
		}

		// Save font at plane 3
		if(mode & VGA_SAVE_FONT1) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 8);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 3);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8 * 8192 ; i++)  {
				state->font1[i] = *((unsigned char *) VGA_GFX_BUFFER + i);
			}
		}

		// Save font at plane 0
		if(mode & VGA_SAVE_TEXT) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 1);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 0);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8192 ; i++)  {
				state->text[i] = *((unsigned char *) VGA_GFX_BUFFER + i);
			}

			// Save font at plane 1
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 2);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 1);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8192 ; i++)  {
				state->text[i + 8192] = *((unsigned char *) VGA_GFX_BUFFER + i);
			}
		}

		// Restore regs
		vga_seq_write(VGA_SEQ_PLANE_WRITE, state->seq[VGA_SEQ_PLANE_WRITE]);
		vga_seq_write(VGA_SEQ_MEMORY_MODE, state->seq[VGA_SEQ_MEMORY_MODE]);
		vga_gfx_write(VGA_GFX_PLANE_READ, state->gfx[VGA_GFX_PLANE_READ]);
		vga_gfx_write(VGA_GFX_MODE, state->gfx[VGA_GFX_MODE]);
		vga_gfx_write(VGA_GFX_MISC, state->gfx[VGA_GFX_MISC]);

		// Unblank screen
		vga_seq_write(VGA_SEQ_RESET, 1);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] & ~(0x20));
		vga_seq_write(VGA_SEQ_RESET, 3);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE]);

	} else if(((state->att[VGA_ATT_MODE] & 1)) && (mode & VGA_SAVE_GRAPH)) {
		for(i = 0 ; i < 8 * 8192 ; i++) {
			state->graphic[i] = *((unsigned char *) VGA_GFX_BUFFER + i);
		}
	}

	// Restore some indexes
	{		
		outb(VGA_SEQ_I, state->seq_index);
		outb(VGA_CRT_I, state->crt_index);
		outb(VGA_GFX_I, state->gfx_index);
	}
}

static void restore_vgastate(const vgastate * state, VGA_SAVEMODE mode) {
	unsigned int i;
	
	// Restore mode
	if(mode & VGA_SAVE_MODE) {
		outb(VGA_MISC_R, state->misc);
		outb(VGA_FCT_R, state->fctl);
	
		// Turn off display
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] | 0x20);
	
		// Disable sequencer
		vga_seq_write(VGA_SEQ_RESET, 1);
	
		for(i = 2 ; i < VGA_SEQ_C ; i++) {
			vga_seq_write(i, state->seq[i]);
		}
	
		// Unprotect vga regs
		vga_crt_write(VGA_CRT_V_SYNC_END, state->crt[VGA_CRT_V_SYNC_END] & ~0x80);
	
		for(i = 0 ; i < VGA_CRT_C ; i++) {
			if(i == VGA_CRT_V_SYNC_END)
				vga_crt_write(i, state->crt[i] & ~0x80);
			else
				vga_crt_write(i, state->crt[i]);
		}
		vga_crt_write(VGA_CRT_V_SYNC_END, state->crt[VGA_CRT_V_SYNC_END]);
		
	
		for(i = 0 ; i < VGA_GFX_C ; i++) {
			vga_gfx_write(i, state->gfx[i]);
		}
	
		vga_att_conf(state->att);
	
		vga_seq_write(VGA_SEQ_RESET, 3);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] & ~(0x20));
	}
	
	// Restore fonts
	if((! (state->att[VGA_ATT_MODE] & 1)) && (mode & VGA_SAVE_FONTS)) {
		// Blank screen
		vga_seq_write(VGA_SEQ_RESET, 1);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] | 0x20);
		vga_seq_write(VGA_SEQ_RESET, 3);

		// Restore font at plane 2
		if(mode & VGA_SAVE_FONT0) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 4);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 2);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 4 * 8192 ; i++)  {
				*((unsigned char *) VGA_GFX_BUFFER + i) = state->font0[i];
			}
		}
		
		// Restore font at plane 3
		if(mode & VGA_SAVE_FONT1) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 8);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 3);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8 * 8192 ; i++)  {
				*((unsigned char *) VGA_GFX_BUFFER + i) = state->font1[i];
			}
		}

		// Restore font at plane 0
		if(mode & VGA_SAVE_TEXT) {
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 1);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 0);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8192 ; i++)  {
				*((unsigned char *) VGA_GFX_BUFFER + i) = state->text[i];
			}

			// Restore font at plane 1
			vga_seq_write(VGA_SEQ_PLANE_WRITE, 2);
			vga_seq_write(VGA_SEQ_MEMORY_MODE, 6);
			vga_gfx_write(VGA_GFX_PLANE_READ, 1);
			vga_gfx_write(VGA_GFX_MODE, 0);
			vga_gfx_write(VGA_GFX_MISC, 5);

			for(i = 0 ; i < 8192 ; i++)  {
				*((unsigned char *) VGA_GFX_BUFFER + i) = state->text[i + 8192];
			}
		}

		// Unblank screen
		vga_seq_write(VGA_SEQ_RESET, 1);
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE] & ~(0x20));
		vga_seq_write(VGA_SEQ_RESET, 3);
	
		// Restore regs
		vga_gfx_write(VGA_GFX_SR_ENABLE, state->gfx[VGA_GFX_SR_ENABLE]);
		vga_gfx_write(VGA_GFX_DATA_ROTATE, state->gfx[VGA_GFX_DATA_ROTATE]);
		vga_gfx_write(VGA_GFX_PLANE_READ, state->gfx[VGA_GFX_PLANE_READ]);
		vga_gfx_write(VGA_GFX_MODE, state->gfx[VGA_GFX_MODE]);
		vga_gfx_write(VGA_GFX_MISC, state->gfx[VGA_GFX_MISC]);
		vga_gfx_write(VGA_GFX_BIT_MASK, state->gfx[VGA_GFX_BIT_MASK]);
		
		vga_seq_write(VGA_SEQ_CLOCK_MODE, state->seq[VGA_SEQ_CLOCK_MODE]);
		vga_seq_write(VGA_SEQ_PLANE_WRITE, state->seq[VGA_SEQ_PLANE_WRITE]);
		vga_seq_write(VGA_SEQ_MEMORY_MODE, state->seq[VGA_SEQ_MEMORY_MODE]);
	} /*else if(((state->att[VGA_ATT_MODE] & 1)) && (mode & VGA_SAVE_GRAPH)) {
		for(i = 0 ; i < 8 * 8192 ; i++) {
			state->graphic[i] = *((unsigned char *) VGA_GFX_BUFFER + i);
		}
	}*/

	// Restore color map
	if(mode & VGA_SAVE_CMAP) {
		vga_pel_conf(state->cmap);
	}

	// Restore some indexes
	{		
		outb(VGA_SEQ_I, state->seq_index);
		outb(VGA_CRT_I, state->crt_index);
		outb(VGA_GFX_I, state->gfx_index);
	}
}

static const vgastate vgamodes[] = {
	{
		0x67, 0x00,
		{ 	0x03, 0x00, 0x03, 0x00, 0x02 },
		{ 	0x5f, 0x4f, 0x50, 0x82, 0x55, 0x81, 0xbf, 0x1f, 
			0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x9c, 0x8e, 0x8f, 0x28, 0x1f, 0x96, 0xb9, 0xa3,
			0xff },
		{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00,
			0xff },
		{ 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
			0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
			0x0c, 0x00, 0x0f, 0x08, 0x00 },
		0, 0, 0,
		{ { 0 } },
		{{
			{ 0 },
			{ 0 },
			{ 0 }
		}},
		(unsigned char *) VGA_TEXT_BUFFER, 80, 25
	},
	{
		0x41, 0x00,
		{ 	0x03, 0x01, 0x0f, 0x00, 0x0e },
		{ 	0x5f, 0x4f, 0x50, 0x82, 0x54, 0x80, 0xbf, 0x1f, 
			0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x9c, 0x8e, 0x8f, 0x28, 0x40, 0x96, 0xb9, 0xe3,
			0xff },
		{ 	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x00,
			0xff },
		{ 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x41, 0x00, 0x0f, 0x00, 0x00 },
		0, 0, 0,
		{ { 0 } },
		{{
			{ 0 },
			{ 0 },
			{ 0 }
		}},
		(unsigned char *) VGA_GFX_BUFFER, 320, 200
	}
};

static vgastate state;
static unsigned char * vga_buffer = 0;
static unsigned int vga_width = 0;
static unsigned int vga_height = 0;

void UVIDEOCALL vga_save(void) {
	save_vgastate(&state, VGA_SAVE_ALL);
}

void UVIDEOCALL vga_restore(void) {
	restore_vgastate(&state, VGA_SAVE_ALL);
}

uvideo_err_t UVIDEOCALL vga_init(VGAMODE mode) {	
	switch(mode) {
	case VGA_TEXT_80x25:
	   vga_buffer = vgamodes[VGA_TEXT_80x25].vgabuffer;
	   vga_width = vgamodes[VGA_TEXT_80x25].vgawidth;
	   vga_height = vgamodes[VGA_TEXT_80x25].vgaheight;
		restore_vgastate(&vgamodes[VGA_TEXT_80x25], VGA_SAVE_MODE);
		break;

	case VGA_GFX_320x200x8:
	   vga_buffer = vgamodes[VGA_GFX_320x200x8].vgabuffer;
	   vga_width = vgamodes[VGA_GFX_320x200x8].vgawidth;
	   vga_height = vgamodes[VGA_GFX_320x200x8].vgaheight;
		restore_vgastate(&vgamodes[VGA_GFX_320x200x8], VGA_SAVE_MODE);
		break;
		
	default:
		return UVIDEO_ERR_BADVIDEOMODE;
	}
	
	vga_mode = true;
	
	return UVIDEO_NOERR;
}

bool UVIDEOCALL vga_isenable(void) {
   return vga_mode;
}

unsigned char * UVIDEOCALL vga_getbuffer(void) {
   return vga_buffer;
}

unsigned int UVIDEOCALL vga_getwidth(void) {
   return vga_width;
}

unsigned int UVIDEOCALL vga_getheight(void) {
   return vga_height;
}


