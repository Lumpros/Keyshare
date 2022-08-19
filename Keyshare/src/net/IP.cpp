#include "IP.h"

#include <Windows.h>
#include <winhttp.h>
#include <stdexcept>

#include <iphlpapi.h>

/* Wrapper class for HINTERNET in order to make sure that the handle is closed */
class InternetHandle
{
public:
	inline HINTERNET operator= (HINTERNET hNet) { return (hInternet = hNet); }

	inline operator HINTERNET(void) { return hInternet; }

	InternetHandle(void) = default;
	InternetHandle(HINTERNET hNet) : hInternet{ hNet } {}
	~InternetHandle(void) { if (hInternet) WinHttpCloseHandle(hInternet); }

private:
	HINTERNET hInternet = NULL;
};

static std::string GetRequestResponse(HINTERNET hRequest)
{
	DWORD dwSize = 0, dwDownloaded = 0;
	std::string response = "";

	do
	{
		WinHttpQueryDataAvailable(hRequest, &dwSize);

		char* pszOutBuffer = new char[dwSize + 1];

		if (!pszOutBuffer)
		{
			throw std::runtime_error("Out of memory! Unable to get server response.");
		}

		else
		{
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (!WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded))
			{
				throw std::runtime_error("Error occured while reading data from server.");
			}

			response += pszOutBuffer;

			delete[] pszOutBuffer;
		}

	} while (dwSize > 0);

	return response;
}

/* Sends a GET request to www.myexternalip.com/raw and returns the response */
std::string	GetExternalIPAddress(void)
{
	InternetHandle hSession = WinHttpOpen(L"External IP retriever",
										  WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
										  WINHTTP_NO_PROXY_NAME,
										  WINHTTP_NO_PROXY_BYPASS, 0);

	InternetHandle hConnect = NULL, hRequest = NULL;

	BOOL bResults = FALSE;

	if (!hSession)
	{
		throw std::runtime_error("Unable to initialize WinHTTP session");
	}

	hConnect = WinHttpConnect(hSession, L"myexternalip.com", INTERNET_DEFAULT_HTTPS_PORT, 0);

	if (!hConnect)
	{
		throw std::runtime_error("Unable to connect to the external ip server");
	}

	hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/raw",
								  NULL, WINHTTP_NO_REFERER,
								  WINHTTP_DEFAULT_ACCEPT_TYPES,
								  WINHTTP_FLAG_SECURE);
		
	if (!hRequest)
	{
		throw std::runtime_error("Unable to create GET request.");
	}
	
	bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	if (!(bResults && WinHttpReceiveResponse(hRequest, NULL)))
	{
		throw std::runtime_error("An error occured during communication with the external ip server.");
	}

	return GetRequestResponse(hRequest);	
}

std::string GetInternalIPAddress(void)
{
	char hostname[256];
	gethostname(hostname, 256);

	struct hostent* host = gethostbyname(hostname);

	if (!host) 
	{
		return "";
	}

	UCHAR b1 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b1;
	UCHAR b2 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b2;
	UCHAR b3 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b3;
	UCHAR b4 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b4;

	return std::to_string(b1) + "." + std::to_string(b2) + "." + std::to_string(b3) + "." + std::to_string(b4);
}