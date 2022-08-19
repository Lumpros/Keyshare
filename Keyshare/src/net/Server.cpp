#include "Server.h"
#include "Receiver.h"
#include "IP.h"

#include <WS2tcpip.h>

#include <cassert>
#include <stdexcept>

void ServerSocket::Create(void)
{
	struct addrinfo *result, hints;

	Close();
	InitializeAddrinfoHints(&hints);
	ResolveServerAddress(&hints, &result);
	CreateSocket(result);
	BindSocket(result);
	freeaddrinfo(result);
}

void ServerSocket::InitializeAddrinfoHints(struct addrinfo* hints)
{
	ZeroMemory(hints, sizeof(addrinfo));

	hints->ai_flags = AI_PASSIVE;
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;
}

void ServerSocket::ResolveServerAddress(struct addrinfo* hints, struct addrinfo** result)
{
	assert(!port.empty());

	if (getaddrinfo(GetInternalIPAddress().c_str(), port.c_str(), hints, result))
	{
		throw std::runtime_error("Unable to resolve the localhost information!");
	}
}

void ServerSocket::BindSocket(struct addrinfo* result)
{
	if (bind(sock, result->ai_addr, static_cast<int>(result->ai_addrlen)))
	{
		freeaddrinfo(result);
		Close();

		throw std::runtime_error("Unable to bind the server socket. Error Code: " + std::to_string(WSAGetLastError()));
	}
}

SOCKET ServerSocket::WaitForIncomingConnection(void)
{
	assert(sock != INVALID_SOCKET);

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
	{
		Close();

		throw std::runtime_error("Failed whilst listening for incoming connections. Error code: " + std::to_string(WSAGetLastError()));
	}

	SOCKET clientSocket = accept(sock, NULL, NULL);

	if (INVALID_SOCKET == clientSocket)
	{
		Close();

		throw std::runtime_error("Unable to accept incoming connection. Error code: " + std::to_string(WSAGetLastError()));
	}

	return clientSocket;
}

void ServerSocket::Receive(SOCKET source, std::string& out)
{
	assert(sock != INVALID_SOCKET);

	constexpr int buffer_size = 64;

	char buffer[buffer_size];
	ZeroMemory(buffer, buffer_size);

	int iResult = 0;

	out.clear();

	do {

		u_long ulBytesRemaining = 0;
		ioctlsocket(source, FIONREAD, &ulBytesRemaining);

		// Check if there is anything else to read, because if there isn't and the connection
		// doesn't close, then the program will block during the following call to recv.
		if (ulBytesRemaining == 0)
		{
			// At this point we also want to test if the reason that there aren't any bytes
			// to read is because the client has disconnected. Now, we could do this with
			// the send or recv functions, but we'd have to connect to the client if we were
			// to send data and the recv call would block the thread if it weren't disconnected
			// So we will use the WSAPoll function instead.
			WSAPOLLFD poll = {};
			poll.fd = source;
			poll.events = POLLRDNORM;
			WSAPoll(&poll, 1, 0);

			if (poll.revents == POLLHUP)
			{
				throw std::runtime_error("Client Disconnected");
			}

			break;
		}

		iResult = recv(source, buffer, buffer_size, 0);

		if (iResult < 0)
		{
			throw std::runtime_error("Error occured whilst receiving data from server. Error Code: " + std::to_string(WSAGetLastError()));
		}

		out.append(buffer);

	} while (iResult > 0);
}

Server::Server(const std::string& port)
{
	SetPort(port);
}

void Server::SetReceiver(Receiver* pReceiver)
{
	assert(pReceiver);

	this->pReceiver = pReceiver;
}

void Server::SetPort(const std::string& port)
{
	assert(!port.empty());

	this->port = port;

	sock.SetPort(port);
}

static DWORD WINAPI ListenForIncomingConnections(LPVOID data)
{
	Server* pServer = reinterpret_cast<Server*>(data);

	try 
	{
		while (true)
		{
			pServer->ListenForConnections();
		}
	}

	catch (std::exception&)
	{
		pServer->Shutdown();
		return FALSE;
	}

	return TRUE;
}

static DWORD WINAPI ReceiveMessagesFromClients(LPVOID data)
{
	Server* pServer = reinterpret_cast<Server*>(data);

	try 
	{
		while (true)
		{
			pServer->ReceiveData();

			Sleep(10);
		}
	}

	catch (std::exception&)
	{
		pServer->Shutdown();
		return FALSE;
	}

	return TRUE;
}

void Server::Launch(void)
{
	assert(pReceiver);

	Shutdown();

	sock.Create();

	hListeningThread = CreateThread(
		NULL,
		0,
		ListenForIncomingConnections,
		this,
		CREATE_SUSPENDED,
		NULL
	);

	if (hListeningThread)
	{
		hReceivingThread = CreateThread(
			NULL,
			0,
			ReceiveMessagesFromClients,
			this,
			CREATE_SUSPENDED,
			NULL
		);

		if (hReceivingThread)
		{
			ResumeThread(hListeningThread);
			ResumeThread(hReceivingThread);
		}
	}
}

void Server::Shutdown(void)
{
	if (hListeningThread != NULL)
	{
		TerminateThread(hListeningThread, EXIT_SUCCESS);
		hListeningThread = NULL;
	}

	if (hReceivingThread != NULL)
	{
		TerminateThread(hReceivingThread, EXIT_SUCCESS);
		hReceivingThread = NULL;
	}

	clients.clear();
	sock.Close();

	if (pReceiver)
	{
		pReceiver->NotifyUIOfShutdown();
	}
}

void Server::ListenForConnections(void)
{
	SOCKET s = sock.WaitForIncomingConnection();

	clientListMutex.lock();

	auto const& it = std::find_if(clients.begin(), clients.end(), [&](std::unique_ptr<Client>& c) {
		return c->Get() == s;
	});

	if (it == clients.end())
	{
		clients.push_back(std::move(std::make_unique<Client>(s)));

		if (pReceiver)
		{
			pReceiver->UpdateUIClientCount(clients.size());
		}
	}

	clientListMutex.unlock();
}

void Server::ReceiveData(void)
{
	clientListMutex.lock();

	for (size_t i = 0; i < clients.size(); ++i)
	{
		try
		{
			std::string clientMessage;
			sock.Receive(clients[i]->Get(), clientMessage);

			if (!clientMessage.empty())
			{
				pReceiver->ProcessData(clientMessage);
			}
		}

		catch (std::exception& e)
		{
			UNREFERENCED_PARAMETER(e);

			clients.erase(clients.cbegin() + i);
				
			// Since we remove an element, during the next iteration i will be incremented by 1
			// but that might be out of bounds because we erased an element, so we decrement
			// it by 1 to make sure we get the next element.
			--i;

			if (pReceiver)
			{
				pReceiver->UpdateUIClientCount(clients.size());
			}
		}
	}

	clientListMutex.unlock();
}