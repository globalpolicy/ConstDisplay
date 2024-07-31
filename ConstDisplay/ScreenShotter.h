#pragma once
#include <Windows.h>
#include "Matrix.h"
#include "IRGBFieldProducer.h"

class ScreenShotter :public IRGBFieldProducer
{
private:
	Matrix<RGBQUAD>* rgbFieldPtr;
	int screenWidth, screenHeight;

public:
	ScreenShotter();
	~ScreenShotter();
	const Matrix<RGBQUAD>& Capture();
	void SaveToFile();
	const Matrix<RGBQUAD>& Generate() override;
	int GetAverageBrightness() const override;
};

