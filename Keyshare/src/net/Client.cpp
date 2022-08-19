#include "Client.h"

#include <WS2tcpip.h>

#include <cassert>
#include <stdexcept>

Client::Client(SOCKET s)
	: Socket(s)
{
	
}

void Client::Create(void)
{
	struct addrinfo* result, hints;

	Close();
	InitializeAddrinfoHints(&hints);
	ResolveServerAddress(&hints, &result);
	CreateSocket(result);
	Connect(result);
	freeaddrinfo(result);
}

void Client::InitializeAddrinfoHints(struct addrinfo* hints)
{
	ZeroMemory(hints, sizeof(addrinfo));

	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;
}

void Client::ResolveServerAddress(struct addrinfo* hints, struct addrinfo** result)
{
	assert(!port.empty());

	if (getaddrinfo(ipAddress.c_str(), port.c_str(), hints, result))
	{
		throw std::runtime_error("Unable to resolve the localhost information!");
	}
}

void Client::Connect(struct addrinfo* result)
{
	if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		Close();

		throw std::runtime_error("Unable to connect to server! Error Code: " + std::to_string(WSAGetLastError()));
	}
}

void Client::Send(const std::string& data)
{
	assert(sock != INVALID_SOCKET);

	int iResult = send(sock, data.c_str(), static_cast<int>(data.length()), 0);

	if (iResult == SOCKET_ERROR)
	{
		throw std::runtime_error("Unable to send data to server!");
	}

	else if (iResult == 0)
	{
		throw std::runtime_error(std::to_string(WSAGetLastError()));
	}
}