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


#include <uvideo/uvideo.h>
#include <uvideo/vga.h>
#include <internal.h>

#pragma pack(push,1)
/**
 * \struct BITMAPFILEHEADER
 * \brief Bitmap file global header
 */
//http://www.fileformat.info/format/bmp/egff.htm
typedef struct {
	unsigned short	bmpMagic;
	unsigned int   bmpSize;
	unsigned int	reserved;
	unsigned int	bmpOffsetData;
} BITMAPFILEHEADER;

#define DIBSIZE_BMPV3HEADER 40
#define DIBSIZE_BMPV4HEADER 108
#define DIBSIZE_BMPV5HEADER 124

/**
 * \struct BITMAPINFORMATIONHEADER
 * \brief Bitmap file information header
 */
typedef struct {
	/* Fields added for Windows 3.x follow this line */
	unsigned int	Size;
	unsigned int	Width;           /* Image width in pixels */
	unsigned int	Height;          /* Image height in pixels */
	unsigned short Planes;          /* Number of color planes */
	unsigned short BitsPerPixel;    /* Number of bits per pixel */
	unsigned int	Compression;     /* Compression methods used */	//Be careful some test are necessary on that field because the V3 header has some variations
	unsigned int	SizeOfBitmap;    /* Size of bitmap in bytes */
	unsigned int	HorzResolution;  /* Horizontal resolution in pixels per meter */
	unsigned int	VertResolution;  /* Vertical resolution in pixels per meter */
	unsigned int	ColorsUsed;      /* Number of colors in the image */
	unsigned int	ColorsImportant; /* Minimum number of important colors */
	
	/* Fields added for Windows 4.x follow this line */
	unsigned int	RedMask;       /* Mask identifying bits of red component */
	unsigned int	GreenMask;     /* Mask identifying bits of green component */
	unsigned int	BlueMask;      /* Mask identifying bits of blue component */
	unsigned int	AlphaMask;     /* Mask identifying bits of alpha component */
	unsigned int	CSType;        /* Color space type */
	unsigned int	RedX;          /* X coordinate of red endpoint */
	unsigned int	RedY;          /* Y coordinate of red endpoint */
	unsigned int	RedZ;          /* Z coordinate of red endpoint */
	unsigned int	GreenX;        /* X coordinate of green endpoint */
	unsigned int	GreenY;        /* Y coordinate of green endpoint */
	unsigned int	GreenZ;        /* Z coordinate of green endpoint */
	unsigned int	BlueX;         /* X coordinate of blue endpoint */
	unsigned int	BlueY;         /* Y coordinate of blue endpoint */
	unsigned int	BlueZ;         /* Z coordinate of blue endpoint */
	unsigned int	GammaRed;      /* Gamma red coordinate scale value */
	unsigned int	GammaGreen;    /* Gamma green coordinate scale value */
	unsigned int	GammaBlue;     /* Gamma blue coordinate scale value */
	
} BITMAPINFORMATIONHEADER;
#pragma pack(pop)

static bool IsBMP(unsigned char * data, unsigned int size) {
   BITMAPFILEHEADER *         header = (BITMAPFILEHEADER *) data;
   BITMAPINFORMATIONHEADER * 	dib = (BITMAPINFORMATIONHEADER *) (data + sizeof(BITMAPFILEHEADER));
   unsigned int					numColor;
   
   if(size < sizeof(BITMAPFILEHEADER)) {
		// Not enough data ...
		return false;
	}

	if(size != header->bmpSize) {
		// Incorrect size ...
		return false;
	}

   switch(header->bmpMagic) {
	case 0x4d42:	// 'BM'
	case 0x4142:	// 'BA'
	case 0x4943:	// 'CI'
	case 0x5043:	// 'CP'
	case 0x4349:	// 'IC'
	case 0x5450:	// 'PT'
		break;
	default:
		// Bad magic value ...
		return false;
	}

	if(header->bmpOffsetData > size) {
		// Bad offset value ...
		return false;
	}

	if(dib->Size + sizeof(BITMAPFILEHEADER) > size) {
		// Not enough data ...
		return false;
	}
	
	if(header->bmpOffsetData < dib->Size) {
		// Incorrect data ...
		return false;
	}
	
	switch(dib->Size) {
	case DIBSIZE_BMPV3HEADER:
	case DIBSIZE_BMPV4HEADER:
	case DIBSIZE_BMPV5HEADER:
		break;
	default:
		// Other DIB format not supported ...
		return false;
	}
	
	if(dib->Compression) {
		// Compression not supported ...
		return false;
	}
	
	if(dib->SizeOfBitmap + header->bmpOffsetData != size) {
		// Bad offset value ...
		return false;
	}
	
	switch(dib->BitsPerPixel) {
	case 4:
	case 8:
		break;
	default:
		// 1, 16, 24, 32 bits colors not supported ...
		return false;
	}

	if((dib->Height * dib->Width * dib->BitsPerPixel + 7) / 8 > dib->SizeOfBitmap) {
		// not enough data according to the picture size
		return false;
	}
	
	if(dib->BitsPerPixel <= 8) {
	   numColor = dib->ColorsUsed;
	   if(numColor == 0) {
		   numColor = 1 << dib->BitsPerPixel;
	   }
   }
	
	// Check palette if present (BitsPerPixel == 1, 4, 8)
	if(dib->BitsPerPixel <= 8) {
	   // NOTE: we don't support BMPv2, so we use 4-byte elements
	   // NOTE: the palette is just after the DIB before BMP data
	   if((header->bmpOffsetData - sizeof(BITMAPFILEHEADER) - dib->Size) / 4 != numColor) {
	      // Bad color data ...
		   return false;
	   }
	}
	
   return true;
}

