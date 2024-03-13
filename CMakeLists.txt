cmake_minimum_required(VERSION 3.1.0)
project(MultiConsole VERSION 0.1.0 LANGUAGES C CXX)

option(UNICODE "Unicode mode" OFF)
if(UNICODE)
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

if(UNICODE)
    target_compile_definitions(MultiConsole PRIVATE "UNICODE" "_UNICODE")
endif()

set_target_properties(MultiConsole PROPERTIES DEBUG_POSTFIX "d")


