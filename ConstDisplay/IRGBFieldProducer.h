#pragma once
#include <Windows.h>
#include "Matrix.h"

class IRGBFieldProducer {
public:
	virtual const Matrix<RGBQUAD>& Generate() = 0;
	virtual int GetAverageBrightness() const = 0;
	virtual ~IRGBFieldProducer() = default; // necessary so that implementing classes' destructors are called when 'delete'ing
};