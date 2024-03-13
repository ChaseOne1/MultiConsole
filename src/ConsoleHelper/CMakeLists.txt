add_executable(MultiConsoleHelper)

target_precompile_headers(MultiConsoleHelper PRIVATE ../pch.hpp)
SET(ConsoleHelperSources CConsoleHelper.cpp ConsoleHelper.cpp)
target_sources(MultiConsoleHelper PRIVATE ${ConsoleHelperSources})

target_compile_features(MultiConsoleHelper PUBLIC cxx_std_17)
set_target_properties(MultiConsoleHelper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ../../../res/)
set_target_properties(MultiConsoleHelper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ../../../res/)

add_custom_command(TARGET MultiConsoleHelper 
                    POST_BUILD COMMAND powershell -ExecutionPolicy Bypass -File export.ps1
                    WORKING_DIRECTORY ../../../res)

if(UNICODE)
    target_compile_definitions(MultiConsoleHelper PRIVATE "UNICODE" "_UNICODE")
endif()