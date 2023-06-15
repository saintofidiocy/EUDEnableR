#ifndef EUDR_H
#define EUDR_H

// generic types
typedef unsigned char  u8;
typedef   signed char  s8;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned int   u32;
typedef   signed int   s32;


// Section Names
#define SECT_TYPE 0x45505954
#define SECT_VER  0x20524556
#define SECT_VCOD 0x444F4356
#define SECT_OWNR 0x524E574F
#define SECT_ERA  0x20415245
#define SECT_DIM  0x204D4944
#define SECT_SIDE 0x45444953
#define SECT_MTXM 0x4D58544D
#define SECT_PUNI 0x494E5550
#define SECT_UPGR 0x52475055
#define SECT_PTEC 0x43455450
#define SECT_UNIT 0x54494E55
#define SECT_THG2 0x32474854
#define SECT_MASK 0x4B53414D
#define SECT_STR  0x20525453
#define SECT_UPRP 0x50525055
#define SECT_MRGN 0x4E47524D
#define SECT_TRIG 0x47495254
#define SECT_MBRF 0x4652424D
#define SECT_SPRP 0x50525053
#define SECT_FORC 0x43524F46
#define SECT_UNIS 0x53494E55
#define SECT_UPGS 0x53475055
#define SECT_TECS 0x53434554
#define SECT_COLR 0x524C4F43
#define SECT_PUPx 0x78505550
#define SECT_PTEx 0x78455450
#define SECT_UNIx 0x78494E55
#define SECT_UPGx 0x78475055
#define SECT_TECx 0x78434554

// SC:R Section Names
#define SECT_CRGB 0x42475243
#define SECT_STRx 0x78525453


// CRGB Modes
#define CRGB_RANDOM  0
#define CRGB_USER    1
#define CRGB_RGB     2
#define CRGB_COLR    3


// Condition/Action Constants
#define EUD_MASK_FLAG       0x4353
#define FLAG_UNIT_TYPE_USED 0x10

#define MODIFIER_AT_LEAST       0
#define MODIFIER_AT_MOST        1
#define MODIFIER_EXACTLY        10
#define MODIFIER_SET_TO         7
#define MODIFIER_ADD            8
#define MODIFIER_SUBTRACT       9

#define PLAYER_NONE             12
#define PLAYER_CURRENT_PLAYER   13
#define PLAYER_FOES             14
#define PLAYER_ALLIES           15
#define PLAYER_NEUTRAL_PLAYERS  16
#define PLAYER_ALL_PLAYERS      17
#define PLAYER_FORCE_1          18
#define PLAYER_FORCE_2          19
#define PLAYER_FORCE_3          20
#define PLAYER_FORCE_4          21
#define PLAYER_UNUSED_1         22
#define PLAYER_UNUSED_2         23
#define PLAYER_UNUSED_3         24
#define PLAYER_UNUSED_4         25
#define PLAYER_NON_ALLIED_VIC   26

#define UNIT_NONE               228
#define UNIT_ANY_UNIT           229
#define UNIT_MEN                230
#define UNIT_BUILDINGS          231
#define UNIT_FACTORIES          232


// Common ASM Opcodes
#define OP_JMP     0xE9
#define OP_CALL    0xE8
#define OP_NOP     0x90


// I have no idea what this actually is
typedef struct {
  u32 end;
  u8* data;
  u8  name[4];
  u32 size;
} chk_call_data;

// Callback to load and/or process a section
typedef __stdcall u32 (*chk_callback)(chk_call_data* data, u32 size, u32 idk);




// Defines a section to load, and the function to load it
typedef struct {
  u32 name;
  u32 function; // chk_callback
  u32 required;
} chk_sect;

// Set of map sections to load together
typedef struct {
  chk_sect* chunks;
  u32 count;
} chk_sets;

// Macro to calculate "count" member of chk_sets
#define setcount(x) sizeof(x)/sizeof(chk_sect)

// Defines what map sections to load for specific gametypes depending on "VER " value
typedef struct {
  u32 version;
  chk_sets mapinfo;
  chk_sets briefing;
  chk_sets melee;
  chk_sets ums;
  u32 isbw;
} chk_ver;


// CRGB struct
typedef struct {
  u8 rgb[8][3];
  u8 mode[8];
} sCRGB;


