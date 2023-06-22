#include "utils.h"
#include "bypass.h"
#include "../kiero/minhook/include/MinHook.h"
#include <iostream>
#include <windows.h>
typedef HANDLE(WINAPI* CREATE_FILE_W)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
namespace bypass {
	uint32_t LDR_LOAD_DLL = 0xFCDC0;
	uint32_t CHEAT_ENGINE = 0xF9940;

	uint64_t base_address = 0;

	bool check() {
		uint64_t checksum = 0;
			
		checksum ^= utils::read<uint64_t>(base_address + LDR_LOAD_DLL);
		checksum ^= utils::read<uint64_t>(base_address + CHEAT_ENGINE);

		if (checksum < 0x75000000ull || checksum > 0x76000000ull) {
			printf("Unsupported SRB version -> 0x%I64X\n", checksum);
			return FALSE;
		}

		return TRUE;
	}

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

	void init() {

		while ((base_address = reinterpret_cast<uint64_t>(GetModuleHandleA("starrailbase.dll"))) == 0);

		utils::suspend_other_threads();

		if (check()) {
			utils::write<uint64_t>(base_address + LDR_LOAD_DLL, 0xCCCCC300000000B8ull); // bypass for dll injections (speedhack in cheat engine / debuggers)
			utils::write<uint8_t>(base_address + CHEAT_ENGINE, 0xC3); // bypass for cheat engine
		}
		else {
			printf("Failed to init bypass\n");

			Sleep(5000);
			TerminateProcess(GetCurrentProcess(), 0);
		}

		utils::resume_other_threads();

		if (MH_Initialize() != MH_OK)
		{
			printf("Error initializing MinHook library\n");

			return;
		}

		if (MH_CreateHookApiEx(L"kernelbase", "CreateFileW", &bypass::h_CreateFileW, reinterpret_cast<void**>(&bypass::p_CreateFileW), reinterpret_cast<void**>(&bypass::t_CreateFileW)) != MH_OK)
		{
			printf("Error creating hook for CreateFileW function\n");

			return;
		}

		if (MH_EnableHook(bypass::t_CreateFileW) != MH_OK)
		{
			printf("Error enabling hook for CreateFileW function\n");

			return;
		}
	}
}
