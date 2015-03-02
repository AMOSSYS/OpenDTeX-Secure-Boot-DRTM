#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#if ARCHI == 32
#define UVIDEOCALL __attribute__((stdcall, regparm(2)))
#define __type_mem unsigned int
#elif ARCHI == 64
#define UVIDEOCALL __attribute__((regparm(4)))
#define __type_mem unsigned long long
#else
#error ARCHI macro not defined
#endif

#define __UVIDEO_CHECK__

#define FONT_W 8
#define FONT_H 16

#define RGBTPACK(r, g, b, t)  ((((unsigned char) r) << 24) + (((unsigned char) g) << 16) + (((unsigned char) b) << 8) + ((unsigned char) t))

/**
 * \typedef uvideo_err_t
 * \brief typedef for error code.
 *
 * Typedef for error code.
 */
typedef enum {
   UVIDEO_NOERR = 0,
   UVIDEO_ERR_NOTINIT = 0xf0000000,
   UVIDEO_ERR_BADVIDEOMODE,
   UVIDEO_ERR_BADPARAMETERS,
   UVIDEO_ERR_TEXTMODE,
   UVIDEO_ERR_IMG_UKNOWN,
   UVIDEO_ERR_IMG_TOOBIG,
   UVIDEO_ERR_CRITICAL,
   UVIDEO_ERR_NOTIMPLEMENTED
} uvideo_err_t;


/**
 * \typedef fn__setpixel_wh
 * \brief Function type for pixel display function.
 *
 * Pixel are displaying according to X and Y video screen coordinates.
 */
typedef void (* fn__setpixel_wh)(unsigned int color, unsigned int width, unsigned int height);

/**
 * \struct videoconf
 * \brief Structure defining video configuration (video card mod)
 */
typedef struct {
	bool      		   TextMode;   /*! Text mod flag (1 if text mod, 0 for graphical mod */
	unsigned int 		Width;            // In pixels
	unsigned int 		Height;           // In pixels
	unsigned char *	Buffer;
	
	fn__setpixel_wh	Setpixel_wh;
	unsigned int		FontSizeW;
	unsigned int		FontSizeH;
	unsigned int      PixelSize;        // In bits
	unsigned int      PixelSizeBytes;   // In bytes
	unsigned int      LineLength;       // In bytes
	
	unsigned char     RedSize;          // In bits
   unsigned char     GreenSize;        // In bits
   unsigned char     BlueSize;         // In bits
   unsigned char     TransSize;        // In bits
   unsigned char     RedPos;           // In bits
   unsigned char     GreenPos;         // In bits
   unsigned char     BluePos;          // In bits
   unsigned char     TransPos;         // In bits
} video_conf;

/**
 * \struct videocons
 * \brief Structure defining console configuration
 */
typedef struct {
	unsigned int      Width;
	unsigned int      Height;
	unsigned int      PosW;
	unsigned int      PosH;
	unsigned int      CurW;
	unsigned int      CurH;
	unsigned int      MaxCharW;
	unsigned int      MaxCharH;
	unsigned int      Fgcolor;
	unsigned int      Bgcolor;
} video_console;

uvideo_err_t    UVIDEOCALL  uvideo_init(bool text, unsigned int width, unsigned int height, unsigned char * buffer, unsigned int linelength, unsigned int pixelsize, unsigned int RGBTSize, unsigned int RGBTPos);
uvideo_err_t    UVIDEOCALL  uvideo_reset(void);
uvideo_err_t    UVIDEOCALL  uvideo_createcons(video_console * cons, unsigned int posw, unsigned int posh, unsigned int width, unsigned int height, unsigned int fgcolor, unsigned int bgcolor);
uvideo_err_t    UVIDEOCALL  uvideo_setfocus(video_console * cons);
bool            UVIDEOCALL  uvideo_istext(void);
unsigned int    UVIDEOCALL  uvideo_getwidth(void);
unsigned int    UVIDEOCALL  uvideo_getheight(void);
unsigned int    UVIDEOCALL  uvideo_getbpp(void);
unsigned int    UVIDEOCALL  uvideo_getlinelen(void);
unsigned char * UVIDEOCALL  uvideo_getbuffer(void);
unsigned char   UVIDEOCALL  uvideo_getredsize(void);
unsigned char   UVIDEOCALL  uvideo_getgreensize(void);
unsigned char   UVIDEOCALL  uvideo_getbluesize(void);
unsigned char   UVIDEOCALL  uvideo_gettranssize(void);
unsigned char   UVIDEOCALL  uvideo_getredpos(void);
unsigned char   UVIDEOCALL  uvideo_getgreenpos(void);
unsigned char   UVIDEOCALL  uvideo_getbluepos(void);
unsigned char   UVIDEOCALL  uvideo_gettranspos(void);

uvideo_err_t    UVIDEOCALL  uvideo_console_putc(video_console * cons, char c);
uvideo_err_t    UVIDEOCALL  uvideo_console_putval(video_console * cons, char c);
uvideo_err_t    UVIDEOCALL  uvideo_console_cls(video_console * cons);
int             UVIDEOCALL  uvideo_console_printf(video_console * cons, const char * format, ...);
int             UVIDEOCALL  uvideo_console_vprintf(video_console * cons, const char * format, va_list ap);
uvideo_err_t    UVIDEOCALL  uvideo_console_display(video_console * cons, unsigned char * picture, unsigned int size);
uvideo_err_t    UVIDEOCALL  uvideo_console_shiftup(video_console * cons);

uvideo_err_t    UVIDEOCALL  uvideo_putc(char c);
uvideo_err_t    UVIDEOCALL  uvideo_putval(char c);
uvideo_err_t    UVIDEOCALL  uvideo_cls(void);
int             UVIDEOCALL  uvideo_printf(const char * format, ...);
int             UVIDEOCALL  uvideo_vprintf(const char * format, va_list ap);
uvideo_err_t    UVIDEOCALL  uvideo_display(unsigned char * picture, unsigned int size);
uvideo_err_t    UVIDEOCALL  uvideo_shiftup(void);


#undef __UVIDEO_CHECK__

