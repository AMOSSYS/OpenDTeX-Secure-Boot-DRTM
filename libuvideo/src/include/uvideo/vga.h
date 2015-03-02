#pragma once

#include <uvideo/uvideo.h>

typedef enum {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE,
	VGA_COLOR_GREEN,
	VGA_COLOR_CYAN,
	VGA_COLOR_RED,
	VGA_COLOR_MAGENTA,
	VGA_COLOR_BROWN,
	VGA_COLOR_LGREY,
	VGA_COLOR_DGREY,
	VGA_COLOR_LBLUE,
	VGA_COLOR_LGREEN,
	VGA_COLOR_LCYAN,
	VGA_COLOR_LRED,
	VGA_COLOR_LMAGENTA,
	VGA_COLOR_YELLOW,
	VGA_COLOR_WHITE
} VGACOLOR;

typedef enum {
	VGA_TEXT_80x25,	// Mode 3h
	VGA_GFX_320x200x8	// Mode 13h
} VGAMODE;

#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_BUFFER 0xB8000
#define VGA_GFX_BUFFER 0xA0000

#define VGA_IS1_RC			0x3DA
#define VGA_IS1_RM			0x3BA
#define VGA_MISC_W			0x3c2
#define VGA_MISC_R			0x3cc
#define VGA_FCT_W				0x3da
#define VGA_FCT_R				0x3ca

#define VGA_CRT_C   	0x19
#define VGA_ATT_C   	0x15
#define VGA_GFX_C   	0x09
#define VGA_SEQ_C   	0x05
#define VGA_MIS_C   	0x01
#define VGA_PEL_C		256

#define VGA_SEQ_I				0x3c4
#define VGA_SEQ_D				0x3c5
	#define VGA_SEQ_RESET			0x00
	#define VGA_SEQ_CLOCK_MODE		0x01
	#define VGA_SEQ_PLANE_WRITE	0x02
	#define VGA_SEQ_CHARACTER_MAP	0x03
	#define VGA_SEQ_MEMORY_MODE	0x04

#define VGA_GFX_I				0x3ce
#define VGA_GFX_D				0x3cf
	#define VGA_GFX_SR_VALUE		0x00
	#define VGA_GFX_SR_ENABLE		0x01
	#define VGA_GFX_COMPARE_VALUE	0x02
	#define VGA_GFX_DATA_ROTATE	0x03
	#define VGA_GFX_PLANE_READ		0x04
	#define VGA_GFX_MODE				0x05
	#define VGA_GFX_MISC				0x06
	#define VGA_GFX_COMPARE_MASK	0x07
	#define VGA_GFX_BIT_MASK		0x08

#define VGA_CRT_I				0x3d4
#define VGA_CRT_D				0x3d5
	#define VGA_CRT_H_TOTAL			0
	#define VGA_CRT_H_DISP			1
	#define VGA_CRT_H_BLANK_START	2
	#define VGA_CRT_H_BLANK_END	3
	#define VGA_CRT_H_SYNC_START	4
	#define VGA_CRT_H_SYNC_END		5
	#define VGA_CRT_V_TOTAL			6
	#define VGA_CRT_OVERFLOW		7
	#define VGA_CRT_PRESET_ROW		8
	#define VGA_CRT_MAX_SCAN		9
	#define VGA_CRT_CURSOR_START	0x0A
	#define VGA_CRT_CURSOR_END		0x0B
	#define VGA_CRT_START_HI		0x0C
	#define VGA_CRT_START_LO		0x0D
	#define VGA_CRT_CURSOR_HI		0x0E
	#define VGA_CRT_CURSOR_LO		0x0F
	#define VGA_CRT_V_SYNC_START	0x10
	#define VGA_CRT_V_SYNC_END		0x11
	#define VGA_CRT_V_DISP_END		0x12
	#define VGA_CRT_OFFSET			0x13
	#define VGA_CRT_UNDERLINE		0x14
	#define VGA_CRT_V_BLANK_START	0x15
	#define VGA_CRT_V_BLANK_END	0x16
	#define VGA_CRT_MODE				0x17
	#define VGA_CRT_LINE_COMPARE	0x18

