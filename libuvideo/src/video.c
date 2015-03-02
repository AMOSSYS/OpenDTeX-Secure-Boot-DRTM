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


/**
 * \file video.c
 * \brief Video API
 * \version 1.0
 * \date 10/10/2012
 * \details The goal of this API is to provide an easy way to display texts and pictures on the screen.
 *          The Video API doesn't configure the video card. The video API just write directly in the
 *          video buffer. So, the Video API expects that the video card is already configured (for an
 *          example by the bootloader). The developper must find a way to get video parameters (for
 *          an example, by using the VBE information of the multiboot structure provided by the bootloader).
 *          Then the developper must use the video_init functions to initalize the Video API.
 *
 */

#include <uvideo/uvideo.h>
#include <uc/uc.h>
#include <internal.h>

#define memset uc_memset
#define memcpy uc_memcpy
#define vsnprintf uc_vsnprintf

/**
 * \var videoconf conf
 * \brief Video configuration
 * \details This variable must be initialized with the function video_init.
 */
video_conf mainconf;
static bool videoinit = false;
static video_console * mainconsole = 0;

video_conf * uvideo_get_mainconf(void) {
   return &mainconf;
}

// Graphical pixel stub functions
static void __setpixel32_wh(unsigned int color, unsigned int width, unsigned int height) {	
	*((unsigned int *) &mainconf.Buffer[height * mainconf.LineLength + width * sizeof(unsigned int)]) = color;
}

static void __setpixel16_wh(unsigned int color, unsigned int width, unsigned int height) {
	*((unsigned short *) &mainconf.Buffer[height * mainconf.LineLength + width * sizeof(unsigned short)]) = color;
}

static void __setpixel8_wh(unsigned int color, unsigned int width, unsigned int height) {
	mainconf.Buffer[height * mainconf.LineLength + width] = color;
}

// Base functions
static inline void __cons_putc(video_console * cons, char c) {
	if(mainconf.TextMode) {
		*((unsigned short *) &mainconf.Buffer[(cons->PosH + cons->CurH) * mainconf.LineLength + (cons->PosW + cons->CurW) * sizeof(unsigned short)]) = (cons->Bgcolor << 12) | (cons->Fgcolor << 8) | c;
	} else {
		unsigned int val;
		unsigned char * font; //= uvideo_get_fonts_8x16()[(unsigned char) c];
		unsigned int W, H;
		TAB_FONT_8x16 fonts = uvideo_get_fonts_8x16();
	   
	   font = (*fonts)[(unsigned char) c];
		
		for(H = cons->PosH + cons->CurH ; H < cons->PosH + cons->CurH + mainconf.FontSizeH ; H++, font++) {
			for(W = cons->PosW + cons->CurW, val = *font ; W < cons->PosW + cons->CurW + mainconf.FontSizeW ; W++, val <<= 1) {
				if(val & 0x80) {
				   mainconf.Setpixel_wh(cons->Fgcolor, W, H);
				} else {
					mainconf.Setpixel_wh(cons->Bgcolor, W, H);
			   }
			}
		}
	}
}

