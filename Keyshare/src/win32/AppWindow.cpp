#include "AppWindow.h"
#include "Utility.h"

#include <thread>
#include <cassert>
#include <stdexcept>
#include <CommCtrl.h>

#include "resource.h"

#define SAFE_DELETE_PTR(ptr) if (ptr) { delete (ptr); ptr = nullptr; }

// When the Join Session button is clicked, it gets the keyboard focus which means that the keyshare window
// procedure doesn't receive any wm_keydown messages. I tried using GetFocus() after this process but it
// doesn't work for some reason, and I don't want to install a low level keyboard hook for privacy reasons.
// For example, imagine youre sharing keys with someone who has notepad open and then you go type your password
// somewhere or send a text message. The other person will be able to read everything you write. For that reason
// the messages will only be sent if the window has the focus.
LRESULT CALLBACK ForwardKeypressProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwRefData)
{
	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP)
	{
		SendMessage(reinterpret_cast<HWND>(dwRefData), uMsg, wParam, lParam);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK KeyshareWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		/* In order for this to work, the last parameter of CreateWindow must be 'this' */
		util::RegisterCreateObject(hWnd, lParam);
		return 0;

	default:
		AppWindow* pWindow = reinterpret_cast<AppWindow*>(util::GetRegisteredObject(hWnd));

		if (pWindow)
		{
			return pWindow->WindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

AppWindow::AppWindow(void)
	: uiFont(L"Segoe UI", 16)
{
	hBackgroundBrush = CreateSolidBrush(RGB(245, 245, 245));

	const HINSTANCE hInstance = GetModuleHandle(NULL);

	RegisterKeyshareClass(hInstance);
	CreateKeyshareWindow(hInstance);

	const double dpiScale = util::GetWindowScaleForDPI(hWnd);
	uiFont.Create(L"Segoe UI", static_cast<int>(round(16 * dpiScale)));

	CreateClientControls(hInstance);
	CreateServerControls(hInstance);

	pYourIPControl = new YourIP(hWnd, POINT{ 
		static_cast<int>(13 * dpiScale),
		static_cast<int>(268 * dpiScale)
	});
}

AppWindow::~AppWindow(void)
{
	SAFE_DELETE_PTR(pYourIPControl);
	SAFE_DELETE_PTR(pReceiver);
	SAFE_DELETE_PTR(pSender);

	DeleteObject(hBackgroundBrush);
}

void AppWindow::RegisterKeyshareClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"KeyshareWindow";
	wcex.lpfnWndProc = KeyshareWindowProc;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(hBackgroundBrush);
	wcex.cbWndExtra = sizeof(AppWindow*); // See CreateKeyshareWindow for an explanation
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wcex))
	{
		throw std::runtime_error("Registration of keyshare window class failed!");
	}
}

void AppWindow::CreateKeyshareWindow(HINSTANCE hInstance)
{
	// We pass 'this' as the last parameter so we can save it in the user data
	// section of the window, so we can retrieve the object inside the global window 
	// procedure and call the procedure that is a member function

	hWnd = CreateWindow(
		L"KeyshareWindow",
		L"Keyshare",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640, 480, // Size doesn't matter here ;) Because we change it right after
		GetDesktopWindow(),
		NULL,
		hInstance,
		this
	);

	if (!hWnd)
	{
		throw std::runtime_error("Keyshare window creation failed!");
	}

	RECT rcWindow, rcClient;
	GetWindowRect(hWnd, &rcWindow);
	GetClientRect(hWnd, &rcClient);

	const int cxWindowFrameSize = ((rcWindow.right - rcWindow.left) - rcClient.right);

	SetWindowPos(hWnd, NULL, NULL, NULL,
		(int)round(246 * util::GetWindowScaleForDPI(hWnd) + cxWindowFrameSize),
		(int)round(332 * util::GetWindowScaleForDPI(hWnd) + cxWindowFrameSize / 2.0),
		SWP_NOZORDER | SWP_NOMOVE
	);
}

