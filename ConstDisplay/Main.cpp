#include <Windows.h>

#include "OverlayWindow.h"
#include "ScreenShotter.h"

#include "IOverlayer.h"
#include "IRGBFieldProducer.h"
#include "WindowColorModulator.h"
#include "simwin.h"
#include "Settings.h"



HWND textboxhwnd, btnhwnd, formhwnd,
staticLabelTargetBrightnessHwnd, trackbarTargetBrightnessHwnd,
staticLabelBrightnessToleranceHwnd, trackbarBrightnessToleranceHwnd,
staticLabelMonitorIntervalHwnd, trackbarMonitorIntervalHwnd,
staticLabelBrightnessChangeStepSizeHwnd, trackbarBrightnessChangeStepSizeHwnd,
staticLabelOverlayColorHwnd, staticLabelOverlayRedHwnd, textboxOverlayRedHwnd, staticLabelOverlayGreenHwnd, textboxOverlayGreenHwnd, staticLabelOverlayBlueHwnd, textboxOverlayBlueHwnd,
btnPickColor,
statusBarHwnd;
HMENU mainpopupmenuFile, mainpopupmenuEdit, trayIconMainMenu;
int mainpopupmenuitemAbout, popupmenufileSave, popupmenufileTray, popupmenufileTopmost, popupmenufileExit, popupmenueditPause, popupmenueditStart, popupmenueditStop,
trayIcon, trayIconPopupMenuPause, trayIconPopupMenuStart, trayIconPopupMenuStop, trayIconPopupMenuExit;

Settings settings;
IOverlayer* overlayer;
IRGBFieldProducer* rgbFieldProducer;
WindowColorModulator* colorModulator;

#pragma region Helper functions

void disableAllSettingsControls() {
	SendMessage(textboxOverlayRedHwnd, EM_SETREADONLY, true, 0);
	SendMessage(textboxOverlayGreenHwnd, EM_SETREADONLY, true, 0);
	SendMessage(textboxOverlayBlueHwnd, EM_SETREADONLY, true, 0);
	EnableWindow(trackbarTargetBrightnessHwnd, false);
	EnableWindow(trackbarBrightnessToleranceHwnd, false);
	EnableWindow(trackbarBrightnessChangeStepSizeHwnd, false);
	EnableWindow(trackbarMonitorIntervalHwnd, false);

}

void enableAllSettingsControls() {
	SendMessage(textboxOverlayRedHwnd, EM_SETREADONLY, false, 0);
	SendMessage(textboxOverlayGreenHwnd, EM_SETREADONLY, false, 0);
	SendMessage(textboxOverlayBlueHwnd, EM_SETREADONLY, false, 0);
	EnableWindow(trackbarTargetBrightnessHwnd, true);
	EnableWindow(trackbarBrightnessToleranceHwnd, true);
	EnableWindow(trackbarBrightnessChangeStepSizeHwnd, true);
	EnableWindow(trackbarMonitorIntervalHwnd, true);

}
#pragma endregion


#pragma region UI Event Handlers

void trackbarTargetBrightnessPositionChanged(int value) {
	SetStatusBarText(statusBarHwnd, std::wstring(L"Target Brightness = " + std::to_wstring(value)).c_str());
}

void fileSaveMenuItemClicked() {
	try {
		// the trackbar-controlled settings and RGB values set via the color picker will update the settings object in real-time, 
		// but for RGB textbox modifications, we need to update it manually

		settings.GetOverlayRed() = std::stoi(GetTextboxText(textboxOverlayRedHwnd));
		settings.GetOverlayGreen() = std::stoi(GetTextboxText(textboxOverlayGreenHwnd));
		settings.GetOverlayBlue() = std::stoi(GetTextboxText(textboxOverlayBlueHwnd));

		settings.Save();
		SetStatusBarText(statusBarHwnd, _T("Settings saved to file!"));
	}
	catch (const std::exception& ex) {
		SetStatusBarText(statusBarHwnd, _T("One or more RGB values is invalid!"));
		MessageBox(formhwnd, _T("Could not save settings! One or more RGB values is invalid."), _T("Error"), MB_ICONERROR);
	}
}

void fileTrayMenuItemClicked() {
	ShowTrayIcon(trayIcon);
	ShowBalloonMessage(trayIcon, _T("ConstDisp"), _T("ConstDisp is running in the background. Click on the tray icon to restore."), TrayIconBalloonTypesEnum::TRAYICONBALLOONTYPE_INFO, true);
	ShowWindow(formhwnd, SW_HIDE);
}

void trayIconRightClicked() {
	ShowTrayIconPopupMenu(trayIcon);
}