static inline void __cons_shiftup(video_console * cons) {
	if(mainconf.TextMode) {
		unsigned int pos, i, offset, lineW;
		
		offset =  mainconf.LineLength;
		lineW = cons->Width * sizeof(unsigned short);
		
		for(i = 1, pos = cons->PosH * mainconf.LineLength + cons->PosW * sizeof(unsigned short) ; i < cons->Height ; i++, pos += offset) {
		   memcpy(&mainconf.Buffer[pos], &mainconf.Buffer[pos + offset], lineW);
		}
		
		for(i = 0 ; i < lineW ; i += sizeof(unsigned short)) {
			*((unsigned short *) &mainconf.Buffer[pos + i]) = (cons->Bgcolor << 12) | (cons->Fgcolor << 8) | ' ';
		}
		
	} else {
	   unsigned int pos, i, offsetFont, lineW;
	   
	   offsetFont = mainconf.FontSizeH * mainconf.LineLength;
	   lineW = cons->MaxCharW * mainconf.PixelSizeBytes;
	   
	   for(i = mainconf.FontSizeH, pos = cons->PosH * mainconf.LineLength + cons->PosW * mainconf.PixelSizeBytes ; i < cons->MaxCharH ; i++, pos += mainconf.LineLength) {
	      memcpy(&mainconf.Buffer[pos], &mainconf.Buffer[pos + offsetFont], lineW);
	   }
	   
	   for(i = 0 ; i < cons->MaxCharW ; i++) {
	      mainconf.Setpixel_wh(cons->Bgcolor, cons->PosW + i, cons->PosH + cons->MaxCharH);
		}
		
		for(i = 1 ; i < mainconf.FontSizeH ; i++, pos += mainconf.LineLength) {
		   memcpy(&mainconf.Buffer[pos + mainconf.LineLength], &mainconf.Buffer[pos], lineW);
		}
	}
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_init(bool text, unsigned int width, unsigned int height, unsigned char * buffer, unsigned int linelength, unsigned int pixelsize, unsigned int RGBTSize, unsigned int RGBTPos)
 * \brief Video API initialization function
 * \details This functions must be called first before using others functions of the Video API.
 *
 * \param text 0 in order to use a graphical mod, text mod otherwise
 * \param width Width of the screen (in pixel for graphical mod or in character for text mod)
 * \param height Height of the screen (in pixel for graphical mod or in character for text mod)
 * \param buffer Address of the video buffer
 * \param color Size in bits of a pixel. This parameter is only used when a graphical mod is chosen
 * \return video error code
 */

uvideo_err_t UVIDEOCALL uvideo_init(bool text, unsigned int width, unsigned int height, unsigned char * buffer, unsigned int linelength, unsigned int pixelsize, unsigned int RGBTSize, unsigned int RGBTPos) {
   memset(&mainconf, 0, sizeof(mainconf));
   
	switch(pixelsize) {
	case 32:
	case 16:
	case 8:
	case 0:
	   break;
	   
	default:
	   return UVIDEO_ERR_BADVIDEOMODE;
	}
	
	if((pixelsize == 0) && ! text) {
	   return UVIDEO_ERR_BADVIDEOMODE;
	}
	
	mainconf.TextMode    = text;
	mainconf.Width       = width;
	mainconf.Height      = height;
	mainconf.Buffer      = buffer;
	mainconf.LineLength  = linelength;
   mainconf.PixelSize   = pixelsize;
   mainconf.RedSize     = (RGBTSize >> 24) & 0xff;
   mainconf.GreenSize   = (RGBTSize >> 16) & 0xff;
   mainconf.BlueSize    = (RGBTSize >> 8) & 0xff;
   mainconf.TransSize   = (RGBTSize >> 0) & 0xff;
      
   mainconf.RedPos      = (RGBTPos >> 24) & 0xff;
   mainconf.GreenPos    = (RGBTPos >> 16) & 0xff;
   mainconf.BluePos     = (RGBTPos >> 8) & 0xff;
   mainconf.TransPos    = (RGBTPos >> 0l) & 0xff;


	switch(pixelsize) {
	case 32:
		mainconf.Setpixel_wh = __setpixel32_wh;
		mainconf.PixelSizeBytes = 4;
		break;
		
	case 16:
		mainconf.Setpixel_wh = __setpixel16_wh;
		mainconf.PixelSizeBytes = 2;
		break;
		
   case 8:
		mainconf.Setpixel_wh = __setpixel8_wh;
		mainconf.PixelSizeBytes = 1;
		break;
	case 0:
		mainconf.LineLength = sizeof(unsigned short) * mainconf.Width;
	}

	if(text) {
		mainconf.FontSizeW = 1;
		mainconf.FontSizeH = 1;
	} else {
		mainconf.FontSizeW = FONT_W;
		mainconf.FontSizeH = FONT_H;
	}
	
	if(mainconf.LineLength == 0) {
	   mainconf.LineLength = mainconf.PixelSizeBytes * mainconf.Width;
	}
	
	videoinit = true;
	
	return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_reset(void)
 * \brief Clear the screen
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_reset(void) {
   if(videoinit != true) {
      return UVIDEO_ERR_NOTINIT;
   }
   
	if(mainconf.TextMode) {
		unsigned int i;

		for(i = 0 ; i < mainconf.Width * mainconf.Height ; i++) {
			((unsigned short *) mainconf.Buffer)[i] = ' ';
		}
	} else {
		memset(mainconf.Buffer, 0, mainconf.LineLength * mainconf.Height);
	}
	
	return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_createcons(video_console * cons, unsigned int posw, unsigned int posh, unsigned int width, unsigned int height, unsigned int fgcolor, unsigned int bgcolor)
 * \brief Console initialization
 * \bug TODO: Overflow and overlapped console are not checked...
 * \details A console need to be defined first before displaying something. A console is an area where picture or text can be printed.
 *
 * \param cons Pointer to a console structure which will be initalized by this function
 * \param posw X-coordinate in the screen of the upper left corner of the console
 * \param posh Y-coordinate in the screen of the upper left corner of the console
 * \param width Width of the console
 * \param height Height of the console
 * \param fgcolor Color value of the foreground
 * \param bgcolor Color value of the background
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_createcons(video_console * cons, unsigned int posw, unsigned int posh, unsigned int width, unsigned int height, unsigned int fgcolor, unsigned int bgcolor) {
	if(videoinit != true) {
      return UVIDEO_ERR_NOTINIT;
   }
   
   if(cons == NULL) {
      return UVIDEO_ERR_BADPARAMETERS;
   }
   
	cons->PosW = posw;
	cons->PosH = posh;
	cons->Width = width;
	cons->Height = height;
	cons->CurW = 0;
	cons->CurH = 0;
	cons->MaxCharW = (width / mainconf.FontSizeW) * mainconf.FontSizeW;
	cons->MaxCharH = (height / mainconf.FontSizeH) * mainconf.FontSizeH;
	cons->Fgcolor = fgcolor;
	cons->Bgcolor = bgcolor;
	
	return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_setfocus(video_console * cons)
 * \brief Load a console as the default console
 *
 * \param cons Pointer to an initialized mainconsole structure
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_setfocus(video_console * cons) {
   if(videoinit != true) {
      return UVIDEO_ERR_NOTINIT;
   }
   
   if(cons == NULL) {
      return UVIDEO_ERR_BADPARAMETERS;
   }
   
	mainconsole = cons;
	
	return UVIDEO_NOERR;
}

/**
 * \fn bool UVIDEOCALL uvideo_istext(void)
 * \brief Get screen mode 
 *
 * \return Return 1 if text mode
 */
bool UVIDEOCALL uvideo_istext(void) {
   return mainconf.TextMode;
}

/**
 * \fn unsigned int UVIDEOCALL uvideo_getwidth(void)
 * \brief Get screen width
 *
 * \return Return screen width
 */
unsigned int UVIDEOCALL uvideo_getwidth(void) {
   return mainconf.Width;
}

/**
 * \fn unsigned int UVIDEOCALL uvideo_getheight(void)
 * \brief Get screen height
 *
 * \return Return screen height
 */
unsigned int UVIDEOCALL uvideo_getheight(void) {
   return mainconf.Height;
}

/**
 * \fn unsigned int UVIDEOCALL uvideo_getbpp(void)
 * \brief Get bits per pixel
 *
 * \return Return number of bits per pixel
 */
unsigned int UVIDEOCALL uvideo_getbpp(void) {
   return mainconf.PixelSize;
}

/**
 * \fn unsigned int UVIDEOCALL uvideo_getlinelen(void)
 * \brief Get screen line size in bytes
 *
 * \return Return screen line size in bytes
 */
unsigned int UVIDEOCALL uvideo_getlinelen(void) {
   return mainconf.LineLength;
}

/**
 * \fn unsigned char * UVIDEOCALL uvideo_getbuffer(void)
 * \brief Get address of the screen buffer
 *
 * \return Return address of the screen buffer
 */
unsigned char * UVIDEOCALL uvideo_getbuffer(void) {
   return mainconf.Buffer;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getredsize(void)
 * \brief Get the size of the mask of red pixel
 *
 * \return Return the size of the mask of red pixel
 */
unsigned char UVIDEOCALL uvideo_getredsize(void) {
   return mainconf.RedSize;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getgreensize(void)
 * \brief Get the size of the mask of green pixel
 *
 * \return Return the size of the mask of green pixel
 */
unsigned char UVIDEOCALL uvideo_getgreensize(void) {
   return mainconf.GreenSize;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getbluesize(void)
 * \brief Get the size of the mask of blue pixel
 *
 * \return Return the size of the mask of blue pixel
 */
unsigned char UVIDEOCALL uvideo_getbluesize(void) {
   return mainconf.BlueSize;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_gettranssize(void)
 * \brief Get the size of the mask of transparent pixel
 *
 * \return Return the size of the mask of transparent pixel
 */
unsigned char UVIDEOCALL uvideo_gettranssize(void) {
   return mainconf.TransSize;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getredpos(void)
 * \brief Get the position of red pixel
 *
 * \return Get the position of red pixel
 */
unsigned char UVIDEOCALL uvideo_getredpos(void) {
   return mainconf.RedPos;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getgreenpos(void)
 * \brief Get the position of green pixel
 *
 * \return Get the position of green pixel
 */
unsigned char UVIDEOCALL uvideo_getgreenpos(void) {
   return mainconf.GreenPos;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_getbluepos(void)
 * \brief Get the position of blue pixel
 *
 * \return Get the position of blue pixel
 */
unsigned char UVIDEOCALL uvideo_getbluepos(void) {
   return mainconf.BluePos;
}

/**
 * \fn unsigned char UVIDEOCALL uvideo_gettranspos(void)
 * \brief Get the position of transparent pixel
 *
 * \return Get the position of transparent pixel
 */
unsigned char UVIDEOCALL uvideo_gettranspos(void) {
   return mainconf.TransPos;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_console_cls(video_console * cons)
 * \brief Clear a specific console with the background color
 *
 * \param cons Pointer to an initialized console structure
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_console_cls(video_console * cons) {
	if(videoinit != true) {
		return UVIDEO_ERR_NOTINIT;
   }
   
   if(cons == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
   }
   //TODO
	cons->CurW = 0;
	cons->CurH = 0;
	
	if(mainconf.TextMode) {
		unsigned int i;
		
		for(i = 0 ; i < cons->Width ; i++) {
			*((unsigned short *) &mainconf.Buffer[cons->PosH * mainconf.LineLength + (cons->PosW + i) * sizeof(unsigned short)]) = (cons->Bgcolor << 12) | (cons->Fgcolor << 8) | ' ';
		}
		
		for(i = 1 ; i < cons->Height ; i++) {
			memcpy(
			   &mainconf.Buffer[(cons->PosH + i) * mainconf.LineLength + cons->PosW * sizeof(unsigned short)],
			   &mainconf.Buffer[cons->PosH  * mainconf.LineLength + cons->PosW * sizeof(unsigned short)],
			   cons->Width * sizeof(unsigned short));
		}
	} else {
	   unsigned int i;
	   
	   for(i = 0 ; i < cons->Width ; i++) {
	      mainconf.Setpixel_wh(cons->Bgcolor, cons->PosW + i, cons->PosH);
		}
	   
	   for(i = 1 ; i < cons->Height ; i++) {
			memcpy(
			   &mainconf.Buffer[(cons->PosH + i) * mainconf.LineLength + cons->PosW * mainconf.PixelSizeBytes],
			   &mainconf.Buffer[cons->PosH  * mainconf.LineLength + cons->PosW * mainconf.PixelSizeBytes],
			   cons->Width * mainconf.PixelSizeBytes);
		}
	}
	
	return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_console_putc(video_console * cons, char c)
 * \brief Append a character in a specific console
 * \details \\n, \\r, \\t characters are interpreted in order to place adequately the cursor console.
 *          The function cons_putvalue can be used instead in order to display the raw representation
 *          of these characters.
 * \bug TODO: backspace character are not interpreted ...
 *
 * \param cons Pointer to an initialized console structure
 * \param c Character do append
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_console_putc(video_console * cons, char c) {
	if(videoinit != true) {
		return UVIDEO_ERR_NOTINIT;
   }
   
   if(cons == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
   }
	
	if(c == '\n') {
		cons->CurW = 0;
		cons->CurH += mainconf.FontSizeH;
	} else if(c == '\b') {
	   if(cons->CurW >= mainconf.FontSizeW) {
	      cons->CurW -= mainconf.FontSizeW;
	   }
	   __cons_putc(cons, ' ');
	} else if(c == '\r') {
		cons->CurW = 0;
	} else if(c == '\t') {
		unsigned int i;
		
		for(i = 0 ; i < 3 ; i++) {
			if(cons->CurW >= cons->Width - mainconf.FontSizeW)
				break;
		
			__cons_putc(cons, ' ');
			cons->CurW += mainconf.FontSizeW;
		}
	} else {
		__cons_putc(cons, c);
		cons->CurW += mainconf.FontSizeW;
	}
	
	if(cons->CurW >= cons->MaxCharW) {
		cons->CurW = 0;
		cons->CurH += mainconf.FontSizeH;
	}
	
	if(cons->CurH >= cons->MaxCharH) {
		__cons_shiftup(cons);
		cons->CurH -= mainconf.FontSizeH;
	}
	
	return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_console_putval(video_console * cons, char c)
 * \brief Append a character in a specific console
 * \details \\n, \\r, \\t characters are not interpreted and theirs raw representations are displayed.
 *          The function cons_putc can be used instead in order to interpret these characters.
 *
 * \param cons Pointer to an initialized console structure
 * \param c Character do append
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_console_putval(video_console * cons, char c) {
	if(videoinit != true) {
		return UVIDEO_ERR_NOTINIT;
   }
   
   if(cons == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
   }
	
   __cons_putc(cons, c);
   cons->CurW += mainconf.FontSizeW;
	
	if(cons->CurW >= cons->MaxCharW) {
		cons->CurW = 0;
		cons->CurH += mainconf.FontSizeH;
	}
	
	if(cons->CurH >= cons->MaxCharH) {
		__cons_shiftup(cons);
		cons->CurH -= mainconf.FontSizeH;
	}
	
	return UVIDEO_NOERR;
}

/**
 * \fn int UVIDEOCALL uvideo_console_printf(video_console * cons, const char * format, ...)
 * \brief Display a printf-style string in a specific console
 *
 * \param cons Pointer to an initialized console structure
 * \param format Printf format string
 * \return Number of displayed characters (the \\0 character is not counted)
 */
int UVIDEOCALL uvideo_console_printf(video_console * cons, const char * format, ...) {
	va_list args;
	int res;
	char buffer[1024];
	char * ptr;
	
	if(videoinit != true) {
		return -1;
   }
	
	if(cons == NULL) {
		return -1;
	}
	
	va_start(args, format);
	res = vsnprintf(buffer, sizeof(buffer), format, args);
	if(res == sizeof(buffer)) {
		buffer[sizeof(buffer) - 1] = 0;
   } else {
		buffer[res] = 0;
	}
		
	va_end(args);
				
	for(ptr = buffer, res = 0 ; *ptr ; ptr++, res++) {
		uvideo_console_putc(cons, *ptr);
	}
	
	return res;
}

/**
 * \fn int UVIDEOCALL uvideo_console_vprintf(video_console * cons, const char * format, va_list ap)
 * \brief Display a printf-style string in a specific console
 *
 * \param cons Pointer to an initialized console structure
 * \param format Printf format string
 * \param ap va_list of printf format values
 * \return Number of displayed characters (the \\0 character is not counted)
 */
int UVIDEOCALL uvideo_console_vprintf(video_console * cons, const char * format, va_list ap) {
	int res;
	char buffer[1024];
	char * ptr;
	
	if(videoinit != true) {
		return -1;
   }
	
	if(cons == NULL) {
		return -1;
	}
	
	res = vsnprintf(buffer, sizeof(buffer), format, ap);
	if(res == sizeof(buffer)) {
		buffer[sizeof(buffer) - 1] = 0;
   } else {
		buffer[res] = 0;
	}
				
	for(ptr = buffer, res = 0 ; *ptr ; ptr++, res++) {
		uvideo_console_putc(cons, *ptr);
	}
	
	return res;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_console_display(video_console * cons, unsigned char * picture, unsigned int size)
 * \brief Display a picture in a specific console
 * \bug TODO: only BMP 8bits not compressed files are supported
 *
 * \param cons Pointer to an initialized console structure
 * \param picture Address of the picture in memory
 * \param size Size of picture in memory
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_console_display(video_console * cons, unsigned char * picture, unsigned int size) {
   if(videoinit != true) {
		return UVIDEO_ERR_NOTINIT;
   }
	
	if(cons == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
	}
		
	if(mainconf.TextMode) {
	   return UVIDEO_ERR_TEXTMODE;
   }
   
   if(picture == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
	}
	
	if(size == 0) {
		return UVIDEO_ERR_BADPARAMETERS;
	}
	
	return uvideo_internal_display(cons, picture, size);
}

/**
 * \fn uvideo_err_t UVIDEOCALL  uvideo_console_shiftup(video_console * cons)
 * \brief Shift up a console
 *
 * \param cons Pointer to an initialized console structure
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_console_shiftup(video_console * cons) {
   if(videoinit != true) {
		return UVIDEO_ERR_NOTINIT;
   }
	
	if(cons == NULL) {
		return UVIDEO_ERR_BADPARAMETERS;
	}
	
	__cons_shiftup(cons);

   return UVIDEO_NOERR;
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_cls(void)
 * \brief Clear the default console with the background color
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_cls(void) {
	return uvideo_console_cls(mainconsole);
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_putc(char c)
 * \brief Append a character in the default console
 * \details \\n, \\r, \\t characters are interpreted in order to place adequately the cursor console.
 *          The function video_putvalue can be used instead in order to display the raw representation
 *          of these characters.
 * \bug TODO: backspace character are not interpreted ...
 *
 * \param c Character do append
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_putc(char c) {
	return uvideo_console_putc(mainconsole, c);
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_putval(char c)
 * \brief Append a character in the default console
 * \details \\n, \\r, \\t characters are not interpreted and theirs raw representations are displayed.
 *          The function video_putc can be used instead in order to interpreted these characters.
 *
 * \param c Character do append
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_putval(char c) {
	return uvideo_console_putval(mainconsole, c);
}

/**
 * \fn int UVIDEOCALL uvideo_printf(const char * format, ...)
 * \brief Display a printf-style string in the default console
 *
 * \param format Printf format string
 * \return Number of displayed characters (the \\0 character is not counted)
 */
int UVIDEOCALL uvideo_printf(const char * format, ...) {
	va_list args;
	int res;
	
	va_start(args, format);
	res = uvideo_console_vprintf(mainconsole, format, args);		
	va_end(args);
	
	return res;
}

/**
 * \fn int UVIDEOCALL uvideo_vprintf(const char * format, va_list ap)
 * \brief Display a printf-style string in the default console
 *
 * \param format Printf format string
 * \param ap va_list of printf format values
 * \return Number of displayed characters (the \\0 character is not counted)
 */
int UVIDEOCALL uvideo_vprintf(const char * format, va_list ap) {
	return uvideo_console_vprintf(mainconsole, format, ap);		
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_display(unsigned char * picture, unsigned int size)
 * \brief Display a picture in the default console
 * \bug TODO: only BMP 8bits not compressed files are supported
 *
 * \param picture Address of the picture in memory
 * \param size Size of picture in memory
 */
uvideo_err_t UVIDEOCALL uvideo_display(unsigned char * picture, unsigned int size) {
   return uvideo_console_display(mainconsole, picture, size);
}

/**
 * \fn uvideo_err_t UVIDEOCALL uvideo_shiftup(void)
 * \brief Shift up the default console
 *
 * \return video error code
 */
uvideo_err_t UVIDEOCALL uvideo_shiftup(void) {
   return uvideo_console_shiftup(mainconsole);
}

