#include<chrono>
#include<iostream>
#include<thread>
#include<Windows.h>

HHOOK g_lowLevelKeyboardHook;

bool g_postQuitMessage = false;

LRESULT CALLBACK LowLevelKeyboardProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	std::chrono::steady_clock::time_point now_steady_clock = std::chrono::steady_clock::now();
	std::chrono::system_clock::time_point now_system_clock = std::chrono::system_clock::now();

	if (nCode)
		return CallNextHookEx(g_lowLevelKeyboardHook, nCode, wParam, lParam);

	static std::chrono::steady_clock::time_point last_steady_clock;
	static DWORD lastKeyScanCode;

	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
		if (((PKBDLLHOOKSTRUCT)lParam)->scanCode != lastKeyScanCode)
		{
			std::string keyName(12, '\0');
			GetKeyNameText(((PKBDLLHOOKSTRUCT)lParam)->scanCode << 16, keyName.data(), keyName.size());
			keyName.resize(strlen(keyName.data()));

			if (last_steady_clock.time_since_epoch().count())
				std::cout << "\t\t\t\t" << std::chrono::duration_cast<std::chrono::milliseconds>(now_steady_clock - last_steady_clock) << std::endl;
			std::cout << std::chrono::get_tzdb().current_zone()->to_local(now_system_clock) << '\t' << keyName << "\tKeydown" << std::endl;
			last_steady_clock = now_steady_clock;

			lastKeyScanCode = ((PKBDLLHOOKSTRUCT)lParam)->scanCode;
		}
	}
	else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
	{
		std::string keyName(12, '\0');
		GetKeyNameText(((PKBDLLHOOKSTRUCT)lParam)->scanCode << 16, keyName.data(), keyName.size());
		keyName.resize(strlen(keyName.data()));

		std::cout << "\t\t\t\t" << std::chrono::duration_cast<std::chrono::milliseconds>(now_steady_clock - last_steady_clock) << std::endl;
		std::cout << std::chrono::get_tzdb().current_zone()->to_local(now_system_clock) << '\t' << keyName << "\tKeyup" << std::endl;
		last_steady_clock = now_steady_clock;

		lastKeyScanCode = 0;
	}

	if (g_postQuitMessage)
		PostQuitMessage(0);
	return CallNextHookEx(g_lowLevelKeyboardHook, nCode, wParam, lParam);
}

void 函数()
{
	while (getchar() != '0')
		puts("输入0以正常退出程序");
	g_postQuitMessage = true;
}

int main()
{
	g_lowLevelKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
	if (g_lowLevelKeyboardHook == NULL)
	{
		puts("Failed to set hook!");
		return 1;
	}
	std::thread thread(函数);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	thread.join();
	UnhookWindowsHookEx(g_lowLevelKeyboardHook);
	return 0;
}