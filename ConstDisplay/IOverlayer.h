#pragma once

class IOverlayer {
public:
	virtual void Show() = 0;
	virtual void ChangeWindowColor(byte red, byte green, byte blue) = 0;
	virtual void ChangeWindowOpacityAlpha(byte alpha) = 0;
	virtual byte GetWindowOpacityAlpha() const = 0;
	virtual ~IOverlayer() = default; // necessary so that implementing classes' destructors are called when 'delete'ing
};