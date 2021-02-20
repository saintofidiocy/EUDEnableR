#include <windows.h>
#include "eudr.h"
#include "ogg2wav.h"
#include "version.h"

asm(".intel_syntax noprefix\n");


// custom section reading functions
__stdcall u32 CHK_CRGB(chk_call_data* thing, u32 size, u32 idk);
__stdcall u32 CHK_COLR(chk_call_data* thing, u32 size, u32 idk);
__stdcall u32 CHK_COLR_opt(chk_call_data* thing, u32 size, u32 idk);
__stdcall u32 CHK_STRx(chk_call_data* thing, u32 size, u32 idk);


// SC:R keeps STR data at a fixed address for EUDs
#define EUD_STR_PTR        0x191943C8
#define EUD_STR_MAX_SIZE   64<<20 // 64 MB reserved


/* ---- Map Version Tables ---- */

// These tables define what sections will be loaded and required for a particular game type and map version.

// For versions requiring "COLR", put COLR after CRGB and use CHK_COLR function
// Otherwise, put COLR before CRGB and use CHK_COLR_opt function

// TODO: Is STRx loaded regardless of map version?
// TODO: What happens if neither STRx nor STR are present?

// Used for map list/lobby
chk_sect general[] = {
{SECT_VER , 0x004CB500, 1},
{SECT_DIM , 0x004CB040, 1},
{SECT_ERA , 0x004CB3A0, 1},
{SECT_OWNR, 0x004CB420, 1},
{SECT_SIDE, 0x004CB490, 1},
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_SPRP, 0x004CAF40, 1},
{SECT_FORC, 0x004CAEE0, 1},
{SECT_VCOD, 0x004CBC40, 1}};


// Non-Remaster Briefings
chk_sect briefing[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MBRF, 0x004CC1F0, 1}};


// SC 1.00 or 1.04 Melee
chk_sect melee_sc[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_COLR, (u32)(&CHK_COLR_opt), 0},
{SECT_CRGB, (u32)(&CHK_CRGB), 0}}; // load COLR if CRGB is present


// SC 1.00 UMS
chk_sect ums_van[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_MASK, 0x004CAF90, 1},
{SECT_UNIS, 0x004CAB10, 1},
{SECT_UPGS, 0x004CA8D0, 1},
{SECT_TECS, 0x004CA6D0, 1},
{SECT_PUNI, 0x004CA600, 1},
{SECT_UPGR, 0x004CB940, 1},
{SECT_PTEC, 0x004CB670, 1},
{SECT_UNIx, 0x004CACD0, 0},
{SECT_UPGx, 0x004CA9F0, 0},
{SECT_TECx, 0x004CA7D0, 0},
{SECT_PUPx, 0x004CBAC0, 0},
{SECT_PTEx, 0x004CB7D0, 0},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_UPRP, 0x004CB250, 1},
{SECT_MRGN, 0x004CB2F0, 1},
{SECT_TRIG, 0x004CBFA0, 1},
{SECT_COLR, (u32)(&CHK_COLR_opt), 0},
{SECT_CRGB, (u32)(&CHK_CRGB), 0}}; // load COLR if CRGB is present


// SC 1.04 UMS
chk_sect ums_hyb[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_MASK, 0x004CAF90, 1},
{SECT_UNIS, 0x004CAB10, 1},
{SECT_UPGS, 0x004CA8D0, 1},
{SECT_TECS, 0x004CA6D0, 1},
{SECT_PUNI, 0x004CA600, 1},
{SECT_UPGR, 0x004CB940, 1},
{SECT_PTEC, 0x004CB670, 1},
{SECT_UNIx, 0x004CACD0, 0},
{SECT_UPGx, 0x004CA9F0, 0},
{SECT_TECx, 0x004CA7D0, 0},
{SECT_PUPx, 0x004CBAC0, 0},
{SECT_PTEx, 0x004CB7D0, 0},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_UPRP, 0x004CB250, 1},
{SECT_MRGN, 0x004CB2A0, 1},
{SECT_TRIG, 0x004CBFA0, 1},
{SECT_COLR, (u32)(&CHK_COLR_opt), 0},
{SECT_CRGB, (u32)(&CHK_CRGB), 0}}; // load COLR if CRGB is present


// BW Melee
chk_sect melee_bw[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_CRGB, (u32)(&CHK_CRGB), 0},
{SECT_COLR, (u32)(&CHK_COLR), 1}}; // always load COLR


// BW UMS
chk_sect ums_bw[] = {
{SECT_STR , (u32)(&CHK_STRx), 1},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_MASK, 0x004CAF90, 1},
{SECT_UNIx, 0x004CACD0, 1},
{SECT_UPGx, 0x004CA9F0, 1},
{SECT_TECx, 0x004CA7D0, 1},
{SECT_PUNI, 0x004CA600, 1},
{SECT_PUPx, 0x004CBAC0, 1},
{SECT_PTEx, 0x004CB7D0, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_UPRP, 0x004CB250, 1},
{SECT_MRGN, 0x004CB2A0, 1},
{SECT_TRIG, 0x004CBFA0, 1},
{SECT_CRGB, (u32)(&CHK_CRGB), 0},
{SECT_COLR, (u32)(&CHK_COLR), 1}}; // always load COLR


// Remaster Briefings
chk_sect briefing_r[] = {
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_MBRF, 0x004CC1F0, 1}};


// SC:R SCM Melee
chk_sect melee_sc_r[] = {
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_COLR, (u32)(&CHK_COLR_opt), 0},
{SECT_CRGB, (u32)(&CHK_CRGB), 0}}; // load COLR if CRGB is present


// SC:R SCM UMS
chk_sect ums_hyb_r[] = {
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_MASK, 0x004CAF90, 1},
{SECT_UNIS, 0x004CAB10, 1},
{SECT_UPGS, 0x004CA8D0, 1},
{SECT_TECS, 0x004CA6D0, 1},
{SECT_PUNI, 0x004CA600, 1},
{SECT_UPGR, 0x004CB940, 1},
{SECT_PTEC, 0x004CB670, 1},
{SECT_UNIx, 0x004CACD0, 0},
{SECT_UPGx, 0x004CA9F0, 0},
{SECT_TECx, 0x004CA7D0, 0},
{SECT_PUPx, 0x004CBAC0, 0},
{SECT_PTEx, 0x004CB7D0, 0},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_UPRP, 0x004CB250, 1},
{SECT_MRGN, 0x004CB2A0, 1},
{SECT_TRIG, 0x004CBFA0, 1},
{SECT_COLR, (u32)(&CHK_COLR_opt), 0},
{SECT_CRGB, (u32)(&CHK_CRGB), 0}}; // load COLR if CRGB is present


