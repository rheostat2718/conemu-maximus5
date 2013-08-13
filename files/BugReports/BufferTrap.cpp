
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	GetConsoleScreenBufferInfo(h, &csbi);
	COORD crNoScroll = {min(80,csbi.srWindow.Right-csbi.srWindow.Left+1), min(25,csbi.srWindow.Bottom-csbi.srWindow.Top+1)};

	printf("Changing console buffer height...");
	SetConsoleScreenBufferSize(h, crNoScroll);

	CHAR_INFO* pLine = new CHAR_INFO[csbi.dwSize.X];
	COORD bufSize = {csbi.dwSize.X,1};
	COORD bufCoord = {0,0};
	SMALL_RECT rgn = {0,9998,csbi.dwSize.X-1,9998};

	printf("\nCalling ReadConsoleOutputW, trap awaiting...");

	// Here will be a crash! Example:
	// First-chance exception at 0x7789F073 (ntdll.dll) in BufferTrap.exe: 0xC0000005: Access violation writing location 0x00A3E000.
	ReadConsoleOutputW(h, pLine, bufSize, bufCoord, &rgn);


	printf("\nSucceeded, strange, no trap?\nPress any key to exit");
	_getch();
	return 0;
}
