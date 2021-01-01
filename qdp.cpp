#include "MPQDraftPlugin.h"
#include "eudr.h"
#include "version.h"

const char* plugin_name = PLUGIN_NAME;

// Can't just GetModuleHandle. Thanks, FireGraft.
HINSTANCE GetMyModuleHandle() {
  MEMORY_BASIC_INFORMATION mbi;
  VirtualQuery((void*)GetMyModuleHandle, &mbi, sizeof(mbi));
  return (HINSTANCE) (mbi.AllocationBase);
}


class MPQDraftPluginInterface : public IMPQDraftPlugin {
  HINSTANCE hInstance;
  public:
  BOOL WINAPI Identify(LPDWORD pluginID) {
    if(!pluginID) {
      return false;
    }
    *pluginID = PLUGIN_QDP_ID;
    return true;
  }
  BOOL WINAPI GetPluginName(LPSTR pPluginName,DWORD namebufferlength) {
    if(!pPluginName) {
      return false;
    }
    if(namebufferlength < strlen(plugin_name)) {
      return false;
    }
    strcpy(pPluginName,plugin_name);
    return true;
  }
  BOOL WINAPI CanPatchExecutable(LPCSTR exefilename) {
    // Check version
    DWORD dwDummy;
    DWORD dwFVISize = GetFileVersionInfoSize((char*)exefilename, &dwDummy);
    LPBYTE lpVersionInfo = new BYTE[dwFVISize];
    GetFileVersionInfo((char*)exefilename, 0, dwFVISize, lpVersionInfo);
    UINT uLen;
    VS_FIXEDFILEINFO *lpFfi;
    VerQueryValue(lpVersionInfo, "\\", (LPVOID *)&lpFfi, &uLen);
    long VerHigh = lpFfi->dwProductVersionMS;
    long VerLow = lpFfi->dwProductVersionLS;
    delete [] lpVersionInfo;
    
    // Only supports one version (1.16.1), currently
    if(VerHigh == 0x00010010 && VerLow == 0x00010001) return true;
    
    // Unsupported version
    return false;
  }
  BOOL WINAPI Configure(HWND parentwindow) {
    return true;
  }
  BOOL WINAPI ReadyForPatch() {
    return true;
  }
  BOOL WINAPI GetModules(MPQDRAFTPLUGINMODULE* pluginmodules,LPDWORD nummodules) {
    if (!nummodules) {
      return false;
    }
    *nummodules = 0;
    return true;
  }
  BOOL WINAPI InitializePlugin(IMPQDraftServer* server) {
    return patch(); // eudr patch function
  }
  BOOL WINAPI TerminatePlugin() {
    return true;
  }
  void WINAPI SetInstance(HINSTANCE hInst) {
    hInstance = hInst;
  }
};

MPQDraftPluginInterface thePluginInterface;

BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
      thePluginInterface.SetInstance(hInstance);
      break;
      
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

BOOL WINAPI GetMPQDraftPlugin(IMPQDraftPlugin **lppMPQDraftPlugin) {
  *lppMPQDraftPlugin = &thePluginInterface;
  return TRUE;
}