// SC:R BW Melee
chk_sect melee_bw_r[] = {
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_CRGB, (u32)(&CHK_CRGB), 0},
{SECT_COLR, (u32)(&CHK_COLR), 1}}; // always load COLR


// SC:R BW UMS
chk_sect ums_bw_r[] = {
{SECT_STR , (u32)(&CHK_STRx), 0},
{SECT_STRx, (u32)(&CHK_STRx), 0},
{SECT_MTXM, 0x004CD0B0, 1},
{SECT_THG2, 0x004CD600, 1},
{SECT_MASK, 0x004CAF90, 1},
{SECT_UNIx, 0x004CACD0, 1},
{SECT_UPGx, 0x004CA9F0, 1},
{SECT_TECx, 0x004CA7D0, 1},
{SECT_PUNI, 0x004CA600, 1},
{SECT_PUPx, 0x004CBAC0, 1},
{SECT_PTEx, 0x004CB7D0, 1},
{SECT_UNIT, 0x004CD7A0, 1},
{SECT_UPRP, 0x004CB250, 1},
{SECT_MRGN, 0x004CB2A0, 1},
{SECT_TRIG, 0x004CBFA0, 1},
{SECT_CRGB, (u32)(&CHK_CRGB), 0},
{SECT_COLR, (u32)(&CHK_COLR), 1}}; // always load COLR


// "VER " version definitions
#define MAP_VER_COUNT 5
chk_ver mapchunks[MAP_VER_COUNT] = {
//+00   +04       +08                 +0c          +10                  +14          +18                    +1c         +20                   +24
  {59,  {general, setcount(general)}, {briefing,   setcount(briefing)},   {melee_sc,   setcount(melee_sc)},   {ums_van,   setcount(ums_van)},   0},
  {63,  {general, setcount(general)}, {briefing,   setcount(briefing)},   {melee_sc,   setcount(melee_sc)},   {ums_hyb,   setcount(ums_hyb)},   0},
  {64,  {general, setcount(general)}, {briefing_r, setcount(briefing_r)}, {melee_sc_r, setcount(melee_sc_r)}, {ums_hyb_r, setcount(ums_hyb_r)}, 0},
  {205, {general, setcount(general)}, {briefing,   setcount(briefing)},   {melee_bw,   setcount(melee_bw)},   {ums_bw,    setcount(ums_bw) },   1},
  {206, {general, setcount(general)}, {briefing_r, setcount(briefing_r)}, {melee_bw_r, setcount(melee_bw_r)}, {ums_bw_r,  setcount(ums_bw_r)},  1}
};



/* ---- Patch Data ---- */

// Pointer replacements to read custom map versions table
struct {
  u32 address;
  s32 arrayOffset;
} patch_mapchunks[] = {
  {0x004BF57D +3, 0x1C},
  {0x004BF58C +3, 0x14},
  {0x004BF658 +2, 0x08},
  {0x004BF65E +2, 0x04},
//  {0x004CC0C0 +1, -1},// -1 to indicate end of table -- changed to the function call for cleardata() as a convenient hook
  {0x004CC0CB +2, 0x00},
  {0x004CC0DE +3, 0x24},
  {0x004CC2DC +2, 0x08},
  {0x004CC2E2 +2, 0x04},
  {0x004CC31C +3, 0x1C},
  {0x004CC32B +3, 0x14},
  {0x004CC92C +3, 0x1C},
  {0x004CC93B +3, 0x14},
  {0x004CCA88 +2, 0x10},
  {0x004CCA8E +2, 0x0C},
  {0x004CCBFA +2, 0x08},
  {0x004CCC00 +2, 0x04},
  0
};

// Code replacements to correctly read STR or STRx data
// All the original code is of the form:
//   MOVZX E?X,AX
//   MOVZX E?X,WORD PTR DS:[E?X+E?X*2+2]
//   ADD EAX,E?X
u32 patch_strdata[] = {
  0x00427F59,
  0x0042803C,
  0x00428092,
  0x0045AC8D,
  0x004C5136,
  0x004C57CF,
  0x00428161,
  0x0046CD9A,
  0x0047B0DC,
  0x004BD0E0,
  0x004BE527,
  0x004BF751,
  0
};

// code for null-checking on portraits -- extra code is placed in the 15 bytes between some functions
u8 patch_portfix1[] = {
/*0045E761*/ 0x8B,0x14,0x95,0x50,0xAC,0x68,0x00, // MOV EDX,DWORD PTR DS:[EDX*4+0x68AC50]
/*0045E768*/ 0x85,0xD2,                          // test edx,edx
/*0045E76A*/ 0x75, (0x0045E7E5 - 0x0045E76C),    // jnz short 0x0045E7E5 ; proceed as usual
/*0045E76C*/ 0xFE,0xCB,                          // dec bl
/*0045E76E*/ 0xEB, (0x0045E7EF - 0x0045E770)     // jmp short 0x0045E7EF ; sets an error state? I don't actually know
};
u8 patch_portfix2[] = {
/*0045E7DE*/ 0xEB, (0x0045E761 - 0x0045E7E0),    // jmp  0x0045E761 ; replace move edx,dword ptr[whatever] with a redirect to custom code
/*0045E7E0*/ 0x90,0x90,0x90,0x90,0x90            // nop
};


