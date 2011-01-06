
#pragma once

class DirectDrawSurface;


enum {
	eRenderFull = 1,
	eRenderScale = 2, // ��� ������ ������
	eRenderRectScale = 4, // ������������� ������ ������� �����������
} DecodingFlags;

enum {
	eCurrentImageCurrentPage, // highest
	eCurrentImageNextPage,
	eNextImage,
} DecodingPriority;


typedef struct tag_DecodeItem
{
	ImageInfo* pImage; // ������ ��� ��������� � �������� ���� GetDecodedImage, IsImageReady
	wchar_t* pFullFileName; // ����� ������. ��������� ������ ��� DecoderCache
	int nFileIndex; // ������ ����� �� ������. ����� ���� -1 ���� ������ �� �������������
	
	// ����������� �����������. � ���������, ��������� ��������� ����� ������ ���� � � �������
	// ��� �� ��������. ��� �������� GFL & GhostScript.
	LPBYTE pBuf;
	DWORD  lBuf;
	
	DecodingFlags Flags;
	DecodingPriority Priority;
	
	RECT rcSource;
	SIZE szTarget;
	
	LPVOID pOutputContext; // �����
	DirectDrawSurface* dds; // � ��� ������
} DecodeItem;

class DecoderCache
{
public:
	
public:
	// �� ��������� ������������� ������ ���������, ������� ��� �����������,
	// � ���� �� - ����������� Invalidate, ����� ����� ���������� �����������
	DWORD CALLBACK DecodingThread(LPVOID pParam);
	
public:
	// �������� ����� ��� ���-�� ��� ����������� ������ �� �� ������ ������� ����.

	// ������� ������ �������� ��������� ������������� ���� ��������
	// �� ������������� � ��������� ��������� �������������
	BOOL GetDecodedImage(int nFileIndex, UINT nPage, DecodingFlags, pvdInfoDecode2* ...);
	// pFullFileName ��� ���� ����������� ����������� �� ������� ������������ ����� �� ������
	BOOL GetDecodedImage(const wchar_t* pFullFileName, UINT nPage, DecodingFlags, pvdInfoDecode2* ...);
	
	// ��������� � ������� �������������
	BOOL RequestDecodedImage(int nFileIndex, UINT nPage, DecodingPriority, DecodingFlags, );
	// pFullFileName ��� ���� ����������� ����������� �� ������� ������������ ����� �� ������
	BOOL RequestDecodedImage(const wchar_t* pFullFileName, UINT nPage, DecodingPriority, DecodingFlags, );

	// ����� ������������� ��� ������ ����� �����������. �� ���� � ����
	// ������ ����� ������������� 	
	BOOL IsImageReady(Image*, DecodingFlags, RECT,...);
};