void fileTopmostMenuItemClicked() {
	bool isChecked = IsMenuItemChecked(formhwnd, popupmenufileTopmost);
	SetWindowPos(formhwnd, !isChecked ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // set the window to be topmost if the menu item was unchecked previously
	SetMenuItemCheckStatus(formhwnd, popupmenufileTopmost, !isChecked);
}

void fileExitMenuItemClicked() {
	SendMessage(formhwnd, WM_CLOSE, 0, 0);
}


void editStartMenuItemClicked() {
	// create an IOverlayer
	delete overlayer;
	overlayer = new OverlayWindow();
	overlayer->Show();
	overlayer->ChangeWindowColor(settings.GetOverlayRed(), settings.GetOverlayGreen(), settings.GetOverlayBlue());

	// create an IRGBFieldProducer
	delete rgbFieldProducer;
	rgbFieldProducer = new ScreenShotter();

	// create the overlay color modulator
	delete colorModulator;
	colorModulator = new WindowColorModulator(*rgbFieldProducer, *overlayer, settings.GetMonitorInterval(), settings.GetTargetBrightness(), settings.GetBrightnessTolerance(), settings.GetBrightnessChangeStepSize());

	// hook up event handlers to report color modulator's monitor thread status
	colorModulator->OnMonitorThreadStarted([]() {
		SetMenuItemGrayStatus(formhwnd, popupmenueditStart, true);
		SetMenuItemGrayStatus(formhwnd, trayIconPopupMenuStart, true);

		SetMenuItemGrayStatus(formhwnd, popupmenueditStop, false);
		SetMenuItemGrayStatus(formhwnd, trayIconPopupMenuStop, false);

		disableAllSettingsControls();
		SetStatusBarText(statusBarHwnd, _T("Monitor active"));
		});
	colorModulator->OnMonitorThreadStopped([]() {
		SetMenuItemGrayStatus(formhwnd, popupmenueditStart, false);
		SetMenuItemGrayStatus(formhwnd, trayIconPopupMenuStart, false);

		SetMenuItemGrayStatus(formhwnd, popupmenueditStop, true);
		SetMenuItemGrayStatus(formhwnd, trayIconPopupMenuStop, true);

		enableAllSettingsControls();
		SetStatusBarText(statusBarHwnd, _T("Monitor inactive"));
		});

	// hook up event handlers to report average brightness read out by the class
	colorModulator->OnBrightnessToReport([](int brightness) {
		SetStatusBarText(statusBarHwnd, (L"Mean brightness:" + std::to_wstring(brightness)).c_str());
		});

	// start the monitor
	colorModulator->StartMonitor();
}

void editStopMenuItemClicked() {
	colorModulator->StopMonitor();
	overlayer->ChangeWindowOpacityAlpha(0);
}

void editPauseMenuItemClicked() {
	colorModulator->StopMonitor();
}

void aboutMenuItemClicked() {
	MessageBox(formhwnd, _T("Product: ConstDisplay v1.0\nAuthor: globalpolicy\nHome: github.com/globalpolicy/constdisplay\nContact: yciloplabolg@gmail.com"), _T("About"), MB_ICONINFORMATION);
}

void trayIconClicked() {
	ShowWindow(formhwnd, SW_RESTORE);
	HideTrayIcon(trayIcon);
}

void trackbarTargetBrightnessChanged(int value) {
	settings.GetTargetBrightness() = value;
	SetStatusBarText(statusBarHwnd, std::wstring(L"Target Brightness = " + std::to_wstring(value)).c_str());
}

void trackbarBrightnessToleranceChanged(int value) {
	settings.GetBrightnessTolerance() = value;
	SetStatusBarText(statusBarHwnd, std::wstring(L"Brightness delta tolerance = " + std::to_wstring(value)).c_str());
}

void trackbarMonitorIntervalChanged(int value) {
	settings.GetMonitorInterval() = value;
	SetStatusBarText(statusBarHwnd, std::wstring(L"Brightness check interval = " + std::to_wstring(value) + L" ms").c_str());
}

void trackbarBrightnessChangeStepSizeChanged(int value) {
	settings.GetBrightnessChangeStepSize() = value;
	SetStatusBarText(statusBarHwnd, std::wstring(L"Brightness change step size = " + std::to_wstring(value)).c_str());
}

void btnColorPickerClicked() {
	static COLORREF acrCustClr[16];
	CHOOSECOLOR chooseColor = { 0 };
	chooseColor.lStructSize = sizeof(chooseColor);
	chooseColor.hwndOwner = formhwnd;
	chooseColor.Flags = CC_RGBINIT;
	chooseColor.lpCustColors = acrCustClr; // this, apparently, is a must
	if (ChooseColor(&chooseColor)) {
		byte red = GetRValue(chooseColor.rgbResult);
		byte green = GetGValue(chooseColor.rgbResult);
		byte blue = GetBValue(chooseColor.rgbResult);

		settings.GetOverlayRed() = red;
		SetTextboxText(textboxOverlayRedHwnd, std::to_wstring(red).c_str());

		settings.GetOverlayGreen() = green;
		SetTextboxText(textboxOverlayGreenHwnd, std::to_wstring(green).c_str());

		settings.GetOverlayBlue() = blue;
		SetTextboxText(textboxOverlayBlueHwnd, std::to_wstring(blue).c_str());
	}
}
#pragma endregion


void MakeUI() {
	formhwnd = CreateForm(_T("ConstDisplay"), _T("SimwinForm"), 320, 280);
	if (formhwnd) {

		LONG_PTR styles = GetWindowLongPtr(formhwnd, GWL_STYLE);
		SetWindowLongPtr(formhwnd, GWL_STYLE, styles & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX); // make the main form non-resizable

		CenterForm(formhwnd); // center the form in the screen

		// i've found it's wise to first deal with menu creation, then move on to other controls - status bar in particular will draw below the bottom edge of the window otherwise
		mainpopupmenuFile = AddMainPopupMenu(formhwnd, _T("File"));
		popupmenufileSave = AddFinalMenuItem(mainpopupmenuFile, _T("Save"));
		popupmenufileTray = AddFinalMenuItem(mainpopupmenuFile, _T("Tray"));
		popupmenufileTopmost = AddFinalMenuItem(mainpopupmenuFile, _T("Topmost"));
		popupmenufileExit = AddFinalMenuItem(mainpopupmenuFile, _T("Exit"));

		mainpopupmenuEdit = AddMainPopupMenu(formhwnd, _T("Edit"));
		popupmenueditPause = AddFinalMenuItem(mainpopupmenuEdit, _T("Pause"));
		popupmenueditStart = AddFinalMenuItem(mainpopupmenuEdit, _T("Start"));
		popupmenueditStop = AddFinalMenuItem(mainpopupmenuEdit, _T("Stop"));

		mainpopupmenuitemAbout = AddMainMenuItem(formhwnd, _T("About"));
		AddMenuItemEvent(mainpopupmenuitemAbout, MenuItemEventsEnum::MENUITEMEVENT_CLICK, &aboutMenuItemClicked);

		AddMenuItemEvent(popupmenufileSave, MENUITEMEVENT_CLICK, &fileSaveMenuItemClicked);
		AddMenuItemEvent(popupmenufileTray, MENUITEMEVENT_CLICK, &fileTrayMenuItemClicked);
		AddMenuItemEvent(popupmenufileTopmost, MENUITEMEVENT_CLICK, &fileTopmostMenuItemClicked);
		AddMenuItemEvent(popupmenufileExit, MENUITEMEVENT_CLICK, &fileExitMenuItemClicked);

		AddMenuItemEvent(popupmenueditPause, MENUITEMEVENT_CLICK, &editPauseMenuItemClicked);
		AddMenuItemEvent(popupmenueditStart, MENUITEMEVENT_CLICK, &editStartMenuItemClicked);
		AddMenuItemEvent(popupmenueditStop, MENUITEMEVENT_CLICK, &editStopMenuItemClicked);


		staticLabelTargetBrightnessHwnd = AddStaticLabel(formhwnd, _T("Target brightness (0-100)"), 5, 5, 150, 20);
		trackbarTargetBrightnessHwnd = AddTrackbar(formhwnd, 200, 5, 100, 30, 0, 100, true, settings.GetTargetBrightness(), false, 1);
		AddTrackbarEvent(trackbarTargetBrightnessHwnd, TrackbarEventsEnum::TRACKBAREVENT_VALUE_CHANGING, &trackbarTargetBrightnessChanged);

		staticLabelBrightnessToleranceHwnd = AddStaticLabel(formhwnd, _T("Brightness delta tolerance (0-100)"), 5, 40, 180, 20);
		trackbarBrightnessToleranceHwnd = AddTrackbar(formhwnd, 200, 40, 100, 30, 0, 100, true, settings.GetBrightnessTolerance(), false, 0);
		AddTrackbarEvent(trackbarBrightnessToleranceHwnd, TrackbarEventsEnum::TRACKBAREVENT_VALUE_CHANGING, &trackbarBrightnessToleranceChanged);

		staticLabelMonitorIntervalHwnd = AddStaticLabel(formhwnd, _T("Brightness check interval (ms)"), 5, 75, 180, 20);
		trackbarMonitorIntervalHwnd = AddTrackbar(formhwnd, 200, 75, 100, 30, 0, 5000, true, settings.GetMonitorInterval(), false, 0);
		AddTrackbarEvent(trackbarMonitorIntervalHwnd, TrackbarEventsEnum::TRACKBAREVENT_VALUE_CHANGING, &trackbarMonitorIntervalChanged);

		staticLabelBrightnessChangeStepSizeHwnd = AddStaticLabel(formhwnd, _T("Brightness change step size (0-255)"), 5, 110, 180, 20);
		trackbarBrightnessChangeStepSizeHwnd = AddTrackbar(formhwnd, 200, 110, 100, 30, 0, 255, true, settings.GetBrightnessChangeStepSize(), false, 0);
		AddTrackbarEvent(trackbarBrightnessChangeStepSizeHwnd, TrackbarEventsEnum::TRACKBAREVENT_VALUE_CHANGING, &trackbarBrightnessChangeStepSizeChanged);

		staticLabelOverlayColorHwnd = AddStaticLabel(formhwnd, _T("Overlay colors (0-255):"), 5, 145, 180, 20);
		btnPickColor = AddButton(formhwnd, _T("Pick"), 200, 145, 50, 20);
		AddButtonEvent(btnPickColor, ButtonEventsEnum::BUTTONEVENT_LCLICK, &btnColorPickerClicked);

		staticLabelOverlayRedHwnd = AddStaticLabel(formhwnd, _T("Red"), 5, 170, 50, 20);
		textboxOverlayRedHwnd = AddTextbox(formhwnd, std::to_wstring(settings.GetOverlayRed()).c_str(), 40, 170, 30, 20, false, false, false);
		SetTextboxNumericInputOnly(textboxOverlayRedHwnd, true);

		staticLabelOverlayGreenHwnd = AddStaticLabel(formhwnd, _T("Green"), 100, 170, 50, 20);
		textboxOverlayGreenHwnd = AddTextbox(formhwnd, std::to_wstring(settings.GetOverlayGreen()).c_str(), 135, 170, 30, 20, false, false, false);
		SetTextboxNumericInputOnly(textboxOverlayGreenHwnd, true);

		staticLabelOverlayBlueHwnd = AddStaticLabel(formhwnd, _T("Blue"), 200, 170, 50, 20);
		textboxOverlayBlueHwnd = AddTextbox(formhwnd, std::to_wstring(settings.GetOverlayBlue()).c_str(), 235, 170, 30, 20, false, false, false);
		SetTextboxNumericInputOnly(textboxOverlayBlueHwnd, true);

		statusBarHwnd = AddStatusBar(formhwnd, _T("Ready"));


		trayIcon = CreateTrayIcon(formhwnd, _T("ConstDisp"));
		if (trayIcon > -1) {
			AddTrayIconEvent(trayIcon, TrayIconEventsEnum::TRAYICONEVENT_LCLICK, &trayIconClicked);
			AddTrayIconEvent(trayIcon, TrayIconEventsEnum::TRAYICONEVENT_RCLICK, &trayIconRightClicked);
			trayIconMainMenu = CreateTrayIconMainPopupMenu(trayIcon);
			if (trayIconMainMenu) {
				trayIconPopupMenuPause = AddTrayIconPopupMenuItem(trayIconMainMenu, _T("Pause"));
				trayIconPopupMenuStart = AddTrayIconPopupMenuItem(trayIconMainMenu, _T("Start"));
				trayIconPopupMenuStop = AddTrayIconPopupMenuItem(trayIconMainMenu, _T("Stop"));
				trayIconPopupMenuExit = AddTrayIconPopupMenuItem(trayIconMainMenu, _T("Exit"));
				AddTrayIconPopupMenuItemEvent(trayIconPopupMenuPause, TrayIconMenuItemEventsEnum::TRAYICONMENUITEMEVENT_CLICK, &editPauseMenuItemClicked);
				AddTrayIconPopupMenuItemEvent(trayIconPopupMenuStart, TrayIconMenuItemEventsEnum::TRAYICONMENUITEMEVENT_CLICK, &editStartMenuItemClicked);
				AddTrayIconPopupMenuItemEvent(trayIconPopupMenuStop, TrayIconMenuItemEventsEnum::TRAYICONMENUITEMEVENT_CLICK, &editStopMenuItemClicked);
				AddTrayIconPopupMenuItemEvent(trayIconPopupMenuExit, TrayIconMenuItemEventsEnum::TRAYICONMENUITEMEVENT_CLICK, &fileExitMenuItemClicked);
			}
		}

	}
}

void main() {

	MakeUI();

	Engage();

	delete colorModulator;
	delete overlayer;
	delete rgbFieldProducer;

}

// entry point for windows subsystem binaries
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	main();
}
