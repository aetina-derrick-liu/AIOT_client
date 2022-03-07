#ifndef __SCREENSHOT_HPP__
#define __SCREENSHOT_HPP__

#ifdef WIN32
void screenshot(char* filename);
void BMP2JPG(char* BMPName,char* JPGName);
bool CompressImagePixel(const WCHAR* pszOriFilePath, const WCHAR* pszDestFilePah,UINT ulNewWidth ,UINT ulNewHeigth);
#else
int ScrotCmd(char* JpgName);
#endif

#endif
