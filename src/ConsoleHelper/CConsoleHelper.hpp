#pragma once

class CConsoleHelper
{

private:
    HANDLE	m_hConsole = INVALID_HANDLE_VALUE;
    CONSOLE_SCREEN_BUFFER_INFO	m_ConsoleBufferInfo;
    HANDLE	m_hPipe;

public:
    BOOL    m_bInitilized = FALSE;

    CConsoleHelper(PCTSTR szPipeName);
    CConsoleHelper(const CConsoleHelper &) = delete;
    CConsoleHelper(CConsoleHelper &&) = delete;
    CConsoleHelper &operator=(const CConsoleHelper &) = delete;
    CConsoleHelper &operator=(CConsoleHelper &&) = delete;

    ~CConsoleHelper();

    BOOL Process();
    //bool ProcessEx();

    //Console Functions
    BOOL OnPrint();
    BOOL OnColoredPrint();
    BOOL OnClear();
    BOOL OnColoredClear();
    BOOL OnSetTitle();
    BOOL OnSetBufferSize();
    BOOL OnGotoYX();
    BOOL OnSystem();
    BOOL OnRedirect();
    BOOL OnDefault();

    template<typename T>
    BOOL ReadPipe(T &val, size_t bytes = sizeof(T))
    {
        DWORD dwRead = 0;
        if constexpr (std::is_pointer_v<T>)
            return ReadFile(m_hPipe, val, bytes, &dwRead, NULL);
        else
            return ReadFile(m_hPipe, &val, bytes, &dwRead, NULL);
    }
};
