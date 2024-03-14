cmake_minimum_required(VERSION 3.1.2)
project(MultiConsole VERSION 0.1.0 LANGUAGES C CXX)

option(DEBUG_CONSOLE "Internal Debug mode" OFF)
if(DEBUG_CONSOLE)
    add_compile_definitions(DEBUG_CONSOLE)
    message(STATUS "Internal Debug mode is ON")
else()
    message(STATUS "Internal Debug mode is OFF")
endif()

option(UNICODE "Unicode mode" OFF)
if(UNICODE)
    add_compile_definitions(UNICODE _UNICODE)
    message(STATUS "Unicode is ON")
else()
    message(STATUS "Unicode is OFF")
endif()

add_subdirectory(src/ConsoleHelper)

add_library(MultiConsole STATIC)
#add_executable(MultiConsole)

target_precompile_headers(MultiConsole REUSE_FROM MultiConsoleHelper)
aux_source_directory(src/Console MultiConsoleSources)
target_sources(MultiConsole PRIVATE ${MultiConsoleSources}
                                    src/ConsoleHelper/CConsoleHelperFactory.cpp)
                                    #src/MultiConsole.cpp)
                                    
target_compile_features(MultiConsole PUBLIC cxx_std_17)

set_target_properties(MultiConsole PROPERTIES DEBUG_POSTFIX "d")


