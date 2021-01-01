#include <stdio.h>
#include <vorbis/codec.h>
#include <windows.h>
#include "eudr.h"
#include "ogg2wav.h"

// Maximum number of files on disk at a time
#define OGG_CACHE_MAX 64

asm(".intel_syntax noprefix\n");

struct {
  u8 srcname[260];      // map string name -- instead hash this or use a string map?
  u8 wavname[32];       // cached filename
  u32 next,prev;        // to keep track of oldest/newest
  u32 playCount;        // keep frequently-used files fresh
} oggcache[OGG_CACHE_MAX] = {0};

u32 oggCacheHead = 0; // Most referenced file
u32 oggCacheTail = 0; // Least referenced file

IDirectSound** dsi = (IDirectSound**)(0x006D59F4); // DirectSound Interface pointer
u32* sfx_volume = (u32*)(0x006CDFE4); // Sound options -> SFX volume
s32* volume_lut = (s32*)(0x005008F0); // DirectSound volume levels, in dB or something

void oggCacheInit();
u32 oggCacheFind(u8* name);
void updateCache(u32 id);

HANDLE StreamedSFX_Constructor(u8* name);
u32 EUDDdaBeginEx(HANDLE hFile, u32 bufferSize, u32 flags, u32 dataOffset, s32 volume, s32 pan, u32 unk);
__fastcall stormsfx* createStormSfx(u32 arg1, u32 arg2, u32 arg3, u32 arg4);

u32 ogg2wav(HANDLE file, char* outname);
u32 oggDecode(char* outname, oggdecdata* ogg, HANDLE file);


void playOgg(/*HANDLE hFile, u8* name*/){ // EAX = hFile, ESI = u8 name[260]
  HANDLE hFile;
  u8* name;
  __asm __volatile("" : "=a"(hFile),"=S"(name)); // get arguments from eax and esi
  
  
  u32 id = oggCacheFind(name);
  if(oggcache[id].srcname[0] == 0){
    // load file
    if(oggcache[id].wavname[0] == 0){
      //tmpnam(oggcache[id].wavname);
      sprintf(oggcache[id].wavname, "ogg%02d.wav", id);
    }
    if(ogg2wav(hFile, oggcache[id].wavname) == 0){
      SFileCloseFile(hFile);
      return;
    }
    strcpy(oggcache[id].srcname, name);
  }
  SFileCloseFile(hFile);
  
  hFile = StreamedSFX_Constructor(oggcache[id].wavname);
  if(hFile != NULL){
    updateCache(id); // increment playCount
  }
  return;
}


// Removes cached files
void oggCacheClear(){
  int i;
  for(i = 0; i < OGG_CACHE_MAX; i++){
    if(oggcache[i].wavname[0] != 0){
      remove(oggcache[i].wavname);
      oggcache[i].wavname[0] = 0;
    }
  }
  oggCacheInit();
}



/* ---- OGG Cache Management ---- */

// Prepares the cache structure
void oggCacheInit(){
  int i;
  for(i = 0; i < OGG_CACHE_MAX; i++){
    oggcache[i].srcname[0] = 0;
    oggcache[i].wavname[0] = 0;
    oggcache[i].next = i+1;
    oggcache[i].prev = (i==0)?OGG_CACHE_MAX:i-1; // since not a real linked list, use a known value instead of NULL
    oggcache[i].playCount = 0;
  }
  oggCacheHead = 0;
  oggCacheTail = OGG_CACHE_MAX-1;
}


// Returns index of matched file, or an empty entry if not found
u32 oggCacheFind(u8* name){
  u32 i;
  for(i = oggCacheHead; i != OGG_CACHE_MAX; i = oggcache[i].next){
    if(oggcache[i].srcname[0] == 0) return i;
    if(strcmpi(oggcache[i].srcname, name) == 0) return i;
    if(oggcache[i].next == OGG_CACHE_MAX){ // last entry -- cache is full, so create an empty entry
      oggcache[i].srcname[0] = 0; // clear string
      oggcache[i].playCount = 0; // reset play count
      return i;
    }
  }
  return OGG_CACHE_MAX; // something broke ???
}


