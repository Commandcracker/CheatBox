#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <codecvt>

DWORD GetProcId(const std::wstring& procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!procName.compare(procEntry.szExeFile))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

bool fileExists(const std::string& filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}

bool InjectDLL(const std::wstring& procName, const char* dllPath)
{
	DWORD procId = GetProcId(procName);
	if (!procId || !fileExists(dllPath))
	{
		return false;
	}
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

	if (hProc && hProc != INVALID_HANDLE_VALUE)
	{
		void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);

		HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

		if (hThread)
		{
			CloseHandle(hThread);
		}
	}

	if (hProc)
	{
		CloseHandle(hProc);
	}
	return true;
}

int main(int argc, char* argv[])
{
	SetConsoleTitleA("Cheat Box");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hConsole, &csbiInfo);

	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	std::wstring procName;
	LPSTR dllPath = NULL;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	if (argc > 1)
	{
		procName = converter.from_bytes(argv[1]);
		if (argc > 2)
		{
			dllPath = _fullpath(dllPath, argv[2], _MAX_PATH);
		}
	}

	if (procName.empty()) {
		std::string temp1;
		std::cout << "Enter Process Name:" << std::endl;

		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
		getline(std::cin, temp1);
		
		procName = converter.from_bytes(temp1);
	}

	if (GetProcId(procName))
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "Process Found" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << "Process Not Found" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	if (dllPath == NULL) {
		std::string temp2;
		std::cout << "Enter dll Path:" << std::endl;

		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
		getline(std::cin, temp2);

		dllPath = _fullpath(dllPath, temp2.c_str(), _MAX_PATH);
	}

	if (fileExists(dllPath))
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "dll Found" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << "dll Not Found" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	std::cout << "Injecting ..." << std::endl;

	if (InjectDLL(procName, dllPath))
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		std::cout << "Successful Injected!" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
		std::cout << "Injection Failed!" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, csbiInfo.wAttributes);
	return 0;
}
