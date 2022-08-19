#include "WinsockInit.h"

#include <WinSock2.h>
#include <stdexcept>
#include <string>

WinsockInit::WinsockInit(void)
{
	WSADATA wsaData;
	const int iError = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iError)
	{
		throw std::runtime_error("WSAStartup failed. Error Code: " + std::to_string(iError));
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();

		throw std::runtime_error("Could not find a usable version of Winsock.dll!");
	}
}

WinsockInit::~WinsockInit(void)
{
	WSACleanup();
}