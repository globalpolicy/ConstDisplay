#pragma once
#include "IRGBFieldProducer.h"
#include "IOverlayer.h"

#include <thread>
#include <atomic>

class WindowColorModulator
{
private:
	IRGBFieldProducer& rgbProducer;
	IOverlayer& overlayer;
	std::thread monitorThread;
	std::atomic<bool> monitorThreadKillFlag;
	void MonitorThreadProc();
	int checkIntervalMs;
	int targetBrightness;
	int brightnessDeltaThreshold;
	int brightnessChangeStepSize;
	void(*monitorThreadStartedEventHandler)(void);
	void(*monitorThreadStoppedEventHandler)(void);
	void(*brightnessReportHandler)(int);

public:
	WindowColorModulator(IRGBFieldProducer& rgbProducer, IOverlayer& overlayer, int checkIntervalMs = 500, int targetBrightness = 50, int brightnessDeltaThreshold = 5, int brightnessChangeStepSize = 2);
	~WindowColorModulator();
	void StartMonitor();
	void StopMonitor();
	void OnMonitorThreadStarted(void(*handler)(void));
	void OnMonitorThreadStopped(void(*handler)(void));
	void OnBrightnessToReport(void(*handler)(int));
};

