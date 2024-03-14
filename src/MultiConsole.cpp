#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include ".\Console\CConsole.hpp"

using namespace cs;

int main()
{
	std::cout << "this is Main Window!" << std::endl;
	CConsole console;
	std::ofstream ofs(console.GetAsOfstream());
	ofs << "Hello" << std::endl;
	console.CancelAsOfstream();
	console << COMMAND_SET_PRINT_COLOR << FOREGROUND_GREEN << ends;
	console.printf("After ofstream");
	getchar();
	return 0;
}