void AppWindow::CreateClientControls(HINSTANCE hInstance)
{
	const double dpiScale = util::GetWindowScaleForDPI(hWnd);

	HWND hJoinButton = CreateWindow(
		L"Button",
		L"Join Session",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		static_cast<int>(round(11 * dpiScale)),
		static_cast<int>(round(40 * dpiScale)),
		static_cast<int>(round(224 * dpiScale)),
		static_cast<int>(round(32 * dpiScale)),
		hWnd,
		(HMENU)IDC_JOIN_SESSION_BUTTON,
		hInstance,
		NULL
	);

	util::SetWindowFont(hJoinButton, uiFont);
	SetWindowSubclass(hJoinButton, ForwardKeypressProc, NULL, reinterpret_cast<DWORD_PTR>(hWnd));

	HWND hDisconnectButton = CreateWindow(
		L"Button",
		L"Disconnect",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		static_cast<int>(round(11 * dpiScale)),
		static_cast<int>(round(80 * dpiScale)),
		static_cast<int>(round(224 * dpiScale)),
		static_cast<int>(round(32 * dpiScale)),
		hWnd,
		(HMENU)IDC_DISCONNECT_BUTTON,
		hInstance,
		NULL
	);

	EnableWindow(hDisconnectButton, FALSE);
	util::SetWindowFont(hDisconnectButton, uiFont);
	

	HWND hSessionIp = CreateWindow(
		L"Static",
		L"Session IP",
		WS_CHILD | WS_VISIBLE,
		static_cast<int>(round(13 * dpiScale)),
		static_cast<int>(round(13 * dpiScale)),
		static_cast<int>(round(80 * dpiScale)),
		static_cast<int>(round(24 * dpiScale)),
		hWnd,
		NULL,
		hInstance,
		NULL
	);

	util::SetWindowFont(hSessionIp, uiFont);


	HWND hEditIP = CreateWindow(
		L"Edit",
		L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		static_cast<int>(round(72 * dpiScale)),
		static_cast<int>(round(12 * dpiScale)),
		static_cast<int>(round(160 * dpiScale)),
		static_cast<int>(round(20 * dpiScale)),
		hWnd,
		(HMENU)IDC_IP_EDIT,
		hInstance,
		NULL
	);

	SendMessage(hEditIP, EM_SETCUEBANNER, 0, (LPARAM)L"e.g. 52.85.158.77");
	util::SetWindowFont(hEditIP, uiFont);
}

