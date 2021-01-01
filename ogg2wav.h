#include <vorbis/codec.h>
#include "eudr.h"

// WAV Chunk Names
#define WAVE_RIFF 0x46464952
#define WAVE_WAVE 0x45564157
#define WAVE_fmt  0x20746D66
#define WAVE_data 0x61746164

// WAV file format
typedef struct {          // value:
/*00*/ u32 riffID;        // WAVE_RIFF
/*04*/ u32 riffSize;      // sizeof(wavehdr) + dataSize
/*08*/ u32 waveID;        // WAVE_WAVE
/*0C*/ u32 fmtID;         // WAVE_fmt
/*10*/ u32 fmtSize;       // 16
/*14*/ u16 format;        // 1 -- start of WAVEFORMATEX
/*16*/ u16 channels;      // vi.channels
/*18*/ u32 sampleRate;    // vi.rate
/*1C*/ u32 byteRate;      // vi.rate * vi.channels * 2
/*20*/ u16 blockAlign;    // vi.channels * 2
/*22*/ u16 bitsPerSample; // 16
/*24*/ u32 dataID;        // WAVE_data
/*28*/ u32 dataSize;      // #samples * vi.channels * 2
} wavehdr;

// ogg/vorbis decoding structs (in a struct rather than passing all of these as individual arguments)
typedef struct{
  ogg_sync_state   oy;
  ogg_stream_state os;
  ogg_page         og;
  ogg_packet       op;
  vorbis_info      vi;
  vorbis_comment   vc;
  vorbis_dsp_state vd;
  vorbis_block     vb;
} oggdecdata;


/* ---- DirectSound Things ---- */
/*#define DSCAPS_PRIMARYMONO          0x00000001
#define DSCAPS_PRIMARYSTEREO        0x00000002
#define DSCAPS_PRIMARY8BIT          0x00000004
#define DSCAPS_PRIMARY16BIT         0x00000008
#define DSCAPS_CONTINUOUSRATE       0x00000010
#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800
#define DSBPLAY_LOOPING             0x00000001
#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
#define DSBLOCK_FROMWRITECURSOR     0x00000001
#define DSSCL_NORMAL                1
#define DSSCL_PRIORITY              2
#define DSSCL_EXCLUSIVE             3
#define DSSCL_WRITEPRIMARY          4
#define DSBCAPS_PRIMARYBUFFER       0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLDEFAULT         0x000000E0  // Pan + volume + frequency.
#define DSBCAPS_CTRLALL             0x000000E0  // All control capabilities
#define DSBCAPS_STICKYFOCUS         0x00004000
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000  // More accurate play cursor under emulation
#define DSSPEAKER_HEADPHONE         1
#define DSSPEAKER_MONO              2
#define DSSPEAKER_QUAD              3
#define DSSPEAKER_STEREO            4
#define DSSPEAKER_SURROUND          5*/

// DirectSound Buffer Descriptor
typedef struct DSBUFFERDESC {
/*00*/u32 dwSize;             // -78 = 20
/*04*/u32 dwFlags;            // -74 = 0x80 // DSBCAPS_CTRLVOLUME
/*08*/u32 dwBufferBytes;      // -70 = 0x4000
/*0C*/u32 dwReserved;         // -4C
/*10*/void* lpwfxFormat;      // -68
} DSBUFFERDESC;

typedef struct IDirectSound_ *IDirectSound;
typedef struct IDirectSoundBuffer_ *IDirectSoundBuffer;

// DirectSound Interface Functions
typedef __stdcall HRESULT (*IDirectSound_CreateSoundBuffer)(IDirectSound* dsi, DSBUFFERDESC* lpcDSBufferDesc, IDirectSoundBuffer** lplpDirectSoundBuffer, u32 pUnkOuter);
struct IDirectSound_ {
/*+00*/ void*                             QueryInterface;       // IDirectSound_QueryInterface(p,a,b)
/*+04*/ void*                             AddRef;               // IDirectSound_AddRef(p)
/*+08*/ void*                             Release;              // IDirectSound_Release(p)
/*+0C*/ IDirectSound_CreateSoundBuffer    CreateSoundBuffer;    // IDirectSound_CreateSoundBuffer(p,a,b,c)
/*+10*/ void*                             GetCaps;              // IDirectSound_GetCaps(p,a)
/*+14*/ void*                             DuplicateSoundBuffer; // IDirectSound_DuplicateSoundBuffer(p,a,b)
/*+18*/ void*                             SetCooperativeLevel;  // IDirectSound_SetCooperativeLevel(p,a,b)
/*+1C*/ void*                             Compact;              // IDirectSound_Compact(p)
/*+20*/ void*                             GetSpeakerConfig;     // IDirectSound_GetSpeakerConfig(p,a)
/*+24*/ void*                             SetSpeakerConfig;     // IDirectSound_SetSpeakerConfig(p,b)
/*+28*/ void*                             Initialize;           // IDirectSound_Initialize(p,a)
};

