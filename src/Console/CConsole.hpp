#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <tchar.h>

namespace cs
{
#define CALC_TSTR_BYTES(str) ((_tcsclen(str) + 1) * sizeof(TCHAR))
#define CALC_WSTR_BYTES(str) ((wcslen(str) + 1) * sizeof(WCHAR))
#define CALC_STR_BYTES(str) ((strlen(str) + 1) * sizeof(CHAR))
#ifdef UNICODE
#define TSTRING std::wstring
#define TSTRING_VIEW std::wstring_view
#define TSTRINGSTREAM std::wstringstream
#else
#define TSTRING std::string
#define TSTRING_VIEW std::string_view
#define TSTRINGSTREAM std::stringstream
#endif

#ifndef CONSOLE_COMMANDS
#define CONSOLE_COMMANDS
    enum eConsoleCommand : DWORD
    {
        COMMAND_PRINT,
        COMMAND_SET_PRINT_COLOR,
        COMMAND_CLEAR,
        COMMAND_COLORED_CLEAR,
        COMMAND_SET_TITLE,
        COMMAND_GOTOXY,
        COMMAND_SET_BUFFER_SIZE,
        COMMAND_SYSTEM,
        COMMAND_EXIT,
        COMMAND_TOTAL
    };
#endif

    class CConsole
    {
    private:
        HANDLE	m_hPipe = INVALID_HANDLE_VALUE;

        volatile long m_fast_critical_section;

        struct
        {
            DWORD bCommanding : 1;
            DWORD bRedirecting : 1;
        }m_nConsoleFlags;

        CConsole(const CConsole &) = delete;
        CConsole(CConsole &&) = delete;
        CConsole &operator=(const CConsole &) = delete;
        CConsole &operator=(CConsole &&) = delete;

        void InitializeCriticalSection(void)
        {
            m_fast_critical_section = 0;
        }

        void DeleteCriticalSection(void)
        {
            m_fast_critical_section = 0;
        }

        // our own LOCK function
        void EnterCriticalSection(void)
        {
            while (InterlockedCompareExchange(&m_fast_critical_section, 1, 0) != 0) Sleep(0);
        }

        // our own UNLOCK function
        void LeaveCriticalSection(void)
        {
            m_fast_critical_section = 0;
        }

        BOOL SafeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                           LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
        {
            EnterCriticalSection();
            BOOL bRet = ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
            LeaveCriticalSection();

            return bRet;
        }

        template <typename T>
        BOOL WritePipe(CONST T &val, SIZE_T bytes = sizeof(T))
        {
            DWORD dwWritten = 0;
            if constexpr (std::is_pointer_v<T>)
                return SafeWriteFile(m_hPipe, val, bytes, &dwWritten, NULL);
            else
                return SafeWriteFile(m_hPipe, &val, bytes, &dwWritten, NULL);
        }

    public:
        CConsole(PCTSTR szConsoleTitle = TEXT(""), SHORT buffer_size_x = -1, SHORT buffer_size_y = -1);
        CConsole(CONST TSTRING_VIEW &szConsoleTitle, SHORT buffer_size_x = -1, SHORT buffer_size_y = -1)
            :CConsole(szConsoleTitle.data(), buffer_size_x, buffer_size_y) {}
        ~CConsole();

        template <typename T = std::nullptr_t>
        void SendCommand(CONST eConsoleCommand cmd, CONST T &parameter = nullptr, SIZE_T parmBytes = sizeof(T))
        {
            //send command
            WritePipe(cmd);
            if constexpr (std::is_null_pointer_v<T>)   return;
            //send bytes of parameter
            WritePipe(parmBytes);
            //send parameter
            WritePipe(parameter, parmBytes);
        }

        template<typename Arg>
        CConsole &operator<<(Arg &&arg);

        int printf(CONST TCHAR *format, ...);

        void SetAsDefaultOutput();
        void ResetAsDefaultOutput();

        std::ofstream GetAsOfstream();
        void CancelAsOfstream();
    };


    template<typename Arg>
    inline CConsole &CConsole::operator<<(Arg &&arg)
    {
        if constexpr (std::is_same_v<std::remove_reference_t<Arg>, eConsoleCommand>) {
            m_nConsoleFlags.bCommanding = true;
            SendCommand(arg);

            return *this;
        }

        static TSTRINGSTREAM strstream{ };  static TSTRING text{ };
        if (m_nConsoleFlags.bCommanding) {
            if constexpr (std::is_convertible_v<Arg, TSTRING_VIEW>) {
                strstream << std::forward<Arg>(arg);    text.assign(strstream.str());

                WritePipe(CALC_TSTR_BYTES(text.c_str()));
                WritePipe(text.c_str(), CALC_TSTR_BYTES(text.c_str()));

                strstream.str("");    strstream.clear();
            }
            else {
                WritePipe(static_cast<SIZE_T>(sizeof(Arg)));
                WritePipe(arg);
            }
        }
        else if constexpr (std::is_convertible_v<Arg, TSTRING_VIEW> || std::is_scalar_v<Arg>) {
            strstream << std::forward<Arg>(arg);    text.assign(strstream.str());

            if (m_nConsoleFlags.bRedirecting) {
                WritePipe(text.c_str(), CALC_TSTR_BYTES(text.c_str()));
            }
            else {
                SendCommand(COMMAND_PRINT, text.c_str(), CALC_TSTR_BYTES(text.c_str()));
            }


            strstream.str("");    strstream.clear();
        }

        return *this;
    }

    template<>
    inline CConsole &CConsole::operator<<(CConsole &(&Pfn)(CConsole &))
    {
        m_nConsoleFlags.bCommanding = false;

        (*Pfn)(*this);

        return *this;
    }

    inline CConsole &endl(CConsole &console)
    {
        console << "\n";

        return console;
    }

    inline CConsole &ends(CConsole &console)
    {
        console << "\0";

        return console;
    }
}