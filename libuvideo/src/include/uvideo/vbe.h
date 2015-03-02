#pragma once

#pragma pack(push,1)

// 'VESA' signature
#define VBESIGNATURE 0x41534556

typedef enum {
   VBE1_2 = 0x0102,
   VBE2_0 = 0x0200,
} VBE_VERSION;

typedef enum {
   VBE_TXT = 0,
   VBE_CGA,
   VBE_HERCULES,
   VBE_PLANAR,
   VBE_PACKED,
   VBE_NONCHAIN4,
   VBE_DIRECT,
   VBE_YUV
} VBE_MEMORYMODEL;

typedef unsigned int vbe_farptr_t;
typedef unsigned int vbe_physptr_t;
typedef unsigned int vbe_status_t;

typedef struct {
   unsigned int   VbeSignature;
   unsigned short VbeVersion;
   vbe_farptr_t   OemStringPtr;
   unsigned int   Capabilities;
   vbe_farptr_t   VideoModePtr;
   unsigned short TotalMemory;

   unsigned short OemSoftwareRev;
   vbe_farptr_t   OemVendorNamePtr;
   vbe_farptr_t   OemProductNamePtr;
   vbe_farptr_t   OemProductRevPtr;

   unsigned char  Reserved[222];
   unsigned char  OemData[256];
} VbeInfoBlock;

typedef struct {
   // Mandatory information for all VBE revisions
   unsigned short       ModeAttributes;
   unsigned char        WinAAttributes;
   unsigned char        WinBAttributes;
   unsigned short       WinGranularity;
   unsigned short       WinSize;
   unsigned short       WinASegment;
   unsigned short       WinBSegment;
   vbe_farptr_t         WinFuncPtr;
   unsigned short       BytesPerScanLine;

   // Mandatory information for VBE 1.2 and above
   unsigned short       XResolution;
   unsigned short       YResolution;
   unsigned char        XCharSize;
   unsigned char        YCharSize;
   unsigned char        NumberOfPlanes;
   unsigned char        BitsPerPixel;
   unsigned char        NumberOfBanks;
   unsigned char        MemoryModel;
   unsigned char        BankSize;
   unsigned char        NumberOfImagePages;
   unsigned char        Reserved;

   // Direct Color fields (required for direct/6 and YUV/7 memory models)
   unsigned char        RedMaskSize;
   unsigned char        RedFieldPosition;
   unsigned char        GreenMaskSize;
   unsigned char        GreenFieldPosition;
   unsigned char        BlueMaskSize;
   unsigned char        BlueFieldPosition;
   unsigned char        RsvdMaskSize;
   unsigned char        RsvdFieldPosition;
   unsigned char        DirectColorModeInfo;

   // Mandatory information for VBE 2.0 and above.
   vbe_physptr_t        PhysBaseAddr;
   unsigned int         OffScreenMemOffset;
   unsigned short       OffScreenMemSize;
   
   unsigned char        Reserved2[206];
} VbeModeInfoBlock;

#pragma pack(pop)