// DirectSoundBuffer Interface Functions
typedef __stdcall HRESULT (*IDirectSoundBuffer_Release)(IDirectSoundBuffer* dsb);
typedef __stdcall HRESULT (*IDirectSoundBuffer_GetCurrentPosition)(IDirectSoundBuffer* dsb, LPDWORD lpdwCurrentPlayCursor, LPDWORD lpdwCurrentWriteCursor);
typedef __stdcall HRESULT (*IDirectSoundBuffer_GetStatus)(IDirectSoundBuffer* dsb, LPDWORD lpdwStatus);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Initialize)(IDirectSoundBuffer* dsb, IDirectSoundBuffer* lpDirectSound, DSBUFFERDESC* lpcDSBufferDesc);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Lock)(IDirectSoundBuffer* dsb, DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID lplpvAudioPtr1, LPDWORD lpdwAudioBytes1, LPVOID lplpvAudioPtr2, LPDWORD lpdwAudioBytes2, DWORD dwFlags);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Play)(IDirectSoundBuffer* dsb, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags);
typedef __stdcall HRESULT (*IDirectSoundBuffer_SetCurrentPosition)(IDirectSoundBuffer* dsb, DWORD dwNewPosition);
typedef __stdcall HRESULT (*IDirectSoundBuffer_SetVolume)(IDirectSoundBuffer* dsb, LONG lVolume);
typedef __stdcall HRESULT (*IDirectSoundBuffer_SetPan)(IDirectSoundBuffer* dsb, LONG lPan);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Stop)(IDirectSoundBuffer* dsb);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Unlock)(IDirectSoundBuffer* dsb, LPVOID lpvAudioPtr1, DWORD dwAudioBytes1, LPVOID lpvAudioPtr2, DWORD dwAudioBytes2);
typedef __stdcall HRESULT (*IDirectSoundBuffer_Restore)(IDirectSoundBuffer* dsb);
struct IDirectSoundBuffer_ {
/*+00*/ void*                                 QueryInterface;     // IDirectSoundBuffer_QueryInterface(p,a,b)
/*+04*/ void*                                 AddRef;             // IDirectSoundBuffer_AddRef(p)
/*+08*/ IDirectSoundBuffer_Release            Release;            // IDirectSoundBuffer_Release(p)
/*+0C*/ void*                                 GetCaps;            // IDirectSoundBuffer_GetCaps(p,a)
/*+10*/ IDirectSoundBuffer_GetCurrentPosition GetCurrentPosition; // IDirectSoundBuffer_GetCurrentPosition(p,a,b)
/*+14*/ void*                                 GetFormat;          // IDirectSoundBuffer_GetFormat(p,a,b,c)
/*+18*/ void*                                 GetVolume;          // IDirectSoundBuffer_GetVolume(p,a)
/*+1C*/ void*                                 GetPan;             // IDirectSoundBuffer_GetPan(p,a)
/*+20*/ void*                                 GetFrequency;       // IDirectSoundBuffer_GetFrequency(p,a)
/*+24*/ IDirectSoundBuffer_GetStatus          GetStatus;          // IDirectSoundBuffer_GetStatus(p,a)
/*+28*/ IDirectSoundBuffer_Initialize         Initialize;         // IDirectSoundBuffer_Initialize(p,a,b)
/*+2C*/ IDirectSoundBuffer_Lock               Lock;               // IDirectSoundBuffer_Lock(p,a,b,c,d,e,f,g)
/*+30*/ IDirectSoundBuffer_Play               Play;               // IDirectSoundBuffer_Play(p,a,b,c)
/*+34*/ IDirectSoundBuffer_SetCurrentPosition SetCurrentPosition; // IDirectSoundBuffer_SetCurrentPosition(p,a)
/*+38*/ void*                                 SetFormat;          // IDirectSoundBuffer_SetFormat(p,a)
/*+3C*/ IDirectSoundBuffer_SetVolume          SetVolume;          // IDirectSoundBuffer_SetVolume(p,a)
/*+40*/ IDirectSoundBuffer_SetPan             SetPan;             // IDirectSoundBuffer_SetPan(p,a)
/*+44*/ void*                                 SetFrequency;       // IDirectSoundBuffer_SetFrequency(p,a)
/*+48*/ IDirectSoundBuffer_Stop               Stop;               // IDirectSoundBuffer_Stop(p)
/*+4C*/ IDirectSoundBuffer_Unlock             Unlock;             // IDirectSoundBuffer_Unlock(p,a,b,c,d)
/*+50*/ IDirectSoundBuffer_Restore            Restore;            // IDirectSoundBuffer_Restore(p)
};

// Storm SFX Structure
typedef struct stormsfx_ stormsfx;
struct stormsfx_ {
/*00*/ stormsfx* prev;
/*04*/ stormsfx* next;
/*08*/ HANDLE hFile;
/*0C*/ u32 writePosition;
/*10*/ u32 byteRate;
/*14*/ u32 unk_14; // (flags >> 0x12) & 1
/*18*/ u32 state;
/*1C*/ u32 unk_1c; // ?
/*20*/ u32 filePtr;
/*24*/ u32 dataSize;
/*28*/ s32 volume;
/*2C*/ s32 panning;
/*30*/ IDirectSoundBuffer* dsb;
/*34*/ u32 bufferSize;
/*38*/ u32 silenceVal; // 0x80 if 8 bps, 0 if 16 bps
/*3C*/ u32 unk_3c; // 1
};

// StreamedSFX Structure
typedef struct streamedsfx_ streamedsfx;
struct streamedsfx_ {
/*00*/ streamedsfx* prev;
/*04*/ streamedsfx* next;
/*08*/ HANDLE hFile;
};


#ifdef __cplusplus
extern "C" { // so that it's linkable from bwl.c
#endif

void playOgg();
void oggCacheClear();

#ifdef __cplusplus
};
#endif


