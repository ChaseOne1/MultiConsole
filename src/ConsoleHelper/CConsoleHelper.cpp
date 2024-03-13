#include <cstdio>
#include <regex>
#include "CConsoleHelper.hpp"

#ifdef DEBUG_CONSOLE
#include <iostream>
#endif

using namespace cs;

//Initialize the console
CConsoleHelper::CConsoleHelper(PCTSTR szPipeName)
{
    while (true) {
        //TODO: READ and WRITE the pipe
        m_hPipe = CreateFile(szPipeName, PIPE_ACCESS_INBOUND, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (m_hPipe != INVALID_HANDLE_VALUE) break;
        else if (GetLastError() != ERROR_PIPE_BUSY)
            MessageBox(NULL, TEXT("Could not open pipe(1)"), TEXT("FAILED"), MB_OK);
        else
            if (!WaitNamedPipe(szPipeName, 2000))
                MessageBox(NULL, TEXT("Could not open pipe(2)"), TEXT("FAILED"), MB_OK);
        Sleep(500);
    }

    m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(m_hConsole, &m_ConsoleBufferInfo)) {
        MessageBox(NULL, TEXT("GetConsoleScreenBufferInfo failed"), TEXT("FAILED"), MB_OK);
        return;
    }

    m_bInitilized = TRUE;
}

CConsoleHelper::~CConsoleHelper()
{
    if (m_hPipe == INVALID_HANDLE_VALUE || m_hPipe == NULL)  return;

    else DisconnectNamedPipe(m_hPipe);
    CloseHandle(m_hPipe);
}

BOOL CConsoleHelper::Process()
{
#ifdef DEBUG_CONSOLE
    std::cout << "[INFO]" << "Wait for command..." << std::endl;
#endif

    eConsoleCommand cmd = COMMAND_TOTAL;
    if (!ReadPipe(cmd)) return FALSE;

#ifdef DEBUG_CONSOLE
    std::cout << "[INFO]" << "COMMAND_ID: " << cmd << std::endl;
#endif

    switch (cmd) {
    case COMMAND_PRINT:             return OnPrint();
    case COMMAND_SET_PRINT_COLOR:   return OnColoredPrint();
    case COMMAND_CLEAR:             return OnClear();
    case COMMAND_COLORED_CLEAR:     return OnColoredClear();
    case COMMAND_SET_TITLE:         return OnSetTitle();
    case COMMAND_SET_BUFFER_SIZE:   return OnSetBufferSize();
    case COMMAND_GOTOYX:            return OnGotoYX();
    case COMMAND_SYSTEM:            return OnSystem();
    case COMMAND_REDIRECT:          return OnRedirect();
    case COMMAND_EXIT:              return FALSE;
    default:                        return OnDefault();
    }
}

BOOL CConsoleHelper::OnPrint()
{
    SIZE_T size = 0;    if (!ReadPipe(size))    return FALSE;
    PTSTR szText = reinterpret_cast<PTSTR>(new BYTE[size]);   ZeroMemory(szText, size);

    //The byte
    if (!ReadPipe(szText, size)) return FALSE;
    DWORD dwLengthWritten = 0;
    if (!WriteFile(m_hConsole, szText, size, &dwLengthWritten, NULL))
        return FALSE;

    delete[] szText;
    return TRUE;
}

BOOL CConsoleHelper::OnColoredPrint()
{
    SIZE_T size = 0;    if (!ReadPipe(size))    return FALSE;
    WORD attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;    if (!ReadPipe(attributes, size))  return FALSE;

    SetConsoleTextAttribute(m_hConsole, static_cast<WORD>(attributes));

    return TRUE;
}

BOOL CConsoleHelper::OnClear()
{
    system("cls");

    return TRUE;
}

