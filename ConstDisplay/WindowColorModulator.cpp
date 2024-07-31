#include "WindowColorModulator.h"


WindowColorModulator::WindowColorModulator(IRGBFieldProducer& rgbProducer, IOverlayer& overlayer, int checkIntervalMs, int targetBrightness, int brightnessDeltaThreshold, int brightnessChangeStepSize)
	:rgbProducer(rgbProducer), overlayer(overlayer), checkIntervalMs(checkIntervalMs), targetBrightness(targetBrightness), brightnessDeltaThreshold(brightnessDeltaThreshold), brightnessChangeStepSize(brightnessChangeStepSize)
{

}

WindowColorModulator::~WindowColorModulator()
{
	this->StopMonitor();
}

void WindowColorModulator::StartMonitor()
{
	this->StopMonitor(); // stop a running thread
	this->monitorThreadKillFlag.store(false); // reset the kill flag
	this->monitorThread = std::thread(&WindowColorModulator::MonitorThreadProc, this); // start a new monitor thread
}

void WindowColorModulator::StopMonitor()
{
	this->monitorThreadKillFlag.store(true); // activate the kill flag
	if (this->monitorThread.joinable())
		this->monitorThread.join(); // wait for a running thread to die
}

/// <summary>
/// Assign a callback to be called when the monitor thread starts. Note that this happens in a new thread.
/// </summary>
/// <param name="handler"></param>
void WindowColorModulator::OnMonitorThreadStarted(void(*handler)(void))
{
	this->monitorThreadStartedEventHandler = handler;
}

/// <summary>
/// Assign a callback to be called when the monitor thread stops. Note that this happens in a new thread.
/// </summary>
/// <param name="handler"></param>
void WindowColorModulator::OnMonitorThreadStopped(void(*handler)(void))
{
	this->monitorThreadStoppedEventHandler = handler;
}

/// <summary>
/// Assign a callback to be called when the monitor thread makes an average-brightness reading. The reading will be reported via this callback func.
/// </summary>
/// <param name="handler"></param>
void WindowColorModulator::OnBrightnessToReport(void(*handler)(int))
{
	this->brightnessReportHandler = handler;
}

void WindowColorModulator::MonitorThreadProc()
{
	if (this->monitorThreadStartedEventHandler)
		std::thread(this->monitorThreadStartedEventHandler).detach(); // notify thread-start subscriber. doing it in a new thread is important to avoid deadlocks and unnecessary holdups

	while (!this->monitorThreadKillFlag.load()) {

		this->rgbProducer.Generate(); // generate rgb field
		int currentBrightness = this->rgbProducer.GetAverageBrightness(); // calc the average brightness

		if (this->brightnessReportHandler)
			std::thread(this->brightnessReportHandler, currentBrightness).detach(); // notify the brightness value subscriber. do it in a new detached thread. we don't want to get stuck here just in case


		// to gradually dim the screen by increasingn our overlay's opacity, if the screen is to bright of course
		int increaseRate = this->brightnessChangeStepSize;
		int deltaBrightness = currentBrightness - this->targetBrightness;
		while (deltaBrightness > this->brightnessDeltaThreshold) { // if screen is too bright, ramp up the opacity of the overlay
			byte currentOpacity = this->overlayer.GetWindowOpacityAlpha();
			byte newOpacity = currentOpacity + increaseRate;
			if (newOpacity < currentOpacity) // this means the new opacity will wrap down to 0 if we increase it further
				break;

			this->overlayer.ChangeWindowOpacityAlpha(currentOpacity + increaseRate);

			this->rgbProducer.Generate(); // generate rgb field
			currentBrightness = this->rgbProducer.GetAverageBrightness(); // calc the average brightness
			deltaBrightness = currentBrightness - this->targetBrightness;
		}

		// to gradually normalize the dimness induced by our overlay if it is too dim of course
		int decreaseRate = this->brightnessChangeStepSize;
		while (deltaBrightness < -this->brightnessDeltaThreshold) { // if screen is too dim, we can't brighten the screen, so just make the overlay transparent
			byte currentOpacity = this->overlayer.GetWindowOpacityAlpha();
			byte newOpacity = currentOpacity - decreaseRate;
			if (newOpacity > currentOpacity) // this means the new opacity will wrap up to 255 if we decrease it further
			{
				this->overlayer.ChangeWindowOpacityAlpha(0);
				break;
			}

			this->overlayer.ChangeWindowOpacityAlpha(currentOpacity - decreaseRate);

			this->rgbProducer.Generate(); // generate rgb field
			currentBrightness = this->rgbProducer.GetAverageBrightness(); // calc the average brightness
			deltaBrightness = currentBrightness - this->targetBrightness;
		}



		Sleep(this->checkIntervalMs);
	}

	if (this->monitorThreadStoppedEventHandler)
		std::thread(this->monitorThreadStoppedEventHandler).detach(); // notify thread-stop subscriber. doing it in a new thread is important to avoid deadlocks and unnecessary holdups
}