// rearranges a few opcodes and adds an invalid frame check in the "drawImage" function -- extra opcodes placed between functions
u8 patch_drawfix1[] = {
/*00497D25*/ 0x89,0x55,0xF4,                     // MOV DWORD PTR SS:[EBP-C],EDX
/*00497D28*/ 0x89,0x45,0xFC,                     // MOV DWORD PTR SS:[EBP-4],EAX
/*00497D2B*/ 0x0F,0xB7,0x46,0x1A,                // MOVZX EAX,WORD PTR DS:[ESI+1A] ; image->frameIndex
/*00497D2F*/ 0x8B,0x4E,0x2C,                     // MOV ECX,DWORD PTR DS:[ESI+2C]  ; image->GRPFile
/*00497D32*/ 0x0F,0xB7,0x11,                     // movzx edx,word ptr ds:[ecx]    ; image->GRPFile->frameCount
/*00497D35*/ 0x3B,0xC2,                          // cmp eax,edx
/*00497D37*/ 0x73, (0x00497D4A - 0x00497D39),    // jnb short 0x00497D4A           ; invalid frame -- exit function
/*00497D39*/ 0x8B,0x56,0x30,                     // MOV EDX,DWORD PTR DS:[ESI+30]  ; image->coloringdata
/*00497D3C*/ 0xEB, (0x00497D54 - 0x00497D3E)     // jmp short 0x00497D54
/*00497D3E*/ // normal function continues from here
};
u8 patch_drawfix2[] = {
/*00497D54*/ 0x52,                               // PUSH EDX
/*00497D55*/ 0x8D,0x55,0xF0,                     // LEA EDX,DWORD PTR SS:[EBP-10]
/*00497D58*/ 0x52,                               // PUSH EDX
/*00497D59*/ 0x8D,0x54,0xC1,0x06,                // LEA EDX,DWORD PTR DS:[ECX+EAX*8+6] ; frame offset
/*00497D5D*/ 0xEB, (0x00497D3E - 0x00497D5F)     // jmp short 0x00497D3E
/*00497D5F*/
};



/* ---- SC Variable/Function Pointers ---- */

u8*  gamepal      = (u8*)(0x005994E0);    // u8 gamepal[256][4] -- Contains the WPE, which is helpfully already loaded
u8*  unitColTable = (u8*)(0x00581D76);    // u8 unitColTable[12][8]
u8*  miniColTable = (u8*)(0x00581DD6);    // u8 miniColTable[8]
u8*  unkPlayerCol = (u8*)(0x0058F442);    // u8 unkPlayerCol[8]
u32* disableRNG   = (u32*)(0x006D11C8);   // u32 disableRNG
u32* rngSCounter  = (u32*)(0x0051C610);   // u32 rngSCounter[256]
u32* rngSeed      = (u32*)(0x0051CA14);   // u32 rngSeed
u32* rngCounter   = (u32*)(0x0051CA18);   // u32 rngCounter
u8** chkStrPtr    = (u8**)(0x005993D4);   // u8* chkStrPtr
u32* chkStrSize   = (u32*)(0x005994D8);   // u32 chkStrSize
u8** stat_txtPtr  = (u8**)(0x006D1238);   // u8* stat_txtPtr

// Fixed STR address to match SC:R
u8* eudStrBase = NULL;
u32 eudStrSize = 0;

u32* UnitsLost     = (u32*)(0x00581E74);  // u32 UnitsLost[12]
u32* BuildingsLost = (u32*)(0x00581FC4);  // u32 BuildingsLost[12]
u32* FactoriesLost = (u32*)(0x005820E4);  // u32 FactoriesLost[12]
u32* UnitDeaths    = (u32*)(0x0058A364);  // u32 UnitDeaths[228][12]
u32* CurrentPlayer = (u32*)(0x006509B0);  // u32 CurrentPlayer

// Pointers to functions using standard calling conventions (no assembly required)
//PlayerGroupIterator_Callback getDeaths = (PlayerGroupIterator_Callback)(0x00460380);
PlayerGroupIterator GetAllPlayers      = (PlayerGroupIterator)(0x0045FC40);
PlayerGroupIterator GetNeutralPlayers  = (PlayerGroupIterator)(0x0045FCA0);
PlayerGroupIterator GetAllies          = (PlayerGroupIterator)(0x0045FD00);
PlayerGroupIterator GetFoes            = (PlayerGroupIterator)(0x0045FD60);
PlayerGroupIterator GetNAVPlayers      = (PlayerGroupIterator)(0x0045FDD0);

// Some CHK_STR related sc functions
#define sc_AppAddExit "0x004F6100"
//#define sc_UnloadSTR  "0x004CB370"

// Addresses of Deaths/Set Deaths
#define CONDITIONS         0x00515A98 // Condition function pointer array
#define CONDITION_DEATHS   CONDITIONS + (15*4)
#define ACTIONS            0x00512800 // Action function pointer array
#define ACTION_SET_DEATHS  ACTIONS + (45*4)

// Memory Backup table -- Defines ranges of memory values that can be edited by EUDs but are not reloaded before each game
struct {
  u32 start; // First byte of data
  u32 end;   // End of data, exclusive
  u32 size;  // Size (end - start)
  void* buf; // Pointer to backed up memory
} memBackup[] = {
  {0x005124D8, 0x005124F4, 0, NULL}, // GameSpeedModifiers
  {0x00514178, 0x00515224, 0, NULL}, // Unit/Upgrade/Tech/Order Requirements & Advisor Portraits
  {0x005152A8, 0x005153E8, 0, NULL}, // Stuff
  {0x00515AF8, 0x00519E50, 0, NULL}, // ContoursCreate, Damage Multipliers, Buttonsets
//  {0x0041A424, 0x0041BBF8, 0, NULL}, // Scratch/Callbacks -- this seemed to make maps unreadable so was excluded
  {0x00666778, 0x0066FBE4, 0, NULL}, // images.dat
// TODO: More?
  {0} // end of list
};


/* ---- Tables and Variables ---- */

// remaster-only player colors -- first 4 are the extras from tunit.pcx, the rest are best matches to the "GameBasic" palette
const u8 scrcolors[11][8] = {
{ 77,  77,  76,  38,  37,  35,  35,  34}, // 12 - pale green
{154, 154, 151, 151, 149, 146,  47,  44}, // 13 - bluish grey
{136, 136, 132, 131, 129, 147,  99,  68}, // 14 - pale yellow
{128, 128, 128,  52,  52,  49, 157, 157}, // 15 - cyan
{ 83, 153,  78, 150, 148, 144,  67,  65}, // 16 - pink (there is not actually any pink lmao)
{101,  97,  97,  92,  92,  89,  86, 138}, // 17 - olive
{166, 166, 106, 101,  97,  96,  16,  86}, // 18 - lime
{160,  42,  42,  42,  41, 118,  40, 138}, // 19 - navy
{164, 164, 164, 163, 161, 161,  90, 139}, // 20 - magenta
{ 74,  74,  73, 144, 142, 141, 140,  64}, // 21 - grey
{142,  67, 141,  66, 140, 139,  64, 138}  // 22 - black
};