// Increments playCount, and repositions the entry so that the list is sorted by descending playCount and ascending age (oldest & least played sounds put towards the end)
void updateCache(u32 id){
  u32 i, p,n;
  oggcache[id].playCount++;
  
  if(oggCacheHead == id) return; // can't move any higher than the top
  
  // just to make it easier to read without a bunch of nested array indexes
  p = oggcache[id].prev;
  n = oggcache[id].next;
  
  if(oggcache[p].playCount > oggcache[id].playCount) return; // list is still in order; don't need to move
  
  // remove from the list
  if(p != OGG_CACHE_MAX) oggcache[p].next = n;
  if(n != OGG_CACHE_MAX) oggcache[n].prev = p;
  if(oggCacheTail == id) oggCacheTail = p;
  
  // find insertion point -- any file with a greater play count should come before, but any with the same or fewer should come after
  for(i = oggCacheHead; i != OGG_CACHE_MAX && oggcache[i].playCount > oggcache[id].playCount; i = oggcache[i].next);
  
  // insert before whichever was found
  if(i == OGG_CACHE_MAX){ // put as the new tail
    oggcache[oggCacheTail].next = id;
    oggcache[id].prev = oggCacheTail;
    oggcache[id].next = OGG_CACHE_MAX;
    oggCacheTail = id;
  }else{ // put somewhere in the middle
    p = oggcache[i].prev;
    oggcache[id].prev = p;
    oggcache[id].next = i;
    oggcache[i].prev = id;
    if(p == OGG_CACHE_MAX){ // new head
      oggCacheHead = id;
    }else{
      oggcache[p].next = id;
    }
  }
}




/* ---- Sound Playback Functions ---- */

// Copy of StreamedSFX_Constructor (0x004BC490) that uses SFileOpenFileEx and calls a modified SFileDdaBeginEx to read from disk files
HANDLE StreamedSFX_Constructor(u8* name){
  HANDLE hFile = NULL; // ebp-4
  
  if(*dsi == NULL) return NULL;
  if(*sfx_volume == 0) return NULL;
  
  if(SFileOpenFileEx(NULL, name, 3, &hFile) == 0 || hFile == NULL){
  //if(SFileOpenFile(name, &hFile) == 0){
    //if(SErrGetLastError() == 2 || SErrGetLastError() == 0x3EE) return 0;
    //SysWarn_FileNotFound(name, SErrGetLastError())
    return NULL;
  }
  
  if(EUDDdaBeginEx(hFile, 0x40000, 0, 0, volume_lut[*sfx_volume*99/100], 0, 0) == 0){
    SFileCloseFile(hFile);
    return NULL;
  }
  
  // Messy linked list junk beyond this point
  streamedsfx* ssfx = SMemAlloc(12, (char*)(0x0051A424+8), -2, 8);
  streamedsfx *ebx, *edx, *ecx, *esi;
  if(ssfx != NULL){
    ssfx->prev = NULL;
    ssfx->next = NULL;
    ebx = ssfx;
  }else{
    ebx = NULL;
    ssfx = (streamedsfx*)(0x0051A20C);
  }
  
  if(ssfx->prev != NULL){
    edx = ssfx->next;
    if((s32)(ssfx->next) <= 0){
      edx = (streamedsfx*)(~(u32)edx);
    }else{
      edx = (streamedsfx*)((u32)(ssfx->next) + (u32)ssfx - (u32)(ssfx->prev->next));
    }
    edx->prev = ssfx->prev;
    ssfx->prev->next = ssfx->next;
    ssfx->prev = NULL;
    ssfx->next = NULL;
  }
  
  ssfx->prev = (streamedsfx*)(0x0051A20C);
  ssfx->next = *(streamedsfx**)(0x0051A210);
  
  edx = *(streamedsfx**)(0x0051A210);
  ecx = *(streamedsfx**)(0x0051A208);
  if((s32)edx <= 0){
    edx = (streamedsfx*)(~(u32)edx);;
  }else{
    if((s32)ecx < 0){
      esi = *(streamedsfx**)(0x0051A20C);
      ecx = (streamedsfx*)(0x0051A20C - (u32)(esi->next));
    }
    edx = (streamedsfx*)((u32)edx + (u32)ecx);
  }
  edx->prev = ssfx;
  
  *(streamedsfx**)(0x0051A210) = ebx;
  ebx->hFile = hFile; // EBX can be null ???
  return hFile;
}


