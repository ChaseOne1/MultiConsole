#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include ".\Console\CConsole.hpp"

using namespace cs;

inline char GetRandomCharacter()
{
	return rand() % 11 % 2 ? toupper(rand() % 26 + 'a') : rand() % 26 + 'a';
}

int main()
{
	std::cout << "this is Main Window!" << std::endl;

	CConsole *Console = new CConsole("New Console Window");
	CConsole &console = *Console;
	console.SetAsDefaultOutput();

	printf("This is Vice Window!\n");
	console.printf("HELLO\n");
	console << "TEST TEST TEST" << endl;
	console.ResetAsDefaultOutput();
	std::cout << "123123123";
	//Sleep(1000);
	console << COMMAND_SET_TITLE << "HAHAHAHAH" << ends;

	console.~CConsole();

	getchar();
	return 0;
}