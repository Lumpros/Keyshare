#include "Receiver.h"
#include "win32/resource.h"

// We want to not only create a server socket, but two threads.
// One thread will listen for connections and accept them, whilst
// the other thread listens for data from the clients.
void Receiver::CreateServer(void)
{
	server.SetPort(APPLICATION_PORT);
	server.SetReceiver(this);
	server.Launch();
}

void Receiver::ProcessData(const std::string& data)
{
	VKDecoder decoder;
	SendKeyToSystem(decoder.Decode((void*)data.c_str()));
}

void Receiver::SendKeyToSystem(vkcode vkc)
{
	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vkc;
	SendInput(1, &input, sizeof(INPUT));
}

void Receiver::NotifyUIOfShutdown(void)
{
	if (hTopWindow)
	{
		SendMessage(hTopWindow, WM_COMMAND, IDC_STOP_SESSION_BUTTON, NULL);
	}
}

void Receiver::UpdateUIClientCount(size_t count)
{
	if (hTopWindow)
	{
		wchar_t lpszCount[32];
		_itow_s(static_cast<int>(count), lpszCount, 32, 10);
		SetWindowText(GetDlgItem(hTopWindow, IDC_CLIENT_COUNT_TEXT), lpszCount);
	}
}

Receiver::~Receiver(void)
{
	server.Shutdown();
}