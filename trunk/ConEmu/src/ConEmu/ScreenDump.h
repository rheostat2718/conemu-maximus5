
#pragma once

BOOL DumpImage(HDC hScreen, HBITMAP hBitmap, int anWidth, int anHeight, LPCWSTR pszFile);
BOOL DumpImage(BITMAPINFOHEADER* pHdr, LPVOID pBits, LPCWSTR pszFile);
