#pragma once

#include <Windows.h>

typedef WORD vkcode;

class VKEncoder
{
public:
	// This function assumes that the destination buffer is at least sizeof(vkcode) + 1 bytes big 
	void Encode(void* dest, vkcode vkc);
};

class VKDecoder
{
public:
	vkcode Decode(void* data);
};

