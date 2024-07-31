#include "Settings.h"
#include <fstream>
#include <stdexcept>

Settings::Settings()
{
	this->Load();
}

Settings::~Settings()
{
	this->Save();
}

void Settings::Load()
{
	std::ifstream inputStream(SAVEFILE, std::ios::in);

	if (inputStream) {
		std::string targetBrightnessStr;
		int targetBrightness;
		std::getline(inputStream, targetBrightnessStr);
		try {
			targetBrightness = std::stoi(targetBrightnessStr);
		}
		catch (const std::exception& ex) {
			targetBrightness = TARGET_BRIGHTNESS_DEFAULT;
		}
		this->targetBrightness = targetBrightness;

		std::string brightnessToleranceStr;
		int brightnessTolerance;
		std::getline(inputStream, brightnessToleranceStr);
		try {
			brightnessTolerance = std::stoi(brightnessToleranceStr);
		}
		catch (const std::exception& ex) {
			brightnessTolerance = BRIGHTNESS_TOLERANCE_DEFAULT;
		}
		this->brightnessTolerance = brightnessTolerance;

		std::string monitorIntervalStr;
		int monitorInterval;
		std::getline(inputStream, monitorIntervalStr);
		try {
			monitorInterval = std::stoi(monitorIntervalStr);
		}
		catch (const std::exception& ex) {
			monitorInterval = MONITOR_INTERVAL_DEFAULT;
		}
		this->monitorInterval = monitorInterval;

		std::string brightnessChangeStepSizeStr;
		int brightnessChangeStepSize;
		std::getline(inputStream, brightnessChangeStepSizeStr);
		try {
			brightnessChangeStepSize = std::stoi(brightnessChangeStepSizeStr);
		}
		catch (const std::exception& ex) {
			brightnessChangeStepSize = BRIGHTNESS_CHANGE_STEP_SIZE_DEFAULT;
		}
		this->brightnessChangeStepSize = brightnessChangeStepSize;

		std::string redStr, greenStr, blueStr;
		byte red, green, blue;

		std::getline(inputStream, redStr);
		try {
			red = std::stoi(redStr);
		}
		catch (const std::exception& ex) {
			red = OVERLAY_RED_DEFAULT;
		}
		this->overlayRedValue = red;

		std::getline(inputStream, greenStr);
		try {
			green = std::stoi(greenStr);
		}
		catch (const std::exception& ex) {
			green = OVERLAY_GREEN_DEFAULT;
		}
		this->overlayGreenValue = green;

		std::getline(inputStream, blueStr);
		try {
			blue = std::stoi(blueStr);
		}
		catch (const std::exception& ex) {
			blue = OVERLAY_BLUE_DEFAULT;
		}
		this->overlayBlueValue = blue;


		inputStream.close();
	}
}

void Settings::Save() {
	std::ofstream outputStream(SAVEFILE, std::ios::out);
	if (!outputStream)
		throw std::runtime_error("Cannot save settings to file!");
	outputStream << this->targetBrightness << std::endl;
	outputStream << this->brightnessTolerance << std::endl;
	outputStream << this->monitorInterval << std::endl;
	outputStream << this->brightnessChangeStepSize << std::endl;
	outputStream << std::to_string(this->overlayRedValue) << std::endl; // to_string is necessary or else the property is written as the text representation of the byte value (which is not what we want)
	outputStream << std::to_string(this->overlayGreenValue) << std::endl;
	outputStream << std::to_string(this->overlayBlueValue) << std::endl;
	outputStream.close();
}

byte& Settings::GetOverlayRed()
{
	return this->overlayRedValue;
}

byte& Settings::GetOverlayGreen()
{
	return this->overlayGreenValue;
}

byte& Settings::GetOverlayBlue()
{
	return this->overlayBlueValue;
}

int& Settings::GetTargetBrightness()
{
	return this->targetBrightness;
}

int& Settings::GetBrightnessTolerance()
{
	return this->brightnessTolerance;
}

int& Settings::GetMonitorInterval()
{
	return this->monitorInterval;
}

int& Settings::GetBrightnessChangeStepSize()
{
	return this->brightnessChangeStepSize;
}
