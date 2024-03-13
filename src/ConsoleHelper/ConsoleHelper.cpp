#include <iostream>
#include "CConsoleHelper.hpp"

using namespace std;

int _tmain(INT argc, PTSTR argv[])
{
#ifndef DEBUG_CONSOLE
    //The correct character type ensures that the index is correct
    if (argc == 1 || !argv[1] || !argv[1][0]) {
        MessageBox(NULL, TEXT("\nFailed to start console\n"), TEXT("FAILED"), MB_OK);
        return -1;
    }
    CConsoleHelper *consoleHelper = new CConsoleHelper(argv[1]);
#else
    CConsoleHelper *consoleHelper = new CConsoleHelper(TEXT("\\\\.\\pipe\\LOGGER_DEBUG"));
#endif
    if (consoleHelper->m_bInitilized)   while (consoleHelper->Process());
    else    MessageBox(NULL, TEXT("\nFailed to start console\n"), TEXT("FAILED"), MB_OK);
    cout << "Console has terminated." << endl;
    delete consoleHelper;

#ifdef DEBUG
    getchar();
#elif defined(DEBUG_CONSOLE)
    getchar();
#endif

    return 0;
}