// Custom replacement of SFileDdaBeginEx utilizing SFileReadFile instead of whatever it does normally
u32 EUDDdaBeginEx(HANDLE hFile, u32 bufferSize, u32 flags, u32 dataOffset, s32 volume, s32 pan, u32 unk){
  wavehdr wav;
  DSBUFFERDESC dsbd;
  u32 read;
  IDirectSoundBuffer* dsb = NULL;
  stormsfx* sfx;
  
  if(bufferSize == 0){
    //SetLastError(0x57);
    return 0;
  }
  if(*dsi == NULL){
    //SetLastError(0x85100071);
    return 0;
  }
  if(hFile == NULL){
    // no file
    return 0;
  }
  
  if(bufferSize & 0x3FFF){
    bufferSize += 0x4000 - (bufferSize & 0x3FFFF);
  }
  if(bufferSize < 0x8000){
    bufferSize = 0x8000;
  }
  
  if(SFileReadFile(hFile, &wav, sizeof(wav), &read, 0) == 0 || read != sizeof(wav)){
    // error reading file
    return 0;
  }
  
  // Super unsafe and low-effort wav format support -- works because we wrote the file and should know exactly what it is already
  if(wav.riffID != WAVE_RIFF || wav.waveID != WAVE_WAVE || wav.fmtID != WAVE_fmt || wav.fmtSize < 16 || wav.format != 1 || wav.dataID != WAVE_data){
    // unsupported format
    return 0;
  }
  
  dsbd.dwSize = 20;
  dsbd.dwFlags = 0x80;
  if(flags & 0x20) dsbd.dwFlags |= 0x20;
  if(flags & 0x40) dsbd.dwFlags |= 0x40;
  dsbd.dwBufferBytes = bufferSize;
  dsbd.dwReserved = 0;
  dsbd.lpwfxFormat = (void*)&wav.format; // start of WAVEFORMATEX in wav header
  
  (**dsi)->CreateSoundBuffer(*dsi, &dsbd, &dsb, 0);
  if(dsb == NULL) return 0;
  
  if(volume != 0 && volume != 0x7FFFFFFF){
    (*dsb)->SetVolume(dsb, volume);
  }
  
  if(pan != 0 && pan != 0x7FFFFFFF){
    (*dsb)->SetPan(dsb, pan);
  }
  
  if(dataOffset != 0){
    SFileSetFilePointer(hFile, dataOffset, 0, SEEK_CUR);
    // I think that's everything?
  }
  
  EnterCriticalSection((LPCRITICAL_SECTION)(0x1505ED54));
  sfx = createStormSfx(0, 2, 0x1505AE28, 0);
  
  sfx->bufferSize = bufferSize;
  sfx->hFile = hFile;
  sfx->byteRate = wav.byteRate;
  sfx->unk_14 = (flags >> 0x12) & 1;
  sfx->state = 0;
  if(dataOffset < wav.dataSize - sizeof(wavehdr)){ // check ?
    sfx->filePtr = dataOffset + sizeof(wavehdr);
  }else{
    sfx->filePtr = wav.dataSize;
  }
  sfx->volume = volume;
  sfx->panning = pan;
  sfx->dataSize = wav.riffSize;
  sfx->dsb = dsb;
  sfx->unk_3c = 1;
  sfx->silenceVal = (wav.bitsPerSample == 8) ? 0x80 : 0; // 0x80 if 8 bits per sample, 0 otherwise
  
  // some hFile internals ... who knows
  (*(u32*)(hFile + 0x140))++;
  *(u32*)(hFile + 0x130) = 1;
  *(u32*)(hFile + 0x11C) = sfx->filePtr;
  
  LeaveCriticalSection((LPCRITICAL_SECTION)(0x1505ED54));
  SetEvent(*(HANDLE*)(0x1505E5F0));
  
  return 1;
}