// Color intensities for player color gradient
const u8 pcolgrad[8] = {255, 222, 189, 156, 124, 91, 58, 25};

// COLR section data
u8 colrdata[8] = {0,1,2,3,4,5,6,7};

// Used to shuffle colors for CRGB_RANDOM
u8 colorShuffle[23] = {0};

// CRGB section data
sCRGB crgbdata = {0};

u32 hascrgb = 0; // CRGB section was found
u32 hascolr = 0; // COLR section was found
u32 hasstrx = 0; // STRx section was found

// Mask used for EUD Actions, since adding a parameter to the functions would be hard
u32 eudmask = 0xFFFFFFFF;

// For getting storm functions
HMODULE hStorm = NULL;
SMemAlloc_type SMemAlloc;
SMemFree_type SMemFree;

SFileCloseFile_type SFileCloseFile;
SFileOpenFileEx_type SFileOpenFileEx;
SFileReadFile_type SFileReadFile;
SFileSetFilePointer_type SFileSetFilePointer;


__fastcall bool TRGCND_Deaths(condition* c);
__fastcall u32 getDeaths(u32 player, u16 unit, u32 number);

__fastcall bool TRGACT_SetDeaths(action* a);
__fastcall u32 SetDeaths(u32 player, u16 unit, u32 number);
__fastcall u32 AddDeaths(u32 player, u16 unit, u32 number);
__fastcall u32 SubtractDeaths(u32 player, u16 unit, u32 number);

// Similar to the PlayerGroupIterators above, but does not use a standard calling convention
u32 GetForcePlayers(u32 player, u32 unit, u32 number, PlayerGroupIterator_Callback cb);

__fastcall void cleardata();
void restoremem();
__fastcall u32 getStr();
__stdcall void* loadGameAlloc(int size, char* filename, int line, int idk);
__stdcall void loadGameFree(void* ptr, char* filename, int line, int idk);
void unloadSTR();

void setPlayerColors(u32 count, u8* buffer);
void genPCol(u8* dst, u8 r, u8 g, u8 b);
u8 pcolLookup(u8 r, u8 g, u8 b);
u16 randBW(u32 id);


void __fastcall fixFrame();




/* ---- EUD Functions ---- */


__fastcall bool TRGCND_Deaths(condition* c){
  u32 deaths = getDeaths(c->player, c->unit, 0);
  u32 num = c->number;
  
  if(c->maskFlag == EUD_MASK_FLAG){
    deaths &= c->location;
    num &= c->location;
  }
  
  switch(c->modifier){
    case MODIFIER_EXACTLY:
      return deaths == num;
    case MODIFIER_AT_MOST:
      return deaths <= num;
    case MODIFIER_AT_LEAST:
      return deaths >= num;
    default:
      return false;
  }
}

__fastcall u32 getDeaths(u32 player, u16 unit, u32 number){
  u32 ptr;
  switch(player){
    case PLAYER_FOES:
      return GetFoes(unit, 0, &getDeaths);
    case PLAYER_ALLIES:
      return GetAllies(unit, 0, &getDeaths);
    case PLAYER_NEUTRAL_PLAYERS:
      return GetNeutralPlayers(unit, 0, &getDeaths);
    case PLAYER_ALL_PLAYERS:
      return GetAllPlayers(unit, 0, &getDeaths);
    case PLAYER_FORCE_1:
    case PLAYER_FORCE_2:
    case PLAYER_FORCE_3:
    case PLAYER_FORCE_4:
      return GetForcePlayers(player, unit, 0, &getDeaths);
    case PLAYER_UNUSED_1:
    case PLAYER_UNUSED_2:
    case PLAYER_UNUSED_3:
    case PLAYER_UNUSED_4:
      return 0;
    case PLAYER_NON_ALLIED_VIC:
      return GetNAVPlayers(unit, 0, &getDeaths);
    
    case PLAYER_CURRENT_PLAYER:
      player = *CurrentPlayer;
    default:
      break;
  }
  switch(unit){
    case UNIT_ANY_UNIT:
      return BuildingsLost[player] + UnitsLost[player];
    case UNIT_MEN:
      return UnitsLost[player];
    case UNIT_BUILDINGS:
      return BuildingsLost[player];
    case UNIT_FACTORIES:
      return FactoriesLost[player];
    default:
      ptr = (u32)UnitDeaths + player*4 + unit*48;
      if(ptr == 0) return 0; // null pointer
      return UnitDeaths[player + unit*12];
  }
}


__fastcall bool TRGACT_SetDeaths(action* a){
  eudmask = 0xFFFFFFFF; // reset mask
  if(a->maskFlag == EUD_MASK_FLAG){
    eudmask = a->location;
  }
  
  switch(a->modifier){
    case MODIFIER_SUBTRACT:
      SubtractDeaths(a->player, a->type, a->number);
      break;
    case MODIFIER_ADD:
      AddDeaths(a->player, a->type, a->number);
      break;
    case MODIFIER_SET_TO:
      SetDeaths(a->player, a->type, a->number);
    default:
      break;
  }
  return true;
}

__fastcall u32 SetDeaths(u32 player, u16 unit, u32 number){
  u32 ptr;
  switch(player){
    case PLAYER_NON_ALLIED_VIC:
      return GetNAVPlayers(unit, number, &SetDeaths);
    case PLAYER_FOES:
      return GetFoes(unit, number, &SetDeaths);
    case PLAYER_ALLIES:
      return GetAllies(unit, number, &SetDeaths);
    case PLAYER_NEUTRAL_PLAYERS:
      return GetNeutralPlayers(unit, number, &SetDeaths);
    case PLAYER_ALL_PLAYERS:
      return GetAllPlayers(unit, number, &SetDeaths);
    case PLAYER_FORCE_1:
    case PLAYER_FORCE_2:
    case PLAYER_FORCE_3:
    case PLAYER_FORCE_4:
      return GetForcePlayers(player, unit, number, &SetDeaths);
    case PLAYER_UNUSED_1:
    case PLAYER_UNUSED_2:
    case PLAYER_UNUSED_3:
    case PLAYER_UNUSED_4:
      return 0;
    
    case PLAYER_CURRENT_PLAYER:
      player = *CurrentPlayer;
    default:
      //if(player >= 8) return false; // :)
      //if(unit >= 233) return false; // :)
      break;
  }
  switch(unit){
    case UNIT_ANY_UNIT:
      return 0;
    case UNIT_MEN:
      UnitsLost[player] &= ~eudmask;
      UnitsLost[player] |= number & eudmask;
      return 0;
    case UNIT_BUILDINGS:
      BuildingsLost[player] &= ~eudmask;
      BuildingsLost[player] |= number & eudmask;
      return 0;
    case UNIT_FACTORIES:
      FactoriesLost[player] &= ~eudmask;
      FactoriesLost[player] |= number & eudmask;
      return 0;
    default:
      //if(unit >= 228) return false; // :)
      ptr = (u32)UnitDeaths + player*4 + unit*48;
      if(ptr == 0) return 0; // null pointer
      UnitDeaths[player + unit*12] &= ~eudmask;
      UnitDeaths[player + unit*12] |= number & eudmask;
  }
  return 0;
}

