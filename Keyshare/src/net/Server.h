#pragma once

#include "Socket.h"
#include "Client.h"

#include <vector>
#include <memory>
#include <queue>
#include <mutex>

class Receiver;

class ServerSocket : public Socket
{
public:
	void Create(void) override;
	void Receive(SOCKET source, std::string& out);

	SOCKET WaitForIncomingConnection(void);

private:
	inline void InitializeAddrinfoHints(struct addrinfo* hints);
	inline void ResolveServerAddress(struct addrinfo* hints, struct addrinfo** result);
	inline void BindSocket(struct addrinfo* result);
};

class Server
{
public:
	void ListenForConnections(void);
	void ReceiveData(void);

	void SetReceiver(Receiver* pReceiver);
	void SetPort(const std::string& port);

	void Launch(void);
	void Shutdown(void);

	Server() = default;
	Server(const std::string& port);

private:
	ServerSocket sock;
	Receiver* pReceiver = NULL;

	HANDLE hListeningThread = NULL;
	HANDLE hReceivingThread = NULL;

	std::vector<std::unique_ptr<Client>> clients;
	std::string port = "";
	std::mutex clientListMutex;
};