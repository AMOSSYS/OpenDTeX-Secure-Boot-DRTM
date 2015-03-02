#pragma once

#define SCANCODESIZE 6
int kbd_waitforread(void);
int kbd_readscancode(unsigned char scode[SCANCODESIZE]);
char kbd_getc(void);