BOOL CConsoleHelper::OnColoredClear()
{
    SIZE_T size = 0;    if (!ReadPipe(size))    return FALSE;
    WORD attributes = 0;    if (!ReadPipe(attributes, size))  return FALSE;
    static COORD coordScreen = { 0, 0 };   DWORD cCharsWritten = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;    ZeroMemory(&csbi, sizeof csbi);
    CONST DWORD dwConsoleSize = m_ConsoleBufferInfo.dwSize.X * m_ConsoleBufferInfo.dwSize.Y;

    SetConsoleTextAttribute(m_hConsole, attributes);
    FillConsoleOutputCharacter(m_hConsole, TCHAR(' '), dwConsoleSize, coordScreen, &cCharsWritten);
    GetConsoleScreenBufferInfo(m_hConsole, &csbi);
    FillConsoleOutputAttribute(m_hConsole, csbi.wAttributes, dwConsoleSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(m_hConsole, coordScreen);

    return TRUE;
}

BOOL CConsoleHelper::OnSetTitle()
{
    SIZE_T size = 0;    if (!ReadPipe(size)) return FALSE;
    PTSTR szTitle = reinterpret_cast<PTSTR>(new BYTE[size]);   ZeroMemory(szTitle, size);
    if (!ReadPipe(szTitle, size)) return FALSE;

    SetConsoleTitle(szTitle);

    delete[] szTitle;
    return TRUE;
}

BOOL CConsoleHelper::OnSetBufferSize()
{
    SIZE_T size = 0;    if (!ReadPipe(size)) return FALSE;
    COORD bufferSize = { 0 }; if (!ReadPipe(bufferSize, size)) return FALSE;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), bufferSize);

    return TRUE;
}

BOOL CConsoleHelper::OnGotoYX()
{
    SIZE_T size = 0;    if (!ReadPipe(size)) return FALSE;
    COORD nPosn = { 0 }; if (!ReadPipe(nPosn, size)) return FALSE;

    SetConsoleCursorPosition(m_hConsole, nPosn);    //Y is column, X is row.

    return TRUE;
}

BOOL CConsoleHelper::OnSystem()
{
    SIZE_T size = 0;    if (!ReadPipe(size)) return FALSE;
    PTSTR szCommand = reinterpret_cast<PTSTR>(new BYTE[size]);   ZeroMemory(szCommand, size);

    if (!ReadPipe(szCommand, size)) return FALSE;
    DWORD dwLengthRead = 0;
#ifdef UNICODE
    size /= 2;
    PSTR szACommand = new CHAR[size];   ZeroMemory(szACommand, size);
    WideCharToMultiByte(CP_ACP, NULL, szCommand, -1, szACommand, 0, NULL, NULL);
    system(szACommand);
    delete[] szACommand;
#else
    system(szCommand);
#endif

    delete[] szCommand;
    return TRUE;
}

/*  TIPS: When in this state, only output operations are supported, and the behavior using the command-driven approach is undefined.
    TIPS: The exit keyword is "COMMAND_REDIRECT".
*/
BOOL CConsoleHelper::OnRedirect()
{
    //TIPS:can read MAX_LENGTH -1 bytes, and set the szText[MAX_LENGTH - 1] = 0 to ensure the safe; 
    TCHAR szText[MAX_LENGTH] = { 0 }, *szSubStr = nullptr;
    while (true) {
        DWORD dwBytesRead = 0, dwTotalBytesAvail = 0, dwBytesLeftThisMessage = 0, dwLengthWritten = 0;
        if (!PeekNamedPipe(m_hPipe, szText, sizeof szText, &dwBytesRead, &dwTotalBytesAvail, &dwBytesLeftThisMessage))  return FALSE;
        if (szSubStr = _tcsstr(szText, TEXT("COMMAND_REDIRECT"))) {
            if (!ReadPipe(szText, dwBytesRead)) return FALSE;
            if (!WriteFile(m_hConsole, szText, dwBytesRead - sizeof TEXT("COMMAND_REDIRECT"), &dwLengthWritten, NULL))    return FALSE;
            break;
        }
        else {
            if (!ReadPipe(szText, dwBytesRead))   return FALSE;
            if (!WriteFile(m_hConsole, szText, dwBytesRead, &dwLengthWritten, NULL))    return FALSE;
        }
        ZeroMemory(szText, MAX_LENGTH);
    }
    return TRUE;
}

BOOL CConsoleHelper::OnDefault()
{
    return FALSE;
}
