#include "Font.h"

Font::Font(const wchar_t* lpszName, int iHeight)
{
	Create(lpszName, iHeight);
}

Font::~Font(void)
{
	Destroy();
}

void Font::Create(const wchar_t* lpszName, int iHeight)
{
	Destroy();

	hFont = CreateFont(
		iHeight, 0,
		0, 0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_DONTCARE,
		lpszName
	);
}

void Font::Destroy(void)
{
	if (hFont)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
}