static uvideo_err_t DisplayBMP(video_console * cons, unsigned char * data, unsigned int size __attribute__((unused))) {
   BITMAPFILEHEADER *         header = (BITMAPFILEHEADER *) data;
   BITMAPINFORMATIONHEADER * 	dib = (BITMAPINFORMATIONHEADER *) (data + sizeof(BITMAPFILEHEADER));
   unsigned int					numColor;
   unsigned char *  				dataPtr;
	unsigned int					width;
	unsigned int					height;
	unsigned int					offW;
	unsigned int					offH;
	unsigned int 					padding;
	unsigned int               sizeline;
	unsigned int               i, j;
	video_conf *               mainconf;
   
   // cons is not NULL
   
   dataPtr = data + header->bmpOffsetData;
	numColor = dib->ColorsUsed;
	width = dib->Width;
	height = dib->Height;
   
   if(width > cons->Width) {
		// picture too big ...
		return UVIDEO_ERR_IMG_TOOBIG;
	}
	
	if(height > cons->Height) {
		// picture too big ...
		return UVIDEO_ERR_IMG_TOOBIG;
	}
	
	if(dib->BitsPerPixel <= 8) {
	   numColor = dib->ColorsUsed;
	   if(numColor == 0) {
		   numColor = 1 << dib->BitsPerPixel;
	   }
   }
	
	// Calulate height and width offsets in order to center the picture
	offW = (cons->Width - width) / 2;
	offH = (cons->Height - height) / 2;
	
	// Calculate scanline padding in the picture
	sizeline = (width * dib->BitsPerPixel + 7) / 8;
	padding = (sizeline % 4 ? 4 - (sizeline % 4) : 0);
	
	// If BitsPerPixel <= 8, the BMP file contains a color palette and color will be indexed on this palette 
	if(dib->BitsPerPixel <= 8) {
	   unsigned char  pel[VGA_PEL_C][3];
	   unsigned char  savepel[VGA_PEL_C][3];
      unsigned int   dac[256];
      unsigned char  (* temp)[4] = (unsigned char (*)[4]) (data + sizeof(BITMAPFILEHEADER) + dib->Size);

	   // If vga is enabled, so we have to load the BMP palette into VGA registers
	   if(vga_isenable() == true) {
	      for(i = 0 ; i < numColor ; i++) {
		      // memory color order is RGB. BMP color order is BGR
		      pel[i][2] = temp[i][0] >> 2;
		      pel[i][1] = temp[i][1] >> 2;
		      pel[i][0] = temp[i][2] >> 2;
		      dac[i] = i;
	      }
	      
	      vga_pel_dump(savepel);
	      vga_pel_conf((const unsigned char (*)[3]) pel);
	      
	   // If vga is NOT enabled, so we have to convert the BMP palette into a array of pixels
	   } else {
	      for(i = 0 ; i < numColor ; i++) {
		      // memory color order is RGB. BMP color order is BGR
		      dac[i] = temp[i][0];
		      dac[i] += temp[i][1] << 8;
		      dac[i] += temp[i][2] << 16;
	      }
	   }
	   
	   mainconf = uvideo_get_mainconf();
	   
	   if(dib->BitsPerPixel == 8) {
	      for(i = 0 ; i < height ; i++) {
	         unsigned int k = height - 1 - i;
	         for(j = 0 ; j < width ; j++) {	            
	            mainconf->Setpixel_wh(dac[dataPtr[k * (sizeline + padding) + j]], cons->PosW + offW, cons->PosH + offH + i);
	         }
	      }
      } else if(dib->BitsPerPixel == 4) {
         for(i = 0 ; i < height ; i++) {
	         unsigned int k = height - 1 - i;
	         for(j = 0 ; j < width ; j++) {
	            unsigned int index = k * (sizeline + padding) + j / 2;
	            unsigned char dacindex = (j & 1 ? dataPtr[index] & 0xf : dataPtr[index] >> 4);
	            mainconf->Setpixel_wh(dac[dacindex], cons->PosW + offW, cons->PosH + offH + i);
	         }
	      }
      } else {
         return UVIDEO_ERR_CRITICAL;
      }
      
      // If VGA is enabled, we just restore 16 first entry of the VGA palette ...
      if(vga_isenable()) {
         //vga_pel_conf(savepel);
         vga_pel_set(savepel, 16, 0);
      }
   } else {
      return UVIDEO_ERR_CRITICAL;
   }
	
	return UVIDEO_NOERR;
}

uvideo_err_t uvideo_internal_display(video_console * cons, unsigned char * data, unsigned int size) {
   if(IsBMP(data, size) == true) {
      return DisplayBMP(cons, data, size);
   }
   
   return UVIDEO_ERR_IMG_UKNOWN;
}


