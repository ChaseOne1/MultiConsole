#include <iostream>
#include <io.h>
#include <Fcntl.h>
#include "CConsole.hpp"
#include "..\ConsoleHelper\CConsoleHelperFactory.hpp"

using namespace std;
namespace cs
{
    CConsole::CConsole(PCTSTR szConsoleTitle, SHORT buffer_size_x, SHORT buffer_size_y)
    {
        InitializeCriticalSection();

        //Initialize some flags
        m_nConsoleFlags.bCommanding = false;
        m_nConsoleFlags.bRedirecting = false;

        TCHAR logger_name[MAX_PATH];
#ifdef DEBUG_CONSOLE
        _stprintf_s(logger_name, TEXT("\\\\.\\pipe\\LOGGER_DEBUG"));
#else
        _stprintf_s(logger_name, TEXT("\\\\.\\pipe\\%lu"), GetTickCount());
#endif
        m_hPipe = CreateNamedPipe(
            logger_name,
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 4096, 0, 1, NULL);
        if (m_hPipe == INVALID_HANDLE_VALUE)
            MessageBox(NULL, TEXT("CreateNamedPipe failed"), TEXT("ConsoleLogger failed"), MB_OK);

        STARTUPINFO si = { 0 }; PROCESS_INFORMATION pi = { 0 }; GetStartupInfo(&si);
        TCHAR cmdline[MAX_PATH];    _stprintf_s(cmdline, TEXT("cmd.exe %s"), logger_name);
#ifdef DEBUG_CONSOLE
        cout << "[INFO]" << "cmdline : " << cmdline << std::endl;
#else 
        //Created by us in debug mode
        CConsoleHelperFactory::Create(cmdline);
#endif
        BOOL bConnected = ConnectNamedPipe(m_hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!bConnected) {
            MessageBox(NULL, TEXT("ConnectNamedPipe failed"), TEXT("ConsoleLogger failed"), MB_OK);
            CloseHandle(m_hPipe); m_hPipe = INVALID_HANDLE_VALUE;
        }

        //set titel
        if (_tcslen(szConsoleTitle))
            SendCommand(COMMAND_SET_TITLE, szConsoleTitle, CALC_TSTR_BYTES(szConsoleTitle));
        else
            SendCommand(COMMAND_SET_TITLE, TEXT("ConsoleLogger"));
        //set buffer size
        if (buffer_size_x != -1 && buffer_size_y != -1) {
            COORD buffer_size{ buffer_size_x, buffer_size_y };
            SendCommand(COMMAND_SET_BUFFER_SIZE, &buffer_size);
        }
    }

    CConsole::~CConsole()
    {
        DeleteCriticalSection();

        if (m_hPipe == INVALID_HANDLE_VALUE || m_hPipe == NULL) return;
        else {
            SendCommand(COMMAND_EXIT);
            DisconnectNamedPipe(m_hPipe);
        }
    }

    static int gs_originStdout = _dup(_fileno(stdout));

    void CConsole::SetAsDefaultOutput()
    {
        SendCommand(COMMAND_REDIRECT);

        int hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(m_hPipe), _O_TEXT);
        _dup2(hConHandle, _fileno(stdout));
        setvbuf(stdout, NULL, _IONBF, 0);

        m_nConsoleFlags.bRedirecting = true;
    }

    void CConsole::ResetAsDefaultOutput()
    {
        WritePipe(TEXT("COMMAND_REDIRECT"));  //exit keyword

        _dup2(gs_originStdout, _fileno(stdout));
        setvbuf(stdout, NULL, _IONBF, 0);

        m_nConsoleFlags.bRedirecting = false;
    }

    ofstream CConsole::GetAsOfstream()
    {
        SendCommand(COMMAND_REDIRECT);

        int hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(m_hPipe), _O_TEXT);

        m_nConsoleFlags.bRedirecting = true;

        return std::ofstream(_fdopen(hConHandle, "w"));
    }

    void CConsole::CancelAsOfstream()
    {
        WritePipe(TEXT("COMMAND_REDIRECT"));  //exit keyword
        m_nConsoleFlags.bRedirecting = false;
    }

    int CConsole::printf(const TCHAR *format, ...)
    {
        va_list argList;    va_start(argList, format);
        TCHAR buffer[MAX_LENGTH] = { 0 };   int ret = 0;
        ret = _vsntprintf_s(buffer, sizeof(buffer), format, argList);
        va_end(argList);

        if (m_nConsoleFlags.bRedirecting) {
            WritePipe(buffer, CALC_TSTR_BYTES(buffer));
        }
        else {
            SendCommand(COMMAND_PRINT, buffer, CALC_TSTR_BYTES(buffer));
        }

        return ret;
    }
}