void AppWindow::CreateServerControls(HINSTANCE hInstance)
{
	const double dpiScale = util::GetWindowScaleForDPI(hWnd);

	HWND hCreateButton = CreateWindow(
		L"Button",
		L"Start Session",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		static_cast<int>(round(11 * dpiScale)),
		static_cast<int>(round(176 * dpiScale)),
		static_cast<int>(round(224 * dpiScale)),
		static_cast<int>(round(32 * dpiScale)),
		hWnd,
		(HMENU)IDC_CREATE_SESSION_BUTTON,
		hInstance,
		NULL
	);

	util::SetWindowFont(hCreateButton, uiFont);

	HWND hStopButton = CreateWindow(
		L"Button",
		L"Stop",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		static_cast<int>(round(11 * dpiScale)),
		static_cast<int>(round(216 * dpiScale)),
		static_cast<int>(round(224 * dpiScale)),
		static_cast<int>(round(32 * dpiScale)),
		hWnd,
		(HMENU)IDC_STOP_SESSION_BUTTON,
		hInstance,
		NULL
	);

	EnableWindow(hStopButton, FALSE);
	util::SetWindowFont(hStopButton, uiFont);

	HWND hCreateInfoText = CreateWindow(
		L"Static",
		L"Session Status:\r\nClients connected:",
		WS_CHILD | WS_VISIBLE,
		static_cast<int>(round(13 * dpiScale)),
		static_cast<int>(round(136 * dpiScale)),
		static_cast<int>(round(120 * dpiScale)),
		static_cast<int>(round(40 * dpiScale)),
		hWnd,
		NULL,
		hInstance,
		NULL
	);

	util::SetWindowFont(hCreateInfoText, uiFont);


	HWND hStatusText = CreateWindow(
		L"Static",
		L"Inactive",
		WS_CHILD | WS_VISIBLE,
		static_cast<int>(round(96 * dpiScale)),
		static_cast<int>(round(136 * dpiScale)),
		static_cast<int>(round(120 * dpiScale)),
		static_cast<int>(round(20 * dpiScale)),
		hWnd,
		(HMENU)IDC_STATUS_TEXT,
		hInstance,
		NULL
	);

	util::SetWindowFont(hStatusText, uiFont);


	HWND hCountText = CreateWindow(
		L"Static",
		L"-",
		WS_CHILD | WS_VISIBLE,
		static_cast<int>(round(116 * dpiScale)),
		static_cast<int>(round(152 * dpiScale)),
		static_cast<int>(round(40 * dpiScale)),
		static_cast<int>(round(20 * dpiScale)),
		hWnd,
		(HMENU)IDC_CLIENT_COUNT_TEXT,
		hInstance,
		NULL
	);

	util::SetWindowFont(hCountText, uiFont);
}

void AppWindow::Show(void)
{
	assert(hWnd);

	ShowWindow(hWnd, SW_SHOW);
}

static void CommandMessageThreadProc(AppWindow* pApp, WPARAM wParam, std::atomic_bool& isHandlingCommand)
{
	pApp->OnCommand(wParam);

	isHandlingCommand = false;
}

