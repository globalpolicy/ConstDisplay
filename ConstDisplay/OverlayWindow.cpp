#include "OverlayWindow.h"

#include <stdexcept>

OverlayWindow::OverlayWindow()
	:windowName(L"ConstDisplay - Overlay window"), windowClassName(L"ConstDisplayWindowClass"), x(0), y(0), opacity(0x00), hWindow(0), red(0), green(0), blue(0) {

	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	if (screenHeight == 0 || screenWidth == 0)
		throw std::runtime_error("Cannot get screen resolution!");

	this->height = screenHeight;
	this->width = screenWidth;
}

OverlayWindow::OverlayWindow(const std::wstring& windowName, const std::wstring& windowClassName, int originX, int originY, int width, int height, byte opacity, byte red, byte green, byte blue)
	:windowName(windowName), windowClassName(windowClassName), x(originX), y(originY), width(width), height(height), opacity(opacity), hWindow(0), red(red), green(green), blue(blue)
{

}

OverlayWindow::~OverlayWindow() {
	SendMessage(this->hWindow, WM_CLOSE, 0, 0); // to unregister a class, all windows using the class must be destroyed first. so, PostMessage will not work here.
	UnregisterClass(this->windowClassName.c_str(), 0);
	this->windowThread.join(); // without this, the main thread will close without ensuring the window thread's termination and lead to "abort() has been called"
}

/// <summary>
/// Actual method to create the window according to the class member values
/// </summary>
void OverlayWindow::MakeWindow() {
	DWORD error = 0;

	WNDCLASS wndClass = { 0 };
	wndClass.lpszClassName = this->windowClassName.c_str();
	wndClass.lpfnWndProc = OverlayWindow::WndProcStatic;
	wndClass.hbrBackground = CreateSolidBrush(RGB(this->red, this->green, this->blue));

	ATOM windowClass = RegisterClass(&wndClass);
	if (windowClass) {

		// ref: https://www.codeproject.com/Articles/12877/Transparent-Click-Through-Forms, https://stackoverflow.com/questions/6165136/ws-ex-transparent-what-does-it-actually-do, https://stackoverflow.com/questions/11064302/hittest-transparency-for-an-entire-form

		HWND hWindow = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, // WS_EX_TRANSPARENT is central to creating a click-through window. WS_EX_LAYERED is needed for controlling window opacity. WS_EX_TOOLWINDOW is needed so that the window's icons don't appear on the taskbar
			MAKEINTATOM(windowClass),
			this->windowName.c_str(),
			WS_VISIBLE | WS_POPUP,
			this->x, this->y,
			this->width, this->height,
			0,
			0,
			0,
			this // pass this instance to the new window's lParam
		);


		if (hWindow) {

			SetLayeredWindowAttributes(hWindow, 0, this->opacity, LWA_ALPHA); // set opacity level

			SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // set the window to be topmost

			this->hWindow = hWindow;

			MSG msg = { 0 };
			while (GetMessage(&msg, 0, 0, 0)) { // start the message pump
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		else {
			error = GetLastError();

		}

	}
	else {
		error = GetLastError();

	}
}

/// <summary>
/// Window Proc for the created window
/// </summary>
/// <param name="hWnd"></param>
/// <param name="uMsg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK OverlayWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/// <summary>
/// A static WndProc to forward to the instance method WndProc. Why? see https://devblogs.microsoft.com/oldnewthing/20140203-00/?p=1893 and countless SO posts on the topic
/// </summary>
/// <param name="hWnd"></param>
/// <param name="uMsg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK OverlayWindow::WndProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
	{
		CREATESTRUCT* lParamStruct = reinterpret_cast<CREATESTRUCT*>(lParam); // this struct is only available the first time the window is created. it contains the last param passed to CreateWindowEx()
		if (lParamStruct)
		{
			SetLastError(0);

			if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lParamStruct->lpCreateParams))) // attach the last param passed to the CreateWindowEx() function permanently to this window
			{
				if (GetLastError() != 0)
					throw std::runtime_error("Could not attach GWLP_USERDATA upon window creation!");
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		else
			throw std::runtime_error("CREATESTRUCT* not available upon window creation!");
		break;
	}
	default:
		OverlayWindow* thisObj = reinterpret_cast<OverlayWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!thisObj)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			return thisObj->WndProc(hWnd, uMsg, wParam, lParam); // I had never used private instance members inside static methods before in C#, much less C++. wow. this explains it all: "The private means private for the class and not private for the instance.". Ref: https://stackoverflow.com/questions/1631124/accessing-private-member-of-a-parameter-within-a-static-method
		break;
	}
}

#pragma region Interface methods
/// <summary>
/// Creates the overlay window in a new thread.
/// </summary>
void OverlayWindow::Show()
{
	this->windowThread = std::thread(&OverlayWindow::MakeWindow, this); // spin up a thread, say window thread, to execute MakeWindow()
}

/// <summary>
/// Changes the opacity of the overlay window as specified. NOTE: This method doesn't run in the window thread
/// </summary>
/// <param name="alpha"></param>
void OverlayWindow::ChangeWindowOpacityAlpha(byte alpha)
{
	this->opacity = alpha;
	SetLayeredWindowAttributes(this->hWindow, 0, this->opacity, LWA_ALPHA); // set opacity level
}

byte OverlayWindow::GetWindowOpacityAlpha() const
{
	return this->opacity;
}

RGBQUAD OverlayWindow::GetWindowColor() const
{
	return RGBQUAD{ this->blue,this->green,this->red };
}

/// <summary>
/// Changes the background color of the overlay window as specified. NOTE: This method doesn't run in the window thread
/// </summary>
/// <param name="red"></param>
/// <param name="green"></param>
/// <param name="blue"></param>
void OverlayWindow::ChangeWindowColor(byte red, byte green, byte blue)
{
	this->red = red;
	this->green = green;
	this->blue = blue;
	HBRUSH newBrush = CreateSolidBrush(RGB(red, green, blue));
	LONG_PTR oldWindowLong = SetClassLongPtr(this->hWindow, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(newBrush));
	BOOL error = InvalidateRect(this->hWindow, NULL, true);
	error = UpdateWindow(this->hWindow);
}
#pragma endregion