#define VGA_ATT_IW			0x3c0
#define VGA_ATT_DR			0x3c1
	#define VGA_ATT_PALETTE0		0x00
	#define VGA_ATT_PALETTE1		0x01
	#define VGA_ATT_PALETTE2		0x02
	#define VGA_ATT_PALETTE3		0x03
	#define VGA_ATT_PALETTE4		0x04
	#define VGA_ATT_PALETTE5		0x05
	#define VGA_ATT_PALETTE6		0x06
	#define VGA_ATT_PALETTE7		0x07
	#define VGA_ATT_PALETTE8		0x08
	#define VGA_ATT_PALETTE9		0x09
	#define VGA_ATT_PALETTEA		0x0A
	#define VGA_ATT_PALETTEB		0x0B
	#define VGA_ATT_PALETTEC		0x0C
	#define VGA_ATT_PALETTED		0x0D
	#define VGA_ATT_PALETTEE		0x0E
	#define VGA_ATT_PALETTEF		0x0F
	#define VGA_ATT_MODE				0x10
	#define VGA_ATT_OVERSCAN		0x11
	#define VGA_ATT_PLANE_ENABLE	0x12
	#define VGA_ATT_PEL				0x13
	#define VGA_ATT_COLOR_PAGE		0x14

#define VGA_PEL_IW  			0x3C8
#define VGA_PEL_IR  			0x3C7
#define VGA_PEL_D				0x3C9
#define VGA_PEL_MSK			0x3C6


unsigned char	   UVIDEOCALL vga_seq_read(unsigned char reg);
void 				   UVIDEOCALL vga_seq_write(unsigned char reg, unsigned char val);
void 				   UVIDEOCALL vga_seq_dump(unsigned char dump[VGA_SEQ_C]);
unsigned char 	   UVIDEOCALL vga_gfx_read(unsigned char reg);
void 				   UVIDEOCALL vga_gfx_write(unsigned char reg, unsigned char val);
void 				   UVIDEOCALL vga_gfx_dump(unsigned char dump[VGA_GFX_C]);
unsigned char 	   UVIDEOCALL vga_crt_read(unsigned char reg);
void 				   UVIDEOCALL vga_crt_write(unsigned char reg, unsigned char val);
void 				   UVIDEOCALL vga_crt_dump(unsigned char dump[VGA_CRT_C]);
unsigned char 	   UVIDEOCALL vga_att_read(unsigned char reg);
void 				   UVIDEOCALL vga_att_write(unsigned char reg, unsigned char val);
void 				   UVIDEOCALL vga_att_dump(unsigned char dump[VGA_ATT_C]);
void 				   UVIDEOCALL vga_att_conf(const unsigned char conf[VGA_ATT_C]);
void 				   UVIDEOCALL vga_pel_dump(unsigned char dump[VGA_PEL_C][3]);
void 				   UVIDEOCALL vga_pel_conf(const unsigned char conf[VGA_PEL_C][3]);
void 				   UVIDEOCALL vga_pel_get(unsigned char (* dump)[3], unsigned char size, unsigned char offset);
void 				   UVIDEOCALL vga_pel_set(unsigned char (* conf)[3], unsigned char size, unsigned char offset);

void 				   UVIDEOCALL vga_save(void);
void 				   UVIDEOCALL vga_restore(void);
uvideo_err_t	   UVIDEOCALL vga_init(VGAMODE mode);
bool              UVIDEOCALL vga_isenable(void);
unsigned char *   UVIDEOCALL vga_getbuffer(void);
unsigned int      UVIDEOCALL vga_getwidth(void);
unsigned int      UVIDEOCALL vga_getheight(void);

