#ifdef WIN32
#include <iostream>
#include <windows.h>
#include <gdiplus.h>
 
using namespace std;
using namespace Gdiplus;

#else
#include <stdio.h>
#endif

#ifdef WIN32
wchar_t *GetWC(char *c)
{
	size_t cSize = strlen(c)+1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs (wc, c, cSize);
	return wc;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure
	GetImageEncoders(num, size, pImageCodecInfo);
	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}
	free(pImageCodecInfo);
	return 0;
}


bool CompressImagePixel(const WCHAR* pszOriFilePath, const WCHAR* pszDestFilePah,UINT ulNewWidth ,UINT ulNewHeigth)
{
    // Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
    Status stat = GenericError;
    stat = GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );
    if ( Ok != stat )return false;
    stat = GenericError;
    Image* pImage = new Image(pszOriFilePath);
 
 	if ( NULL == pImage )return false;
	UINT unOriHeight = pImage->GetHeight();
	UINT unOriWidth = pImage->GetWidth();
	
	CLSID encoderClsid;
	if ( unOriWidth < 1 || unOriHeight < 1 )return false;
	if ( !GetEncoderClsid(L"image/jpeg", &encoderClsid) )return false;
	REAL fSrcX = 0.0f;
	REAL fSrcY = 0.0f;
	REAL fSrcWidth = (REAL) unOriWidth;
	REAL fSrcHeight = (REAL) unOriHeight ;
	RectF RectDest( 0.0f, 0.0f, (REAL)ulNewWidth, (REAL)ulNewHeigth);
 
 	Bitmap* pTempBitmap = new Bitmap( ulNewWidth, ulNewHeigth );
	Graphics* graphics = NULL;
    if ( !pTempBitmap )return false;
 
    graphics = Graphics::FromImage( pTempBitmap );
    if ( !graphics )return false;
	stat = graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
	if ( Ok != stat )return false;
 
    stat = graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    if ( Ok != stat )return false;
 
    stat = graphics->DrawImage( pImage, RectDest, fSrcX, fSrcY, fSrcWidth, fSrcHeight,
        UnitPixel, NULL, NULL, NULL);
    if ( Ok != stat )return false;
 
    stat = pTempBitmap->Save( pszDestFilePah, &encoderClsid, NULL );
	if ( Ok != stat )return false;
}


void BMP2JPG(char* BMPName,char* JPGName)
{

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID   encoderClsid;
	Status  stat;
	WCHAR * BMPS={NULL};
	WCHAR * JPG={NULL};
	BMPS=GetWC(BMPName);
	JPG=GetWC(JPGName);
	Image  image(BMPS);

	int result =GetEncoderClsid(L"image/jpeg", &encoderClsid);
	stat = image.Save(JPG, &encoderClsid, NULL);

	if(stat == Ok)
		printf("Jpeg was saved successfully\n");
	else
		printf("Failure: stat = %d\n", stat);

}
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;
	int iBits;
	WORD wBitCount;
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	BITMAP Bitmap0;
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;
	HANDLE fh, hDib, hPal, hOldPal2 = NULL;
	hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;
	GetObject(hBitmap, sizeof(Bitmap0), (LPSTR)&Bitmap0);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap0.bmWidth;
	bi.biHeight = -Bitmap0.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 256;
	dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8
		* Bitmap0.bmHeight;
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = GetDC(NULL);
		hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}


	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+ dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	if (hOldPal2)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

void screenshot(char* filename)
{
	HDC hScreenDC = CreateDC("DISPLAY", NULL, NULL, NULL);     

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);

	// maybe worth checking these are positive values
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);

	// get a new bitmap
	HBITMAP hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP) SelectObject(hMemoryDC, hOldBitmap);
	SaveHBITMAPToFile(hBitmap,filename);

	// clean up
	DeleteDC(hMemoryDC);	
	DeleteDC(hScreenDC);
}

#else
int ScrotCmd(char* JpgName)
{
	FILE * fp;
	char buffer[80]={0};
	char cmd[50]={0};
	sprintf(cmd,"scrot %s --thumb 42",JpgName);
	fp = popen(cmd, "r");

	int status=pclose(fp);
	if(status!=0)
	{
		return -1;
	}
	return 0;
}
#endif