LRESULT AppWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(EXIT_SUCCESS);
		return 0;

	case WM_CTLCOLORSTATIC:
		return OnCtlColorStatic(wParam, lParam);

	case WM_COMMAND:
		if (!isHandlingCommand)
		{
			isHandlingCommand = true;
			std::thread commandThread(CommandMessageThreadProc, this, wParam, std::ref(isHandlingCommand));
			commandThread.detach();
		}
		return 0;

	case WM_KEYDOWN:
		return OnKeydown(wParam);

	case WM_KEYUP:
		return OnKeyup(wParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT AppWindow::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
{
	HDC hDC = reinterpret_cast<HDC>(wParam);
	HWND hStaticWnd = reinterpret_cast<HWND>(lParam);
	
	SetBkColor(hDC, RGB(245, 245, 245));

	if (hStaticWnd == GetDlgItem(hWnd, IDC_STATUS_TEXT))
	{
		wchar_t lpszStatusText[64];
		GetWindowText(hStaticWnd, lpszStatusText, 64);

		if (!lstrcmp(lpszStatusText, L"Active")) 
		{
			SetTextColor(hDC, RGB(0, 220, 0));
		}

		else if (!lstrcmp(lpszStatusText, L"Inactive")) 
		{
			SetTextColor(hDC, RGB(230, 0, 0));
		}
	}

	return reinterpret_cast<LRESULT>(hBackgroundBrush);
}

LRESULT AppWindow::OnKeydown(WPARAM wParam)
{
	if (pSender)
	{
		if (!pSender->SendKeyDownToServer(static_cast<vkcode>(wParam)))
		{
			DisconnectFromSession(); 
		}
	}

	return 0;
}

LRESULT AppWindow::OnKeyup(WPARAM wParam)
{
	if (pSender)
	{
		if (!pSender->SendKeyUpToServer(static_cast<vkcode>(wParam)))
		{
			DisconnectFromSession();
		}
	}

	return 0;
}

LRESULT AppWindow::OnCommand(WPARAM wParam)
{
	try 
	{
		switch (LOWORD(wParam))
		{
		case IDC_JOIN_SESSION_BUTTON:
			OnJoinSessionClicked();
			break;

		case IDC_DISCONNECT_BUTTON:
			DisconnectFromSession();
			break;

		case IDC_CREATE_SESSION_BUTTON:
			CreateNewSession();
			break;

		case IDC_STOP_SESSION_BUTTON:
			StopRunningSession();
			break;

		case IDC_COPY_IP_BUTTON:
			pYourIPControl->CopyIpToClipboard();
			break;
		}
	}

	catch (std::exception& e)
	{
		SAFE_DELETE_PTR(pSender);

		// Because we change the title to 'Connecting...' whilst trying to connect,
		// but if we fail to connect we throw an exception so we have to change it
		SetWindowText(hWnd, L"Keyshare");

		// If an exception was thrown, then a fail occured whilst either joining
		// or creating a session, so we want to re-enable the buttons because 
		// we disabled them when we started serving the request.
		// We don't need to worry about the other buttons because they aren't disabled
		// unless the request was successful, which would mean we wouldn't end up
		// here in the first place.
		EnableWindow(GetDlgItem(hWnd, IDC_JOIN_SESSION_BUTTON), TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_CREATE_SESSION_BUTTON), TRUE);

		MessageBoxA(hWnd, e.what(), "Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}

void AppWindow::CreateNewSession(void)
{
	// If the sender object is alive, that means that the user is connected to a session.
	if (pSender)
	{
		if (MessageBox(hWnd, L"If you create a new session you will be automatically disconnected from your current session. Do you wish to proceed?", L"Create Session", MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
		{
			return;
		}
	}

	DisconnectFromSession();
	EnableWindow(GetDlgItem(hWnd, IDC_CREATE_SESSION_BUTTON), FALSE);
	pReceiver = new Receiver(hWnd);
	pReceiver->CreateServer();
	SetWindowText(GetDlgItem(hWnd, IDC_STATUS_TEXT), L"Active");
	SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_COUNT_TEXT), L"0");
	EnableWindow(GetDlgItem(hWnd, IDC_STOP_SESSION_BUTTON), TRUE);
}

void AppWindow::StopRunningSession(void)
{
	SAFE_DELETE_PTR(pReceiver);
	SetWindowText(GetDlgItem(hWnd, IDC_STATUS_TEXT), L"Inactive");
	SetWindowText(GetDlgItem(hWnd, IDC_CLIENT_COUNT_TEXT), L"-");
	EnableWindow(GetDlgItem(hWnd, IDC_CREATE_SESSION_BUTTON), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_STOP_SESSION_BUTTON), FALSE);
}

void AppWindow::DisconnectFromSession(void)
{
	SAFE_DELETE_PTR(pSender);
	EnableWindow(GetDlgItem(hWnd, IDC_DISCONNECT_BUTTON), FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_JOIN_SESSION_BUTTON), TRUE);
}

void AppWindow::OnJoinSessionClicked(void)
{
	if (!pSender)
	{
		char lpszSessionIP[256];
		GetWindowTextA(GetDlgItem(hWnd, IDC_IP_EDIT), lpszSessionIP, 256);

		// Check string isn't empty
		if (lpszSessionIP[0] != '\0')
		{
			if (MessageBox(NULL,
				L"If you attempt to join another session the session you're hosting will be shutdown. Do you wish to proceed?",
				L"Join Session",
				MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				StopRunningSession();

				EnableWindow(GetDlgItem(hWnd, IDC_JOIN_SESSION_BUTTON), FALSE);

				SAFE_DELETE_PTR(pSender);
				pSender = new Sender();

				SetWindowText(hWnd, L"Connecting...");
				pSender->Connect(lpszSessionIP);
				SetWindowText(hWnd, L"Keyshare");

				EnableWindow(GetDlgItem(hWnd, IDC_DISCONNECT_BUTTON), TRUE);
			}
		}
	}
}