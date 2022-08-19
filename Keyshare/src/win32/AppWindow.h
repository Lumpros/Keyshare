#pragma once

#include "net/Receiver.h"
#include "net/Sender.h"

#include "Font.h"
#include "YourIP.h"

#include <Windows.h>
#include <atomic>

class AppWindow
{
public:
	AppWindow(void);
	~AppWindow(void);

	void Show(void);

	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCommand(WPARAM wParam);

private:
	void RegisterKeyshareClass(HINSTANCE hInstance);
	void CreateKeyshareWindow(HINSTANCE hInstance);

	void CreateClientControls(HINSTANCE hInstance);
	void CreateServerControls(HINSTANCE hInstance);

	void OnJoinSessionClicked(void);
	void DisconnectFromSession(void);
	void StopRunningSession(void);
	void CreateNewSession(void);

	LRESULT OnCtlColorStatic(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeydown(WPARAM wParam);
	LRESULT OnKeyup(WPARAM wParam);

private:
	HWND hWnd = NULL;
	HBRUSH hBackgroundBrush = NULL;

	Font uiFont;
	YourIP* pYourIPControl = nullptr;
	Sender* pSender = nullptr;
	Receiver* pReceiver = nullptr;

	std::atomic_bool isHandlingCommand = false;
};

