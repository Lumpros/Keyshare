#pragma once

#include "Socket.h"

class Client : public Socket
{
public:
	Client() = default;
	Client(SOCKET s);

	void Create(void) override;
	void Send(const std::string& data);

private:
	inline void InitializeAddrinfoHints(struct addrinfo* hints);
	inline void ResolveServerAddress(struct addrinfo* hints, struct addrinfo** result);
	inline void Connect(struct addrinfo* result);
};

