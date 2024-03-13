#pragma once
#include <windows.h>

#define CONSOLE_COMMANDS

namespace cs
{
    enum eConsoleCommand : DWORD
    {
        COMMAND_PRINT,
        COMMAND_SET_PRINT_COLOR,
        COMMAND_CLEAR,
        COMMAND_COLORED_CLEAR,
        COMMAND_SET_TITLE,
        COMMAND_GOTOYX,
        COMMAND_SET_BUFFER_SIZE,
        COMMAND_SYSTEM,
        COMMAND_REDIRECT,
        COMMAND_EXIT,
        COMMAND_TOTAL
    };
}