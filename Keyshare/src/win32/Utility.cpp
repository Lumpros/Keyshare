#include "Utility.h"

void util::RegisterCreateObject(HWND hWnd, LPARAM lParam)
{
	const LPCREATESTRUCT pCreate = reinterpret_cast<LPCREATESTRUCT>(lParam);

	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
}

LPVOID util::GetRegisteredObject(HWND hWnd)
{
	return reinterpret_cast<void*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

void util::SetWindowFont(HWND hWnd, HFONT hFont)
{
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

double util::GetWindowScaleForDPI(HWND hWnd)
{
	return GetDpiForWindow(hWnd) / 100.0 + 0.05;
}