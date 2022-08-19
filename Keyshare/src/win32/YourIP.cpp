#include "YourIP.h"

#include "net/IP.h"
#include "resource.h"

#include "Utility.h"

#include <thread>

static void SetWindowTextAsLoadingDots(HWND hWnd, std::atomic_bool& stopThread)
{
	char dots[4];
	ZeroMemory(dots, sizeof(dots));

	unsigned short iDotCount = 0;

	while (!stopThread)
	{
		if (++iDotCount == 4)
		{
			iDotCount = 1;
		}

		memset(dots, '.', iDotCount * sizeof(char));
		dots[iDotCount] = '\0';
		SetWindowTextA(hWnd, dots);
		Sleep(500);
	}
}

static void FillYourIPControl(YourIP* pYourIP, std::atomic_bool& isRunning)
{
	isRunning.store(true);

	// We'll use this variable to communicate with the dot thread
	// in order to stop it when we've acquired the IP address.
	std::atomic_bool stopDotThread = false;

	// Begin the fancy . .. ... thread!!!
	std::thread dotThread(SetWindowTextAsLoadingDots, pYourIP->Get(), std::ref(stopDotThread));

	while (isRunning)
	{
		try
		{
			const std::string ip = GetExternalIPAddress();
			
			// Once we get the user's external IP address we can stop the dot thread
			// because the text will be replaced with the IP. So we let the thread
			// know that it should stop...
			stopDotThread.store(true);

			// ...and then we wait for it to receive the message and actually stop
			dotThread.join();

			// Once it has actually stopped we change the actual text of the static window...
			SetWindowTextA(pYourIP->Get(), ip.c_str());

			// ...and then finally announce that this thread is done executing
			isRunning.store(false);
		}

		catch (std::exception&)
		{
			// Ugh... Here's the thing. Whenever we call WinHttpSendRequest, the WinHttp API allocates
			// some stuff globally in the background, so it appears as if there is a memory leak, when in
			// reality there is not. For that reason we sleep for a small duration so that:
			// 
			//		1. We don't exhaust the processor
			//		2. We don't constantly allocate the forementioned resources.
			// 
			// Although the size of the allocated resources isn't much, I'd rather avoid the allocations anyway.
			// I suspect that if the allocation doesn't happen often then some resources may be
			// deallocated later, but I'm not sure. I'll have to look into that.
			//
			// This could be avoided if we link the library dynamically instead of statically and then
			// unload the library forcing it to deallocate the resources, but I'm too lazy to implement that.
			Sleep(1000);
		}
	}
}

YourIP::YourIP(HWND hParentWindow, POINT ptTopLeft)
{
	isThreadRunning = false;

	const double dpiScale = util::GetWindowScaleForDPI(hParentWindow);

	uiFont.Create(L"Segoe UI", 16 * dpiScale);

	hYourIpText = CreateWindow(
		L"Static",
		L"Your IP Address: ",
		WS_CHILD | WS_VISIBLE,
		ptTopLeft.x,
		ptTopLeft.y,
		static_cast<int>(round(96 * dpiScale)),
		static_cast<int>(round(30 * dpiScale)),
		hParentWindow,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	util::SetWindowFont(hYourIpText, uiFont);

	hIpStatic = CreateWindow(
		L"Static",
		L"...",
		WS_CHILD | WS_VISIBLE,
		ptTopLeft.x + static_cast<int>(round(96 * dpiScale)),
		ptTopLeft.y,
		static_cast<int>(round(80 * dpiScale)),
		static_cast<int>(round(30 * dpiScale)),
		hParentWindow,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	util::SetWindowFont(hIpStatic, uiFont);

	hCopyBtn = CreateWindow(
		L"Button",
		L"Copy",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		ptTopLeft.x + static_cast<int>(round(176 * dpiScale)),
		ptTopLeft.y - static_cast<int>(round(4 * dpiScale)),
		static_cast<int>(round(48 * dpiScale)),
		static_cast<int>(round(24 * dpiScale)),
		hParentWindow,
		(HMENU)IDC_COPY_IP_BUTTON,
		GetModuleHandle(NULL),
		NULL
	);

	util::SetWindowFont(hCopyBtn, uiFont);

	Refresh();
}

void YourIP::Refresh(void)
{
	// If the thread is already running there is no reason to start it again.
	if (hIpStatic && !isThreadRunning.load())
	{
		std::thread ipLoadingThread(FillYourIPControl, this, std::ref(isThreadRunning));

		ipLoadingThread.detach();
	}
}

void YourIP::CopyIpToClipboard(void)
{
	// If the thread isn't running then the IP address has been
	// retrieved, therefore the hIpStatic window text contains
	// the user's external IP address.
	if (!isThreadRunning)
	{
		if (OpenClipboard(NULL))
		{
			wchar_t externalIpAddress[128];
			GetWindowText(hIpStatic, externalIpAddress, 128);

			const int len = GetWindowTextLength(hIpStatic);

			HGLOBAL hDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(wchar_t));

			if (hDest)
			{
				LPWSTR lpszDest = (LPWSTR)GlobalLock(hDest);

				if (lpszDest)
				{
					memcpy(lpszDest, externalIpAddress, len * sizeof(wchar_t));
					lpszDest[len] = 0;

					GlobalUnlock(hDest);

					EmptyClipboard();
					SetClipboardData(CF_UNICODETEXT, hDest);
				}
			}

			CloseClipboard();
		}
	}
}