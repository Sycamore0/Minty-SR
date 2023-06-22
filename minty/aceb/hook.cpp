#include "hook.h"
#include "utils.h"
#include "../kiero/minhook/include/MinHook.h"

#include <iostream>
#include <windows.h>
#pragma comment(lib, "aceb/MinHook.lib")

typedef HANDLE(WINAPI* CREATE_FILE_W)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

namespace hook {

	CREATE_FILE_W p_CreateFileW = nullptr;
	CREATE_FILE_W t_CreateFileW;

	HANDLE WINAPI h_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	{
		if (memcmp(lpFileName, L"\\\\.\\ACE-BASE", 12) == 0) {
			wprintf(L"Thread (%i) attempting to communicate with anti-cheat driver -> %s\n", GetCurrentThreadId(), lpFileName);

			SuspendThread(GetCurrentThread()); // 200iq bypass for memory protection
		}

		return p_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

}
namespace {
	void initHookBypass(int a) {
		if (MH_Initialize() != MH_OK)
		{
			printf("Error initializing MinHook library\n");

			return;
		}

		if (MH_CreateHookApiEx(L"kernelbase", "CreateFileW", &hook::h_CreateFileW, reinterpret_cast<void**>(&hook::p_CreateFileW), reinterpret_cast<void**>(&hook::t_CreateFileW)) != MH_OK)
		{
			printf("Error creating hook for CreateFileW function\n");

			return;
		}

		if (MH_EnableHook(hook::t_CreateFileW) != MH_OK)
		{
			printf("Error enabling hook for CreateFileW function\n");

			return;
		}
	}
}