__fastcall u32 AddDeaths(u32 player, u16 unit, u32 number){
  u32 ptr,val;
  switch(player){
    case PLAYER_NON_ALLIED_VIC:
      return GetNAVPlayers(unit, number, &AddDeaths);
    case PLAYER_FOES:
      return GetFoes(unit, number, &AddDeaths);
    case PLAYER_ALLIES:
      return GetAllies(unit, number, &AddDeaths);
    case PLAYER_NEUTRAL_PLAYERS:
      return GetNeutralPlayers(unit, number, &AddDeaths);
    case PLAYER_ALL_PLAYERS:
      return GetAllPlayers(unit, number, &AddDeaths);
    case PLAYER_FORCE_1:
    case PLAYER_FORCE_2:
    case PLAYER_FORCE_3:
    case PLAYER_FORCE_4:
      return GetForcePlayers(player, unit, number, &AddDeaths);
    case PLAYER_UNUSED_1:
    case PLAYER_UNUSED_2:
    case PLAYER_UNUSED_3:
    case PLAYER_UNUSED_4:
      return 0;
    
    case PLAYER_CURRENT_PLAYER:
      player = *CurrentPlayer;
    default:
      //if(player >= 8) return false; // :)
      //if(unit >= 233) return false; // :)
      break;
  }
  switch(unit){
    case UNIT_ANY_UNIT:
      return 0;
    case UNIT_MEN:
      val = UnitsLost[player] & eudmask;
      UnitsLost[player] &= ~eudmask;
      number &= eudmask;
      UnitsLost[player] |= (val + number) & eudmask;
      return 0;
    case UNIT_BUILDINGS:
      val = BuildingsLost[player] & eudmask;
      BuildingsLost[player] &= ~eudmask;
      number &= eudmask;
      BuildingsLost[player] |= (val + number) & eudmask;
      return 0;
    case UNIT_FACTORIES:
      val = FactoriesLost[player] & eudmask;
      FactoriesLost[player] &= ~eudmask;
      number &= eudmask;
      FactoriesLost[player] |= (val + number) & eudmask;
      return 0;
    default:
      //if(unit >= 228) return false; // :)
      ptr = (u32)UnitDeaths + player*4 + unit*48;
      if(ptr == 0) return 0; // null pointer
      val = UnitDeaths[player + unit*12] & eudmask;
      UnitDeaths[player + unit*12] &= ~eudmask;
      number &= eudmask;
      UnitDeaths[player + unit*12] |= (val + number) & eudmask;
  }
  return 0;
}

__fastcall u32 SubtractDeaths(u32 player, u16 unit, u32 number){
  u32 ptr,val;
  switch(player){
    case PLAYER_NON_ALLIED_VIC:
      return GetNAVPlayers(unit, number, &SubtractDeaths);
    case PLAYER_FOES:
      return GetFoes(unit, number, &SubtractDeaths);
    case PLAYER_ALLIES:
      return GetAllies(unit, number, &SubtractDeaths);
    case PLAYER_NEUTRAL_PLAYERS:
      return GetNeutralPlayers(unit, number, &SubtractDeaths);
    case PLAYER_ALL_PLAYERS:
      return GetAllPlayers(unit, number, &SubtractDeaths);
    case PLAYER_FORCE_1:
    case PLAYER_FORCE_2:
    case PLAYER_FORCE_3:
    case PLAYER_FORCE_4:
      return GetForcePlayers(player, unit, number, &SubtractDeaths);
    case PLAYER_UNUSED_1:
    case PLAYER_UNUSED_2:
    case PLAYER_UNUSED_3:
    case PLAYER_UNUSED_4:
      return 0;
    
    case PLAYER_CURRENT_PLAYER:
      player = *CurrentPlayer;
    default:
      //if(player >= 8) return false; // :)
      //if(unit >= 233) return false; // :)
      break;
  }
  switch(unit){
    case UNIT_ANY_UNIT:
      return 0;
    case UNIT_MEN:
      val = UnitsLost[player] & eudmask;
      UnitsLost[player] &= ~eudmask;
      number &= eudmask;
      if(val > number){
        UnitsLost[player] |= (val - number) & eudmask;
      }
      return 0;
    case UNIT_BUILDINGS:
      val = BuildingsLost[player] & eudmask;
      BuildingsLost[player] &= ~eudmask;
      number &= eudmask;
      if(val > number){
        BuildingsLost[player] |= (val - number) & eudmask;
      }
      return 0;
    case UNIT_FACTORIES:
      val = FactoriesLost[player] & eudmask;
      FactoriesLost[player] &= ~eudmask;
      number &= eudmask;
      if(val > number){
        FactoriesLost[player] |= (val - number) & eudmask;
      }
      return 0;
    default:
      //if(unit >= 228) return false; // :)
      ptr = (u32)UnitDeaths + player*4 + unit*48;
      if(ptr == 0) return 0; // null pointer
      val = UnitDeaths[player + unit*12] & eudmask;
      UnitDeaths[player + unit*12] &= ~eudmask;
      number &= eudmask;
      if(val > number){
        UnitDeaths[player + unit*12] |= (val - number) & eudmask;
      }
  }
  return 0;
}


