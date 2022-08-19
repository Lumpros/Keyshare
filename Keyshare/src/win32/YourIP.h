#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <atomic>

#include "Font.h"

class YourIP
{
public:
	YourIP(HWND hParentWindow, POINT ptTopLeft);

	inline HWND Get(void) { return hIpStatic; }

	void Refresh(void);
	void CopyIpToClipboard(void);

private:
	HWND hIpStatic = nullptr;
	HWND hCopyBtn = nullptr;
	HWND hYourIpText = nullptr;

	Font uiFont;

	std::atomic_bool isThreadRunning = false;
};
