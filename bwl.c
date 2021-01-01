// Because it doesn't exist, apparently
#ifndef __cplusplus
typedef enum {false, true} bool;
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "eudr.h"
#include "version.h"

#define BWLAPI 4
#define STARCRAFTBUILD 13 // 1.16.1

typedef struct{
  int iPluginAPI;
  int iStarCraftBuild;
  BOOL bNotSCBWmodule;
  BOOL bConfigDialog;
} ExchangeData;

char dllpath[1024]; // Full path to this plugin
char exepath[1024]; // Path to whatever has loaded the plugin

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
  int i;
  switch(ul_reason_for_call)
  {
   case DLL_PROCESS_ATTACH:
     GetModuleFileNameA(hModule, dllpath, sizeof(dllpath));
     GetModuleFileNameA(NULL, exepath, sizeof(exepath));
     for(i = strlen(exepath); i > 0 && exepath[i-1] != '\\'; i--);
     if(strcmpi(&exepath[i], "starcraft.exe") == 0){
       patch();
     }
     return true;
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
     break;
  }
  return true;
}

__declspec(dllexport) void GetPluginAPI(ExchangeData *Data){
  Data->iPluginAPI = BWLAPI;
  Data->iStarCraftBuild = STARCRAFTBUILD;
  Data->bConfigDialog = false;
  Data->bNotSCBWmodule = true;
}

__declspec(dllexport) void GetData(char *name, char *description, char *updateurl){
  strcpy(name, PLUGIN_NAME);
  strcpy(description, PLUGIN_BWL_DESC);
  strcpy(updateurl, "");
}

__declspec(dllexport) BOOL OpenConfig(){
  return true;
}

__declspec(dllexport) BOOL ApplyPatchSuspended(HANDLE hProcess, DWORD dwProcessID){
  // Loads this plugin .dll in to StarCraft.exe remotely
  HMODULE hKernel32 = GetModuleHandle("Kernel32");
  void* dllpathRemote = VirtualAllocEx(hProcess, NULL, sizeof(dllpath), MEM_COMMIT, PAGE_READWRITE);
  WriteProcessMemory(hProcess, dllpathRemote, (void*)dllpath, sizeof(dllpath), NULL);
  HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), dllpathRemote, 0, NULL);
  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hThread);
  VirtualFreeEx(hProcess, dllpathRemote, sizeof(dllpath), MEM_RELEASE);
  return true;
}

__declspec(dllexport) BOOL ApplyPatch(HANDLE hProcess, DWORD dwProcessID){
  return true;
}