// Similar to the PlayerGroupIterator, but does not use a standard calling convention
u32 __attribute__ ((noinline)) GetForcePlayers(u32 player, u32 unit, u32 number, PlayerGroupIterator_Callback cb){
  u32 out;
  // eax = player, ebx = cb;
  __asm __volatile("\n"
"    push ecx\n" // arg2 number
"    push edx\n" // arg1 unit
"    mov edx,0x0045FBE0\n" // getForcePlayers 0045FBE0 00000060
"    call edx\n"
 : "=a"(out)
 : "a"(player),"b"(cb),"d"(unit),"c"(number)
 : 
);
  return out;
}




/* ---- CHK Loading Functions ---- */

__stdcall u32 CHK_STRx(chk_call_data* thing, u32 size, u32 idk){
  if(eudStrBase != NULL){
    *chkStrPtr = (u8*)VirtualAlloc((void*)EUD_STR_PTR, size, MEM_COMMIT, PAGE_READWRITE);
    if(*chkStrPtr == NULL) return 0;
    *chkStrPtr = (u8*)EUD_STR_PTR;
    eudStrSize = size;
  }else{
    if(*chkStrPtr != NULL) SMemFree(*chkStrPtr, PLUGIN_NAME, __LINE__, 0);
    *chkStrPtr = (u8*)SMemAlloc(size, PLUGIN_NAME, __LINE__, 0);
    if(*chkStrPtr == NULL) return 0;
  }
  *chkStrSize = size;
  
  // the easiest way I could think of to call this function
  __asm __volatile("\n"
//"    mov ebx,"sc_UnloadSTR"\n"  // handle to UnloadSTR function
"    mov edx,"sc_AppAddExit"\n"
"    call edx\n" // AppAddExit(&unloadStr)
 : /*no outputs*/
 : "b"(&unloadSTR) // ebx = &unloadSTR
 : "eax","ecx","edx" // clobbers everything
);
  
  if((u32)thing->data + thing->size > thing->end) return 0;
  memcpy(*chkStrPtr, thing->data, thing->size);
  
  // 1 if 'STRx', 0 if 'STR '
  hasstrx = (thing->name[3] == 'x');
  return 1;
}

// real COLR section -- always applies -- load after CRGB in case CRGB doesn't exist
__stdcall u32 CHK_COLR(chk_call_data* thing, u32 size, u32 idk){
  u8 data[8];
  if(size != 8 || (u32)thing->data + thing->size > thing->end) return 0;
  memcpy(data, thing->data, thing->size);
  hascolr = 1;
  setPlayerColors(8, data);
  return 1;
}

// optional COLR section -- only applies if CRGB is present -- load before CRGB
__stdcall u32 CHK_COLR_opt(chk_call_data* thing, u32 size, u32 idk){
  if(size != 8 || (u32)thing->data + thing->size > thing->end) return 0;
  memcpy(colrdata, thing->data, thing->size);
  hascolr = 1;
  return 1;
}

__stdcall u32 CHK_CRGB(chk_call_data* thing, u32 size, u32 idk){
  if(size != 32 || (u32)thing->data + thing->size > thing->end) return 0;
  memcpy(&crgbdata, thing->data, thing->size);
  hascrgb = 1;
  
  // Apply colors if "optional" COLR section has been found, otherwise they will be applied later by the "real" COLR section
  if(hascolr){
    setPlayerColors(8, colrdata);
  }
  return 1;
}



// Clears whether or not SC:R sections should be used -- this is called whenever a CHK is being parsed
__fastcall void cleardata(){
  hascrgb = 0;
  hascolr = 0;
  hasstrx = 0;
  oggCacheClear();
  restoremem();
  __asm __volatile(""
 : /*no outputs*/
 : "a"(&mapchunks[MAP_VER_COUNT]) // mov eax, &mapchunks[MAP_VER_COUNT] ; eax points to the first byte past the end of the table
 : /*no clobbers*/
);
}

// Backs up and restores memory that can be modified by EUDs but is not restored on map load
void restoremem(){
  int i;
  for(i = 0; memBackup[i].start != 0; i++){
    if(memBackup[i].buf == NULL){
      // Get memory backups
      memBackup[i].size = memBackup[i].end - memBackup[i].start;
      memBackup[i].buf = malloc(memBackup[i].size);
      memcpy(memBackup[i].buf, (u8*)(memBackup[i].start), memBackup[i].size);
    }else{
      // Restore memory backup
      memcpy((u8*)(memBackup[i].start), memBackup[i].buf, memBackup[i].size);
    }
  }
}


// Reads "STR " formatted or "STRx" formatted strings depending on which section was loaded
//   inputs: edx/ecx chkStrPtr, eax strID
//   outputs: eax str pointer
__fastcall u32 getStr(){
  u32 strID;
  u32 out;
  u32 stat_txt;
  __asm __volatile("" : "=a"(strID),"=c"(stat_txt)); // get argument from eax, and tbl pointer from ecx
  // ignore edx/ecx entirely so the same function can be used in either case
  
  if(stat_txt == (u32)(*stat_txtPtr)){ // Always read STR format if the pointer was supposed to be stat_txt -- necessary because the assembly redirected at 0047B0DC can read either table
    out = (u32)(*stat_txtPtr) + ((u16*)(*stat_txtPtr))[strID+1];
  }else if(hasstrx){ // read STR table normally
    // STRx format
    out = (u32)(*chkStrPtr) + ((u32*)(*chkStrPtr))[strID+1];
  }else{
    // STR format
    out = (u32)(*chkStrPtr) + ((u16*)(*chkStrPtr))[strID+1];
  }
  
  __asm __volatile("" : : "d"(*chkStrPtr)); // set EDX to *chkStrPtr for the single case where it's needed
  return out;
}


// Replaces a call to SMemAlloc
__stdcall void* loadGameAlloc(int size, char* filename, int line, int idk){
  void* ptr;
  if(eudStrBase != NULL){
    ptr = VirtualAlloc((void*)EUD_STR_PTR, size, MEM_COMMIT, PAGE_READWRITE);
  }else{
    ptr = SMemAlloc(size, filename, line, idk);
  }
  return ptr;
}
// Replaces calls to SMemFree
__stdcall void loadGameFree(void* ptr, char* filename, int line, int idk){
  if(eudStrBase != NULL){
    // idk
  }else{
    if(ptr != NULL) SMemFree(ptr, filename, line, idk);
  }
}


