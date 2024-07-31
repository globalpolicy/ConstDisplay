#include "ScreenShotter.h"

ScreenShotter::ScreenShotter()
	:rgbFieldPtr(nullptr)
{
	this->screenWidth = GetSystemMetrics(SM_CXSCREEN);
	this->screenHeight = GetSystemMetrics(SM_CYSCREEN);
}

ScreenShotter::~ScreenShotter()
{
	delete this->rgbFieldPtr;
}

const Matrix<RGBQUAD>& ScreenShotter::Capture()
{
	// ref: https://stackoverflow.com/questions/2659932/how-to-read-the-screen-pixels

	HWND hDesktopWnd = GetDesktopWindow();
	HDC hDesktopDC = GetDC(hDesktopWnd);
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, this->screenWidth, this->screenHeight);
	SelectObject(hCaptureDC, hCaptureBitmap);

	BitBlt(hCaptureDC, 0, 0, this->screenWidth, this->screenHeight, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = this->screenWidth;
	bmi.bmiHeader.biHeight = this->screenHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* pPixels = new RGBQUAD[this->screenWidth * this->screenHeight];

	GetDIBits(
		hCaptureDC,
		hCaptureBitmap,
		0,
		this->screenHeight,
		pPixels,
		&bmi,
		DIB_RGB_COLORS
	);

	// You can now access the raw pixel data in pPixels.  Note that they are stored from the bottom scanline to the top, so pPixels[0] is the lower
	// left pixel, pPixels[1] is the next pixel to the right, pPixels[nScreenWidth] is the first pixel on the second row from the bottom, etc.

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);

	delete this->rgbFieldPtr;
	this->rgbFieldPtr = new Matrix<RGBQUAD>(this->screenHeight, this->screenWidth, pPixels, true); // make a matrix with the pixels, specifying that the vertical axis is to be flipped (we want 0,0 to be top left, not bottom left)
	delete[] pPixels; // clear the allocated mem for the pixels

	return *(this->rgbFieldPtr);
}

/// <summary>
/// Save the captured bitmap to file. NOTE: Only to be called after invoking Capture()
/// </summary>
void ScreenShotter::SaveToFile()
{
	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = this->rgbFieldPtr->GetNumCols();
	bi.biHeight = this->rgbFieldPtr->GetNumRows();
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bi.biWidth * bi.biBitCount + 31) / 32) * 4 * bi.biHeight;

	// A file is created, this is where we will save the screen capture.
	HANDLE hFile = CreateFile(L"captureqwsx.bmp",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size.
	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// Size of the file.
	bmfHeader.bfSize = dwSizeofDIB;

	// bfType must always be BM for Bitmaps.
	bmfHeader.bfType = 0x4D42; // BM.

	DWORD dwBytesWritten = 0;
	WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);

	RGBQUAD* pixelArray = this->rgbFieldPtr->GetRawArray(); // get the captured RGBX pixel array
	BYTE* bgrbgrContiguousArray = new BYTE[bi.biWidth * bi.biHeight * 4]; // allocate space for (total no. of pixels * 4) bytes (coz each pixel is a 4-byte RGBX struct called RGBQUAD)
	for (int i = 0; i < bi.biWidth * bi.biHeight; i++) {
		bgrbgrContiguousArray[i * 4] = pixelArray[i].rgbBlue;
		bgrbgrContiguousArray[i * 4 + 1] = pixelArray[i].rgbGreen;
		bgrbgrContiguousArray[i * 4 + 2] = pixelArray[i].rgbRed;
		bgrbgrContiguousArray[i * 4 + 3] = pixelArray[i].rgbReserved;
	}

	WriteFile(hFile, bgrbgrContiguousArray, dwBmpSize, &dwBytesWritten, NULL);

	// Free the bytearray for rgb data
	delete[] bgrbgrContiguousArray;

	// Close the handle for the file that was created.
	CloseHandle(hFile);
}

#pragma region Interface methods
/// <summary>
/// Generate the RGB field for the screen
/// </summary>
/// <returns></returns>
const Matrix<RGBQUAD>& ScreenShotter::Generate()
{
	return this->Capture();
}

/// <summary>
/// Get average brightness value (as a percentage, 0-100) from the current capture of RGB field
/// </summary>
/// <returns></returns>
int ScreenShotter::GetAverageBrightness() const
{
	int totalBrightness = 0;
	Matrix<RGBQUAD>* rgbField = this->rgbFieldPtr;
	for (int i = 0; i < rgbField->GetNumCols(); i++) {
		for (int j = 0; j < rgbField->GetNumRows(); j++) {
			RGBQUAD pixelRgb = rgbField->GetElement(i, j);
			totalBrightness += pixelRgb.rgbRed + pixelRgb.rgbBlue + pixelRgb.rgbGreen;
		}
	}
	byte averagePixelBrightness = (totalBrightness) / (rgbField->GetNumCols() * rgbField->GetNumRows() * 3);
	return (int)((float)averagePixelBrightness / 0xff * 100);
}
#pragma endregion
