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


#include <kbd.h>
#include <uc/uc.h>

#define KBD_DATA 		0x60
#define KBD_STATUS 	0x64

#define LOOP_KBD 0x1000
#define TEMPO_KBD 0x10

typedef struct _STATE_KBD {
	unsigned char shiftL;
	unsigned char shiftR;
	unsigned char ctrlL;
	unsigned char ctrlR;
	unsigned char altL;
	unsigned char altR;
	unsigned char winL;
	unsigned char winR;
	unsigned char popup;
	unsigned char verrNum;
	unsigned char capsLock;
} STATE_KBD;

#define SCODE_ESC 0x01

#define KBD_NOP 0xff
#define KBD_ESC 0xf8
#define KBD_F1 0x01
#define KBD_F2 0x02
#define KBD_F3 0x03
#define KBD_F4 0x04
#define KBD_F5 0x05
#define KBD_F6 0x06
#define KBD_F7 0x07
#define KBD_F8 0x08
#define KBD_F9 0x09
#define KBD_F10 0x0a
#define KBD_F11 0x0b
#define KBD_F12 0x0c
#define KBD_PAUSE 0x0d
#define KBD_ARRET_DEFIL 0x0e
#define KBD_IMPR 0x0f
#define KBD_TAB 0x10
#define KBD_CAPS_LOCK 0x11
#define KBD_SHIFTL 0x12
#define KBD_CTRLL 0x13
#define KBD_ALTL 0x14
#define KBD_WINL 0x15
#define KBD_SHIFTR 0x16
#define KBD_CTRLR 0x17
#define KBD_ALTR 0x18
#define KBD_POPUP 0x19
#define KBD_ENTER 0x1a
#define KBD_BACK 0x1b
#define KBD_INS 0x1c
#define KBD_HOME 0x1d
#define KBD_PGUP 0x1e
#define KBD_DEL 0x1f
#define KBD_END 0x20
#define KBD_PGDN 0x21
#define KBD_UP 0x22
#define KBD_LEFT 0x23
#define KBD_RIGHT 0x24
#define KBD_DOWN 0x25
#define KBD_VERRNUM 0x26
#define KBD_WINR 0x27
#define KBD_PAD_ENTER 0x28
#define KBD_CTRL_UP 0x29
#define KBD_CTRL_DOWN 0x30
#define KBD_CTRL_LEFT 0x31
#define KBD_CTRL_RIGHT 0x32
#define KBD_MAJ_UP 0x33
#define KBD_MAJ_DOWN 0x34
#define KBD_MAJ_LEFT 0x35
#define KBD_MAJ_RIGHT 0x36

static inline unsigned char inb(unsigned short port) {
	unsigned char	data;
	__asm volatile("inb %w1, %0" : "=a" (data) : "Nd" (port));
	return (data);
}

static inline void pause(void) {
	__asm volatile("pause" : : );
}

STATE_KBD keyboard = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

 int kbd_waitforread(void) {
	int cpt = LOOP_KBD;
	unsigned char val;
	
	while(cpt) {
		val = inb(KBD_STATUS);
		if((val & 1) && !(val & 0x20)) {
			return 0;
		}
		//pause();
		cpt--;
	}
	
	return -1;
}

int kbd_readscancode(unsigned char scode[SCANCODESIZE]) {
	unsigned int i = 0;
	
	if(scode == 0) {
		return -1;
	}
	
	memset(scode, 0, SCANCODESIZE);
	
	while(i < 6) {
		if(kbd_waitforread() == 0) {
			scode[i] = inb(KBD_DATA);
		} else {
			if(i == 0) {
				//Timeout
				return -1;
			} else {
				continue;
			}
		}
		
		/***************************************/
		if(scode[0] == 0xe1) {
			i++;
			if(i < 6)
				continue;
			else
				return 0;
		}
		/***************************************/
		if((i == 1) && (scode[0] == 0xe0) && (scode[1] == 0x2a)) {
			i++;
			continue;
		}

		if((i == 1) && (scode[0] == 0xe0) && (scode[1] == 0xb7)) {
			i++;
			continue;
		}
		/***************************************/
		if((scode[i] != 0xf0) && (scode[i] != 0xe0))
			return 0;
		
		i++;
	}
	
	return -1;
}

