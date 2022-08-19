#include "VKCoder.h"

// Please read the function decleration
void VKEncoder::Encode(void* dest, vkcode vkc)
{
	memcpy_s(dest, sizeof(vkc), &vkc, sizeof(vkc));

	// And of course, we terminate the data because ya never know...
	((unsigned char*)dest)[sizeof(vkc)] = 0;
}

vkcode VKDecoder::Decode(void* data)
{
	vkcode vkc;
	memcpy_s(&vkc, sizeof(vkc), data, sizeof(vkc));
	return vkc;
}