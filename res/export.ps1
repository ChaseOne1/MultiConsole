$exeFilePath = ".\MultiConsoleHelper.exe"  
$outputFilePath = "..\src\ConsoleHelper\resource.hpp"  

$hexBytes = [System.IO.File]::ReadAllBytes($exeFilePath) | ForEach-Object { "0x" + $_.ToString("X2") }  

$cppArrayString = "#pragma once`n#include <windows.h>`n`nBYTE g_abyExeData[] = {" + ($hexBytes -join ", ") + "};`n`nconstexpr int g_nExeData = sizeof(g_abyExeData) / sizeof(g_abyExeData[0]);"  

[System.IO.File]::WriteAllText($outputFilePath, $cppArrayString)  
  
Write-Host "Done! The hex data has been written to $outputFilePath"