// Creates an sfx object, inserts it in an internal list, and returns the object pointer
__fastcall stormsfx* __attribute__ ((noinline)) createStormSfx(u32 arg1, u32 arg2, u32 arg3, u32 arg4){
  stormsfx* out;
  __asm __volatile("\n"
"    push edi\n"    // arg4 -> ebp+c
"    push eax\n"    // arg3 -> ebp+8
"    mov edi,edx\n" // arg2 -> edi
"    mov eax,ecx\n" // arg1 -> eax
"    mov edx,0x150045C0\n" // function pointer
"    call edx\n"
  : "=a"(out)
  : "a"(arg3),"D"(arg4) // pass arg3 and arg4 from stack to eax and edi; arg1 and arg2 are in ecx and edx, respectively
  : "edx" // clobbers edx -- ebx and esi should be untouched
);
  return out;
}



/* ---- OGG Decoding ---- */
// Decoder code adapted from: https://github.com/xiph/vorbis/blob/master/examples/decoder_example.c

#define CONV_BUFFER_SIZE 4096
ogg_int16_t convbuffer[CONV_BUFFER_SIZE];
int convsize = CONV_BUFFER_SIZE;

// Initializes ogg conversion. Returns 1 on success and 0 on failure.
u32 ogg2wav(HANDLE file, char* outname){
  oggdecdata ogg;
  
  char *buffer;
  int  bytes;
  
  u32 success = 0;
  
  if(file == NULL) return 0;
  
  SFileSetFilePointer(file, 0, NULL, SEEK_SET); // reset to start of file
  
  ogg_sync_init(&ogg.oy);
  
  buffer = ogg_sync_buffer(&ogg.oy, CONV_BUFFER_SIZE);
  SFileReadFile(file, buffer, CONV_BUFFER_SIZE, &bytes, 0);
  ogg_sync_wrote(&ogg.oy, bytes);
  
  if(ogg_sync_pageout(&ogg.oy, &ogg.og) == 1){
    ogg_stream_init(&ogg.os, ogg_page_serialno(&ogg.og));
    vorbis_info_init(&ogg.vi);
    vorbis_comment_init(&ogg.vc);
    
    if(ogg_stream_pagein(&ogg.os, &ogg.og) >= 0 &&
       ogg_stream_packetout(&ogg.os, &ogg.op) == 1 &&
       vorbis_synthesis_headerin(&ogg.vi, &ogg.vc, &ogg.op) >= 0){
      success = oggDecode(outname, &ogg, file);
    }
    
    ogg_stream_clear(&ogg.os);
    vorbis_comment_clear(&ogg.vc);
    vorbis_info_clear(&ogg.vi);  /* must be called last */
  }
  
  ogg_sync_clear(&ogg.oy);
  
  return success;
}