// TRIG structs
typedef struct {
/*0x00*/ u32 location;
/*0x04*/ u32 player;
/*0x08*/ u32 number;
/*0x0C*/ u16 unit;
/*0x0E*/ u8  modifier;
/*0x0F*/ u8  condition;
/*0x10*/ u8  type;
/*0x11*/ u8  flags;
/*0x12*/ u16 maskFlag;
} condition;

typedef struct {
/*0x00*/ u32 location;
/*0x04*/ u32 string;
/*0x08*/ u32 wav;
/*0x0C*/ u32 time;
/*0x10*/ u32 player;
/*0x14*/ u32 number;
/*0x18*/ u16 type;
/*0x1A*/ u8  action;
/*0x1B*/ u8  modifier;
/*0x1C*/ u8  flags;
/*0x1D*/ u8  padding;
/*0x1E*/ u16 maskFlag;
} action;

typedef __fastcall u32 (*PlayerGroupIterator_Callback)(u32 player, u16 unit, u32 number);
typedef __stdcall u32 (*PlayerGroupIterator)(u32 unit, u32 number, PlayerGroupIterator_Callback cb);


// No padding between struct members
#include <pshpack1.h>

// GRP Frame Header
typedef struct {
  unsigned char x;
  unsigned char y;
  unsigned char w;
  unsigned char h;
  unsigned int offset;
} grpFrame;

// GRP File Header
typedef struct {
  u16 frameCount;
  u16 w;
  u16 h;
  grpFrame frame[1];
} grpHead;

// CImage struct Modified from bwapi
typedef struct{
/*0x00*/ u32      prev;
/*0x04*/ u32      next;
/*0x08*/ u16      imageID;
/*0x0A*/ u8       paletteType;
/*0x0B*/ u8       direction;
/*0x0C*/ u16      flags;
/*0x0E*/ s8       horizontalOffset;
/*0x0F*/ s8       verticalOffset;
/*0x10*/ u16      iscriptHeader;
/*0x12*/ u16      iscriptOffset;
/*0x14*/ u16      iscriptReturn;
/*0x16*/ u8       anim;
/*0x17*/ u8       sleep;
/*0x18*/ u16      frameSet;
/*0x1A*/ u16      frameIndex;
/*0x1C*/ s16      mapPosition[2];
/*0x20*/ s16      screenPosition[2];
/*0x24*/ s16      grpBounds[4];
/*0x2C*/ grpHead* GRPFile;
/*0x30*/ u32      coloringData;
/*0x34*/ u32      renderFunction;
/*0x38*/ u32      updateFunction;
/*0x3C*/ u32      spriteOwner;
} CImage;


// Restore padding
#include <poppack.h>

// Storm Functions
typedef void* (WINAPI *SMemAlloc_type)(int size, char* filename, int line, int value); // 401
typedef u32 (WINAPI *SMemFree_type)(void* ptr, char* filename, int line, int idk);   // 403

typedef u32 (WINAPI *SFileCloseFile_type)(HANDLE hFile); // 253
typedef u32 (WINAPI *SFileOpenFileEx_type)(HANDLE hArchive, char *filename, int scope, HANDLE * hFile); // 268
typedef u32 (WINAPI *SFileReadFile_type)(HANDLE hFile, void *buffer, int toRead, int *read, int overlapped); // 269
typedef u32 (WINAPI *SFileSetFilePointer_type)(HANDLE hFile, int filePos, int *filePosHigh, int method); // 271
typedef u32 (WINAPI *SBmpLoadImage_type)(char* filename, u8* paletteentries, u8* bitmapbits, u32 buffersize, int *width, int *height, int *bitdepth); // 323

extern SMemAlloc_type SMemAlloc;
extern SMemFree_type SMemFree;
extern SFileCloseFile_type SFileCloseFile;
extern SFileOpenFileEx_type SFileOpenFileEx;
extern SFileReadFile_type SFileReadFile;
extern SFileSetFilePointer_type SFileSetFilePointer;
extern SBmpLoadImage_type SBmpLoadImage;

#ifdef __cplusplus
extern "C" { // so that it's linkable from bwl.c
#endif

// Called by the plugin
u32 patch();

#ifdef __cplusplus
};
#endif


#endif
