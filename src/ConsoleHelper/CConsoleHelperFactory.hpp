#pragma once
#include <dbghelp.h>

class CConsoleHelperFactory
{
private:
    inline static bool ms_bRsrcHasLoaded = false;

    CConsoleHelperFactory() = default;
    ~CConsoleHelperFactory() = default;

    static void UnmapDummyMemory(HANDLE handle, PVOID pImageBase);
    inline static LPVOID LoadExecResource();
    inline static void GetNTHeaders(DWORD pImageBase, _Out_ PIMAGE_NT_HEADERS32 &pHeaders);
    inline static  void GetLoadedImage(DWORD dwImageBase, _Out_ PLOADED_IMAGE pLoadedImage);
    static void WriteSectionAligned(HANDLE handle, DWORD dwImageBase, PVOID pRsrcExec, PLOADED_IMAGE pSourceImage);
    static void Relocate(HANDLE handle, DWORD dwImageBase, PBYTE pRsrcExec, PLOADED_IMAGE pSourceImage, PIMAGE_NT_HEADERS32 pSourceHeaders, DWORD dwDelta);
public:
    //function about to create
    static void Create(PTSTR szCmdline);
};