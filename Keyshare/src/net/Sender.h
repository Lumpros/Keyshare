#pragma once

#include "Client.h"
#include "VKCoder.h"

#include <string>

class Sender
{
public:
	void Connect(const std::string& serverIp);
	bool SendVKCodeToServer(vkcode vkCode);

private:
	Client client;
};