// Called on exit
void unloadSTR(){
  if(eudStrBase != NULL){
    VirtualFree(eudStrBase, 0, MEM_RELEASE);
    eudStrBase = NULL;
  }else{
    if(*chkStrPtr != NULL) SMemFree(*chkStrPtr, PLUGIN_NAME, __LINE__, 0);
  }
  *chkStrPtr = NULL;
  return;
}


// Based on 0x0049B130, but now also applies CRGB colors if the section is present
void setPlayerColors(u32 count, u8* buffer){
  u8 tmini[12];  // [ebp-c]
  u8 tunit[96]; // [ebp-6c] -- neat note: the first 12 entries of the unit color table are copied to the stack, which explains why colors 12 to 15 don't work despite being in tunit.pcx and anything >11 gives random garbage
  u8 useCOLR;
  
  memcpy(tunit, unitColTable, 96);
  memcpy(tmini, miniColTable, 12);
  
  if(hascrgb){
    u8 useRandom = 0;
    u8 i,j,swap,tmp;
    
    // Find all available colors
    memset(colorShuffle, 1, sizeof(colorShuffle)); // mark all as available
    for(i = 0; i < count; i++){
      if(crgbdata.mode[i] == CRGB_RANDOM){
        useRandom = 1;
      }else if(crgbdata.mode[i] == CRGB_COLR){
        if(buffer[i] < 23){
          // exclude color from shuffle pool
          colorShuffle[buffer[i]] = 0;
        }
      }
    }
    
    // Proceed only if necessary
    if(useRandom){
      // Exclude P9 - P12 colors, maybe?
      for(i = 8; i < 12; i++) colorShuffle[i] = 0;
      
      // Initialize list of available colors
      j = 0;
      for(i=0; i < 23; i++){
        if(colorShuffle[i]){
          colorShuffle[j] = i;
          j++;
        }
      }
      
      // Shuffle list
      for(i = 0; i < j; i++){
        // TODO: rng isn't set when this is called :(
        swap = randBW(28) % j;
        if(i == swap) continue;
        tmp = colorShuffle[swap];
        colorShuffle[swap] = colorShuffle[i];
        colorShuffle[i] = tmp;
      }
    }
  }
  
  while(count != 0){
    count--;
    
    useCOLR = 0;
    
    // apply CRGB
    if(hascrgb){
      switch(crgbdata.mode[count]){
        case CRGB_RANDOM:
          useCOLR = 1;
          buffer[count] = colorShuffle[count]; // override color with shuffled color
          break;
        case CRGB_RGB:
          genPCol(&unitColTable[count*8], crgbdata.rgb[count][0], crgbdata.rgb[count][1], crgbdata.rgb[count][2]);
          miniColTable[count] = unitColTable[count*8];
          break;
        case CRGB_COLR:
          useCOLR = 1;
          break;
        case CRGB_USER:
          // can't implement this
        default:
          break;
      }
    }
    
    // apply COLR
    if(hascolr && (!hascrgb || useCOLR)){
      if(buffer[count] != count){
        if(hascrgb && buffer[count] > 11){ // use scr colors
          if(buffer[count] > 22){
            buffer[count] = count; // large IDs just use default color
          }else{
            buffer[count] -= 12; // SC:R colors start at 12
            memcpy(&unitColTable[count*8], scrcolors[buffer[count]], 8);
            miniColTable[count] = scrcolors[buffer[count]][0];
            unkPlayerCol[count] = buffer[count] + 12;
            continue;
          }
        }
        // use fun overflow colors (hello, random stack data :) )
        memcpy(&unitColTable[count*8], &tunit[buffer[count]*8], 8);
        miniColTable[count] = tmini[buffer[count]];
        unkPlayerCol[count] = buffer[count];
      }
    }
  }
}


// Generates a player color using closest palette matches
void genPCol(u8* dst, u8 r, u8 g, u8 b){
  u8 i;
  u32 rr,gg,bb;
  for(i=0; i<8; i++){
    rr = r * pcolgrad[i] / 255;
    gg = g * pcolgrad[i] / 255;
    bb = b * pcolgrad[i] / 255;
    dst[i] = pcolLookup(rr, gg, bb);
  }
}

// Finds the closest palette match to the given RGB value
u8 pcolLookup(u8 r, u8 g, u8 b){
  u32 i;
  u8 id = 0;
  s32 dist = 3*256*256; // greater than the maximum possible distance
  s32 dr,dg,db;
  s32 calc;
  
  for(i=0; i<256; i++){
    // Should color cycling colors be excluded?
    //if( (i >= 1 && i <= 13) || (i >= 248 && i <= 254) ) continue;
    if(i == 1) i = 14;
    if(i == 248) i = 255;
    
    dr = r - gamepal[i*4];
    dg = g - gamepal[i*4+1];
    db = b - gamepal[i*4+2];
    calc = dr*dr + dg*dg + db*db;
    if(calc == 0)
      return i; // exact color -- stop looking
    if(calc < dist){
      dist = calc;
      id = i;
    }
  }
  return id;
}

// BW RNG Function
u16 randBW(u32 id){
  if(*disableRNG) return 0;
  rngSCounter[id]++;
  *rngCounter++;
  *rngSeed *= 0x15A4E35;
  *rngSeed++;
  return (*rngSeed >> 16) & 0x7FFF;
}


/* ---- Misc Patches ---- */

// Prevents drawing invalid GRP frames -- modified from isdbg
void __fastcall fixFrame(){
  CImage* img;
  grpHead* grp;
  u32 frame;
  u32 sprite;
  u32 esi,edi;
  __asm __volatile("" : "=a" (img), "=S" (esi), "=D" (edi));
  
  grp = img->GRPFile;
  if(img->frameIndex >= grp->frameCount){
    //img->frameIndex = 0;
    //img->frameSet = 0;
    frame = 0; // don't actually change the frame !
  }else{
    frame = img->frameIndex;
  }
  sprite = img->spriteOwner;
  
  __asm __volatile("" : : "a" (img), "c" (frame), "d" (sprite), "S" (esi), "D" (edi));
}




/* ---- Plugin Call ---- */