// Decoder inner function. Returns 1 on success and 0 on failure.
u32 oggDecode(char* outname, oggdecdata* ogg, HANDLE file){
  char *buffer;
  int bytes;
  int eos=0;
  int i;
  
  u32 dataSize = 0;
  
  FILE* fWave = NULL;
  wavehdr wav;
  wav.riffID = WAVE_RIFF;
  wav.waveID = WAVE_WAVE;
  wav.fmtID = WAVE_fmt;
  wav.fmtSize = 16; // sizeof fmt chunk
  wav.format = 1; // PCM
  wav.bitsPerSample = 16;
  wav.dataID = WAVE_data;
  
  i=0;
  while(i<2){
    while(i<2){
      int result=ogg_sync_pageout(&ogg->oy,&ogg->og);
      if(result==0)break;
      if(result==1){
        ogg_stream_pagein(&ogg->os,&ogg->og);
        while(i<2){
          result=ogg_stream_packetout(&ogg->os,&ogg->op);
          if(result==0)break;
          if(result<0){
            return 0;
          }
          result=vorbis_synthesis_headerin(&ogg->vi,&ogg->vc,&ogg->op);
          if(result<0){
            return 0;
          }
          i++;
        }
      }
    }
    buffer=ogg_sync_buffer(&ogg->oy,CONV_BUFFER_SIZE);
    SFileReadFile(file, buffer, CONV_BUFFER_SIZE, &bytes, 0);
    if(bytes==0 && i<2){
      return 0;
    }
    ogg_sync_wrote(&ogg->oy,bytes);
  }
  
  wav.channels = ogg->vi.channels;
  wav.sampleRate = ogg->vi.rate;
  wav.byteRate = ogg->vi.rate * ogg->vi.channels * 2;
  wav.blockAlign = ogg->vi.channels * 2;
  fWave = fopen(outname, "wb");
  if(fWave == NULL){
    return 0;
  }
  fwrite(&wav, 1, sizeof(wav), fWave);
  
  convsize = CONV_BUFFER_SIZE/ogg->vi.channels;
  
  if(vorbis_synthesis_init(&ogg->vd,&ogg->vi)==0){
    vorbis_block_init(&ogg->vd,&ogg->vb);
    while(!eos){
      while(!eos){
        int result=ogg_sync_pageout(&ogg->oy,&ogg->og);
        if(result==0) break;
        if(result<0){
          // continuing
        }else{
          ogg_stream_pagein(&ogg->os,&ogg->og);
          while(1){
            result=ogg_stream_packetout(&ogg->os,&ogg->op);
            
            if(result==0) break;
            if(result<0){
            }else{
              float **pcm;
              int samples;
              
              if(vorbis_synthesis(&ogg->vb,&ogg->op)==0)
                vorbis_synthesis_blockin(&ogg->vd,&ogg->vb);
              
              while((samples=vorbis_synthesis_pcmout(&ogg->vd,&pcm))>0){
                int j;
                int bout=(samples<convsize?samples:convsize);
                
                for(i=0;i<ogg->vi.channels;i++){
                  ogg_int16_t *ptr=convbuffer+i;
                  float  *mono=pcm[i];
                  for(j=0;j<bout;j++){
#if 1
                    int val=floor(mono[j]*32767.f+.5f);
#else /* optional dither */
                    int val=mono[j]*32767.f+drand48()-0.5f;
#endif
                    if(val>32767) val=32767;
                    if(val<-32768) val=-32768;
                    *ptr=val;
                    ptr+=ogg->vi.channels;
                  }
                }

                fwrite(convbuffer, 2*ogg->vi.channels, bout, fWave);
                
                dataSize += 2*ogg->vi.channels * bout;
                
                vorbis_synthesis_read(&ogg->vd,bout);
              }
            }
          }
          if(ogg_page_eos(&ogg->og))eos=1;
        }
      }
      if(!eos){
        buffer=ogg_sync_buffer(&ogg->oy,CONV_BUFFER_SIZE);
        SFileReadFile(file, buffer, CONV_BUFFER_SIZE, &bytes, 0);
        ogg_sync_wrote(&ogg->oy,bytes);
        if(bytes==0)eos=1;
      }
    }
    
    vorbis_block_clear(&ogg->vb);
    vorbis_dsp_clear(&ogg->vd);
  }else{
    // error state
    fclose(fWave);
    return 0;
  }
  
  wav.riffSize = sizeof(wavehdr) + dataSize;
  wav.dataSize = dataSize;
  
  fseek(fWave, 4, SEEK_SET); // seek to riffSize
  fwrite(&wav.riffSize, 1, sizeof(u32), fWave);
  
  fseek(fWave, 40, SEEK_SET); // seek to dataSize
  fwrite(&wav.dataSize, 1, sizeof(u32), fWave);
  
  fclose(fWave);
  return 1;
}


