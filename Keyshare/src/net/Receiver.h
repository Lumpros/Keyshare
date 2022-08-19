#pragma once

#include "Server.h"
#include "Client.h"
#include "VKCoder.h"

// The receiver sets up a server that receives the keys pressed.
// The server then sends the keys to the receiver which in turn
// sends the keys to the operating system.
class Receiver
{
public:
	Receiver(HWND hWnd) : hTopWindow(hWnd) {}

	void CreateServer(void);
	void ProcessData(const std::string& data);

	void NotifyUIOfShutdown(void);
	void UpdateUIClientCount(size_t count);

	~Receiver(void);

private:
	Server server;

	HWND hTopWindow = NULL;

	void SendKeyToSystem(vkcode vkc);
};

