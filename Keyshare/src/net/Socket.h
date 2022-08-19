#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>

#include <string>

#define APPLICATION_PORT "20075"

class Socket
{
public:
	virtual void Create(void) = 0;

	void SetIPAddress(const std::string& ip) noexcept;
	void SetPort(const std::string& port);
	void Close(void) noexcept;

	inline operator SOCKET(void) { return sock; }
	inline SOCKET Get(void) { return sock; }

	Socket(void) = default;
	Socket(SOCKET s) : sock(s) {}

	~Socket(void) noexcept;

protected:
	void CreateSocket(struct addrinfo* result);

protected:
	SOCKET sock = INVALID_SOCKET;

	std::string port = "";
	std::string ipAddress = "";
};