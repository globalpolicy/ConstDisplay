#pragma once

#include <string>
#include <Windows.h>
#include <thread>
#include "IOverlayer.h"

class OverlayWindow :public IOverlayer
{
private:
	std::wstring windowName;
	std::wstring windowClassName;
	int x, y, width, height;
	byte opacity;
	std::thread windowThread;
	HWND hWindow;
	void MakeWindow();
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	byte red, green, blue;
public:
	OverlayWindow();
	OverlayWindow(const std::wstring& windowName, const std::wstring& windowClassName, int originX, int originY, int width, int height, byte opacity, byte red, byte green, byte blue);
	~OverlayWindow();
	void ChangeWindowColor(byte red, byte green, byte blue) override;
	void Show() override;
	void ChangeWindowOpacityAlpha(byte alpha) override;
	byte GetWindowOpacityAlpha() const override;
	RGBQUAD GetWindowColor() const;
};

