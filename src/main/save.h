#pragma once

#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2 
#include <stdint.h>


#define BI_RGB 0

//typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef int32_t LONG;
typedef uint16_t WORD;


typedef struct tagBITMAPINFOHEADER {
	DWORD  biSize = 0;
	LONG   biWidth = 0;
	LONG   biHeight = 0;
	WORD   biPlanes = 0;
	WORD   biBitCount = 0;
	DWORD  biCompression = 0;
	DWORD  biSizeImage = 0;
	LONG   biXPelsPerMeter = 0;
	LONG   biYPelsPerMeter = 0;
	DWORD  biClrUsed = 0;
	DWORD  biClrImportant = 0;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER_word_version {// 単純に定義するとパディングが起きるので、16バイト単位で定義
	WORD  bfType = 0;
	WORD bfSize_l = 0;
	WORD bfSize_u = 0;
	WORD  bfReserved1 = 0;
	WORD  bfReserved2 = 0;
	WORD bfOffBits_l = 0;
	WORD bfOffBits_u = 0;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

bool SaveImage(char* szPathName, void* lpBits, int w, int h)
{

	//Create a new file for writing

	FILE *pFile;
	fopen_s(&pFile, szPathName, "wb");

	if (pFile == NULL)

	{

		return false;

	}

	BITMAPINFOHEADER BMIH;

	BMIH.biSize = sizeof(BITMAPINFOHEADER);

	BMIH.biSizeImage = w * h * 3;

	// Create the bitmap for this OpenGL context

	BMIH.biSize = sizeof(BITMAPINFOHEADER);

	BMIH.biWidth = w;

	BMIH.biHeight = h;

	BMIH.biPlanes = 1;

	BMIH.biBitCount = 24;

	BMIH.biCompression = BI_RGB;

	BMIH.biSizeImage = w * h * 3;

	BITMAPFILEHEADER bmfh;

	int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;

	LONG lImageSize = BMIH.biSizeImage;

	LONG lFileSize = nBitsOffset + lImageSize;

	bmfh.bfType = 'B' + ('M' << 8);

	bmfh.bfOffBits_u = nBitsOffset >> 16;
	bmfh.bfOffBits_l = nBitsOffset;

	bmfh.bfSize_u = lFileSize >> 16;
	bmfh.bfSize_l = lFileSize;

	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

	//Write the bitmap file header

	UINT nWrittenFileHeaderSize = fwrite(&bmfh, 1,

		sizeof(BITMAPFILEHEADER), pFile);

	//And then the bitmap info header

	UINT nWrittenInfoHeaderSize = fwrite(&BMIH,

		1, sizeof(BITMAPINFOHEADER), pFile);

	//Finally, write the image data itself

	//-- the data represents our drawing

	UINT nWrittenDIBDataSize =

		fwrite(lpBits, 1, lImageSize, pFile);

	fclose(pFile);



	return true;

}