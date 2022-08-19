#include "Sender.h"
#include "VKCoder.h"

#include <assert.h>

void Sender::Connect(const std::string& serverIp)
{
	client.SetPort(APPLICATION_PORT);
	client.SetIPAddress(serverIp);
	client.Create();
}

// This function sends the virtual key code of the key pressed to the server.
// The vkCode parameter is received during the processing of the WM_KEYDOWN message
//
// Returns true if it was sent, otherwise fault.
bool Sender::SendKeyDownToServer(vkcode vkCode)
{
	assert(client != INVALID_SOCKET);

	// Since the Socket class sends and receives strings, we want
	// to send a string whose data contains the vkCode
	char vkString[sizeof(vkCode) + 2] = { 0 };

	VKEncoder encoder;
	encoder.Encode(vkString, vkCode);

	try 
	{
		client.Send(vkString);
		return true;
	}

	catch (std::exception&)
	{
		return false;
	}
}

bool Sender::SendKeyUpToServer(vkcode vkCode)
{
	assert(client != INVALID_SOCKET);

	// Since the Socket class sends and receives strings, we want
	// to send a string whose data contains the vkCode
	char vkString[sizeof(vkCode) + 2] = { 0 };

	VKEncoder encoder;
	encoder.Encode(vkString, vkCode);

	// The second byte is always zero, and since windows is little endian
	// we can set the second character as an exclamation mark, which marks
	// that this should be a key-up message
	vkString[1] = '!';

	try
	{
		client.Send(vkString);
		return true;
	}

	catch (std::exception&)
	{
		return false;
	}
}