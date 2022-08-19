// If you remove the macro below the universe will literally explode
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <stdexcept>
#include <CommCtrl.h>

#include "net/WinsockInit.h"
#include "win32/AppWindow.h"
#include "net/IP.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "ComCtl32.lib")

#pragma comment(linker,"\"/manifestdependency:type = 'win32' \
                                              name = 'Microsoft.Windows.Common-Controls' \
                                           version = '6.0.0.0' \
                             processorArchitecture = '*' \
                                    publicKeyToken = '6595b64144ccf1df' \
                                          language = '*'\"")

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	InitCommonControls();

	try
	{
		WinsockInit winsockInit;
		AppWindow window;
		
		window.Show();
		
		MSG msg = {};
		
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return EXIT_SUCCESS;
	}

	catch (std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);

		return EXIT_FAILURE;
	}
}