static char scantable_print[] =	{
	  0,   0, '&', 130, '"', '\'', '(', '-', 138, '_', 135, 133, ')', '=',   0,   0,
	'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$',   0,   0, 'q', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 151, 253,   0, '*', 'w', 'x', 'c', 'v',
	'b', 'n', ',', ';', ':', '!',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+',  '1',
	'2', '3', '0', '.',   0,   0, '<',   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

//PROBLEMES AVEC ¨ ET µ
static char scantable_maj_print[] =	{ 
	  0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 248, '+',   0,   0,
	'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0xa8, 156,   0,   0, 'Q', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', 253,   0, 0xb5, 'W', 'X', 'C', 'V',
	'B', 'N', '?', '.', '/', 0xa7,   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+',  '1',
	'2', '3', '0', '.',   0,   0, '>',   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static char scantable_print_e0[] =	{
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0, '/',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static char scantable_maj_print_e0[] =	{
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0, '/',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static char scantable_commande[] = {
	0, KBD_ESC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KBD_BACK, KBD_TAB,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KBD_ENTER, KBD_CTRLL, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KBD_SHIFTL, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, KBD_SHIFTR, 0, KBD_ALTL, 0, KBD_CAPS_LOCK, KBD_F1, KBD_F2, KBD_F3, KBD_F4, KBD_F5,
	KBD_F6, KBD_F7, KBD_F8, KBD_F9, KBD_F10, KBD_VERRNUM, KBD_ARRET_DEFIL, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, KBD_F11, KBD_F12, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char scantable_commande_e0[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KBD_PAD_ENTER, KBD_CTRLR, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, KBD_ALTR, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, KBD_HOME, KBD_UP, KBD_PGUP, 0, KBD_LEFT, 0, KBD_RIGHT, 0, KBD_END,
	KBD_DOWN, KBD_PGDN, KBD_INS, KBD_DEL, 0, 0, 0, 0, 0, 0, 0, KBD_WINL, KBD_WINR, KBD_POPUP, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char scantable_ctrl_commande_e0[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, KBD_CTRL_UP, 0, 0, KBD_CTRL_LEFT, 0, KBD_CTRL_RIGHT, 0, 0,
	KBD_CTRL_DOWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char scantable_shift_commande_e0[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, KBD_MAJ_UP, 0, 0, KBD_MAJ_LEFT, 0, KBD_MAJ_RIGHT, 0, 0,
	KBD_MAJ_DOWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned int kbd_interpretcommand(unsigned char scode[6]) {
	unsigned int cmd;
	unsigned int pressed = 1;
	
	//PAUSE
	if((scode[0] == 0xe1) && (scode[1] == 0x1d) && (scode[2] == 0x45)) { //Incomplet
		cmd = KBD_PAUSE;
	//IMPRESSION ECRAN APPUYE
	} else if((scode[0] == 0xe0) && (scode[1] == 0x2a) && (scode[2] == 0xe0) && (scode[3] == 0x37)) {
		cmd = KBD_IMPR;
	//IMPRESSION ECRAN RLEACHE
	} else if((scode[0] == 0xe0) && (scode[1] == 0xb7) && (scode[2] == 0xe0) && (scode[3] == 0xaa)) {
		cmd = KBD_IMPR;
		pressed = 0;
	//RELACHE EN MODE F0
	} else if(scode[0] == 0xf0) {
		cmd = scantable_commande[scode[1]];
		pressed = 0;
	//COMMANDE EN E0 avec CTRL
	} else if((scode[0] == 0xe0) && ((keyboard.ctrlL == 1) || (keyboard.ctrlR == 1))) {
		cmd = scantable_ctrl_commande_e0[scode[1] & 0x7f];
		if(scode[1] & 0x80)
			pressed = 0;
	//COMMANDE EN E0 avec SHIFT
	} else if((scode[0] == 0xe0) && ((keyboard.shiftL == 1) || (keyboard.shiftR == 1))) {
		cmd = scantable_shift_commande_e0[scode[1] & 0x7f];
		if(scode[1] & 0x80)
			pressed = 0;
	//COMMANDE EN E0
	} else if(scode[0] == 0xe0) {
		cmd = scantable_commande_e0[scode[1] & 0x7f];
		if(scode[1] & 0x80)
			pressed = 0;
	//COMMANDE CLASSIQUE (SHIFT ET CTRL NON TRAITES)
	} else {
		cmd = scantable_commande[scode[0] & 0x7f];
		if(scode[0] & 0x80)
			pressed = 0;
	}

	switch(cmd) {
	case KBD_SHIFTL:
		keyboard.shiftL = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_SHIFTR:
		keyboard.shiftR = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_CTRLL:
		keyboard.ctrlL = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_CTRLR:
		keyboard.ctrlR = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_ALTL:
		keyboard.altL = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_ALTR:
		keyboard.altR = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_WINL:
		keyboard.winL = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_WINR:
		keyboard.winR = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_POPUP:
		keyboard.popup = pressed;
		cmd = KBD_NOP;
		break;
	case KBD_CAPS_LOCK:
		if(pressed == 1) {
			keyboard.capsLock = !keyboard.capsLock;
		}
		cmd = KBD_NOP;
		break;
	default:
		if(pressed == 0)
			cmd = KBD_NOP;
	}

	return cmd;
}

static unsigned int kbd_interpretascii(unsigned char scode[6]) {
	unsigned int maj;
	char ascii;
	
	if((keyboard.ctrlL == 1) || (keyboard.ctrlR == 1))
		return 0;

	if(scode[0] == 0xf0)
		return 0;

	if(scode[0] == 0xe1)
		return 0;

	maj = (keyboard.shiftL == 1) || (keyboard.shiftR == 1);
	maj ^= keyboard.capsLock;

	if(scode[0] != 0xe0) {
		if(maj)
			ascii = scantable_maj_print[scode[0] & 0x7f];
		else
			ascii = scantable_print[scode[0] & 0x7f];
		
		if(ascii != 0) {
			if(scode[0] & 0x80)
				return 0;
			else
				return ascii;
		}

	} else {
		if(maj)
			ascii = scantable_maj_print_e0[scode[1] & 0x7f];
		else
			ascii = scantable_print_e0[scode[1] & 0x7f];

		if(ascii != 0) {
			if(scode[1] & 0x80)
				return 0;
			else
				return ascii;
		}
	}

	return 0;
}

char kbd_getc(void) {
	unsigned char scode[6];
	unsigned int cmd;
	
	while(1) {
		while(kbd_readscancode(scode) < 0);
	
		cmd = kbd_interpretcommand(scode);
	
		switch(cmd) {
		case KBD_ENTER:
		case KBD_PAD_ENTER:
			return '\n';
		case KBD_BACK:
			return '\b';
		case KBD_TAB:
			return '\t';
		case 0:
			cmd = kbd_interpretascii(scode);
			if(cmd)
				return cmd;
		default:
			;
		}
	}
}



