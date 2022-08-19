#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

class Font
{
public:
	Font(void) = default;
	Font(const wchar_t* lpszName, int iHeight);
	~Font(void);

	operator HFONT(void) { return hFont; }

	inline HFONT Get(void) { return hFont; }

	void Create(const wchar_t* lpszName, int iHeight);

private:
	HFONT hFont = NULL;

	void Destroy(void);
};

