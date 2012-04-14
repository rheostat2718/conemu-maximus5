
#include <windows.h>

int mainCRTStartup(int argc, char** argv)
{
	SYSTEMTIME st; GetLocalTime(&st);
	char szTime[32];
	wsprintfA(szTime, "%u:%02u:%02u\n", st.wHour, st.wMinute, st.wSecond);
	DWORD n;
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), szTime, lstrlenA(szTime), &n, NULL);
	return 0;
}