u32 patch(){
  int i,j, min, max;
  u8* offs = NULL;
  u32* addr = NULL;
  DWORD oldPVal;
  
  // VirtualProtect wants a range, so let's just calculate it
  min = 0x7FFFFFFF;
  max = 0;
  
  for(i=0; patch_mapchunks[i].address != 0; i++){
    if(patch_mapchunks[i].address < min) min = patch_mapchunks[i].address;
    if(patch_mapchunks[i].address > max) max = patch_mapchunks[i].address;
  }
  for(i=0; patch_strdata[i] != 0; i++){
    if(patch_strdata[i] < min) min = patch_strdata[i];
    if(patch_strdata[i] > max) max = patch_strdata[i];
  }
  if(0x004BD7E7 < min) min = 0x004BD7E7;
  if(0x004BD7EE > max) max = 0x004BD7EE;
  if(0x004D57B6 < min) min = 0x004D57B6;
  if(0x004D57B6 > max) max = 0x004D57B6;
  if(0x0045E761 < min) min = 0x0045E761;
  if(0x0045E7E4 > max) max = 0x0045E7E4;
  if(0x00497D25 < min) min = 0x00497D25;
  if(0x00497D5E > max) max = 0x00497D5E;
  if(0x004BC518 < min) min = 0x004BC518;
  if(0x004BC51D > max) max = 0x004BC51D;
  if(0x004D00EE < min) min = 0x004D00EE;
  if(0x004EEB76 > max) max = 0x004EEB76;
  
  if(min > max){
   return true;
  }
  
  if(VirtualProtect((int*)min, max - min, PAGE_EXECUTE_READWRITE, &oldPVal) == 0){
   return false;
  }
  
  // replace map chunk table pointers
  for(i = 0; patch_mapchunks[i].address != 0; i++){
    addr = (u32*)(patch_mapchunks[i].address);
    *addr = (u32)(mapchunks) + patch_mapchunks[i].arrayOffset;
  }
  // Replace at 0x004CC0BB:
  //   MOV ECX,3                 ; old map version count
  //   MOV EAX,StarCraf.005005D8 ; end of chunk table
  // with:
  //   CALL cleardata            ; returns end of chunk table in EAX
  //   MOV ECX,MAP_VER_COUNT     ; new value
  offs = (u8*)(0x004CC0BB);
  addr = (u32*)(0x004CC0BB +1);
  offs[0] = OP_CALL;
  *addr = (u32)(&cleardata) - 0x004CC0C0; // relative pointer to function
  
  addr = (u32*)(0x004CC0C0 +1);
  offs[5] = 0xB9; // MOV ECX, imm32
  *addr = MAP_VER_COUNT;
  
  
  // replace STR access with a function call
  for(i = 0; patch_strdata[i] != 0; i++){
    offs = (u8*)(patch_strdata[i]);
    offs[2] = 0xC0; // MOVZX E?X,AX -> MOVZX EAX,AX
    offs[3] = OP_CALL;
    addr = (u32*)(patch_strdata[i] + 4);
    *addr = (u32)(&getStr) - (patch_strdata[i] + 8); // relative pointer to function
    offs[8] = OP_NOP;
    offs[9] = OP_NOP;
  }
  // special case where the result needs to be in ESI
  offs = (u8*)(0x004C57D9);
  offs[0] = 0x8B;
  offs[1] = 0xF0; // MOV ESI,EAX
  
  // Changes some calls to SMemFree and SMemAlloc relating to the STR buffer
  addr = (u32*)(0x004D00EE +1);
  *addr = (u32)(&loadGameFree) - 0x004D00F3;
  addr = (u32*)(0x004D0105 +1);
  *addr = (u32)(&loadGameAlloc) - 0x004D010A;
  addr = (u32*)(0x004D0295 +1);
  *addr = (u32)(&loadGameFree) - 0x004D029A;
  addr = (u32*)(0x004EEB76 +1);
  *addr = (u32)(&loadGameFree) - 0x004EEB7B;
  
  
  // Invalid grp frame fix -- CImage__updateGraphicData
  offs = (u8*)(0x004D57B6);
  addr = (u32*)(0x004D57B6 +1);
  offs[0] = OP_CALL;
  *addr = (u32)(&fixFrame) - (0x004D57B6 + 5);
  offs[5] = OP_NOP;
  offs[6] = OP_NOP;
  
  // Invalid grp frame fix -- drawImage
  addr = (u32*)(0x00497D25);
  memcpy(addr, patch_drawfix1, sizeof(patch_drawfix1));
  addr = (u32*)(0x00497D54);
  memcpy(addr, patch_drawfix2, sizeof(patch_drawfix2));
  
  // Invalid portrait fix
  addr = (u32*)(0x0045E761);
  memcpy(addr, patch_portfix1, sizeof(patch_portfix1));
  addr = (u32*)(0x0045E7DE);
  memcpy(addr, patch_portfix2, sizeof(patch_portfix2));
  
  // playOgg call
  offs = (u8*)(0x004BC518);
  addr = (u32*)(0x004BC51A);
  *offs = OP_NOP; // remove "push eax"
  *addr = (u32)(&playOgg) - 0x004BC51E;
  
  
  // Restore permissions
  VirtualProtect((int*)min, max - min, oldPVal, &oldPVal);
  
  // Write condition/action functions
  addr = (u32*)(CONDITION_DEATHS);
  *(u32**)addr = (u32*)(&TRGCND_Deaths);
  
  addr = (u32*)(ACTION_SET_DEATHS);
  *(u32**)addr = (u32*)(&TRGACT_SetDeaths);
  
  // Get Storm functions
  hStorm = GetModuleHandle("storm.dll");
  SMemAlloc = (SMemAlloc_type)GetProcAddress(hStorm, (LPSTR)401);
  SMemFree = (SMemFree_type)GetProcAddress(hStorm, (LPSTR)401);
  
  SFileCloseFile = (SFileCloseFile_type)GetProcAddress(hStorm, (LPSTR)253);
  SFileOpenFileEx = (SFileOpenFileEx_type)GetProcAddress(hStorm, (LPSTR)268);
  SFileReadFile = (SFileReadFile_type)GetProcAddress(hStorm, (LPSTR)269);
  SFileSetFilePointer = (SFileSetFilePointer_type)GetProcAddress(hStorm, (LPSTR)271);
  
  // Reserve STR memory
  eudStrBase = (u8*)VirtualAlloc((void*)EUD_STR_PTR, EUD_STR_MAX_SIZE, MEM_RESERVE, PAGE_NOACCESS);
  eudStrSize = 0;
  // what to do if it fails?
  
  // Hopefully everything.
  return true;
}

