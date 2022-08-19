#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

namespace util
{
	void RegisterCreateObject(HWND hWnd, LPARAM lParam);

	LPVOID GetRegisteredObject(HWND hWnd);

	void SetWindowFont(HWND hWnd, HFONT hFont);

	double GetWindowScaleForDPI(HWND hWnd);
}