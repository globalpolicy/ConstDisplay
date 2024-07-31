#pragma once
#include <string>
#include <tchar.h>
#include <Windows.h>

class Settings
{
private:
	const TCHAR* SAVEFILE = _T("ConstDisp.dat");

	const int TARGET_BRIGHTNESS_DEFAULT = 50, BRIGHTNESS_TOLERANCE_DEFAULT = 5, MONITOR_INTERVAL_DEFAULT = 500, BRIGHTNESS_CHANGE_STEP_SIZE_DEFAULT = 2;
	const byte OVERLAY_RED_DEFAULT = 0, OVERLAY_GREEN_DEFAULT = 0, OVERLAY_BLUE_DEFAULT = 0;

	int targetBrightness = TARGET_BRIGHTNESS_DEFAULT, brightnessTolerance = BRIGHTNESS_TOLERANCE_DEFAULT, monitorInterval = MONITOR_INTERVAL_DEFAULT, brightnessChangeStepSize = BRIGHTNESS_CHANGE_STEP_SIZE_DEFAULT;
	byte overlayRedValue = OVERLAY_RED_DEFAULT, overlayGreenValue = OVERLAY_GREEN_DEFAULT, overlayBlueValue = OVERLAY_BLUE_DEFAULT;
public:
	Settings();
	~Settings();
	void Load();
	void Save();

#pragma region Properties (implemented as reference accessors)
	byte& GetOverlayRed();
	byte& GetOverlayGreen();
	byte& GetOverlayBlue();
	int& GetTargetBrightness();
	int& GetBrightnessTolerance();
	int& GetMonitorInterval();
	int& GetBrightnessChangeStepSize();
#pragma endregion



};

