#include "Socket.h"

#include <WS2tcpip.h>
#include <cassert>
#include <stdexcept>

Socket::~Socket(void) noexcept
{
	Close();
}

void Socket::SetPort(const std::string& port)
{
	assert(!port.empty());

	this->port = port;
}

void Socket::Close(void) noexcept
{
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

void Socket::SetIPAddress(const std::string& ip) noexcept
{
	assert(!ip.empty());

	this->ipAddress = ip;
}

void Socket::CreateSocket(struct addrinfo* result)
{
	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (INVALID_SOCKET == sock)
	{
		freeaddrinfo(result);

		throw std::runtime_error("Unable to create socket. Error Code: " + std::to_string(WSAGetLastError()));
	}
}