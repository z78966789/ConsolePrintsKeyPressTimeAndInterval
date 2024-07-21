#include<chrono>
#include<iostream>
#include<thread>
#include<Windows.h>

HHOOK g_lowLevelKeyboardHook;
HHOOK g_lowLevelMouseHook;

DWORD lastTime;

bool g_postQuitMessage = false;
bool g_running = true;

LRESULT CALLBACK LowLevelKeyboardProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	std::chrono::system_clock::time_point now_system_clock = std::chrono::system_clock::now();

	if (!g_running || nCode)
		return CallNextHookEx(g_lowLevelKeyboardHook, nCode, wParam, lParam);

	static DWORD lastKeyScanCode;
	PKBDLLHOOKSTRUCT pKeyboardStruct = (PKBDLLHOOKSTRUCT)lParam;

	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
	{
		if (pKeyboardStruct->scanCode != lastKeyScanCode)
		{
			std::string keyName(12, '\0');
			GetKeyNameText(pKeyboardStruct->scanCode << 16, keyName.data(), keyName.size());
			keyName.resize(strlen(keyName.data()));

			if (lastTime)
				std::cout << "\t\t\t\t" << pKeyboardStruct->time - lastTime << "ms" << std::endl;
			std::cout << std::chrono::get_tzdb().current_zone()->to_local(now_system_clock) << '\t' << keyName << " Keydown" << std::endl;
			lastTime = pKeyboardStruct->time;

			lastKeyScanCode = pKeyboardStruct->scanCode;
		}
	}
	else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
	{
		std::string keyName(12, '\0');
		GetKeyNameText(pKeyboardStruct->scanCode << 16, keyName.data(), keyName.size());
		keyName.resize(strlen(keyName.data()));

		std::cout << "\t\t\t\t" << pKeyboardStruct->time - lastTime << "ms" << std::endl;
		std::cout << std::chrono::get_tzdb().current_zone()->to_local(now_system_clock) << '\t' << keyName << " Keyup" << std::endl;
		lastTime = pKeyboardStruct->time;

		lastKeyScanCode = 0;
	}

	if (g_postQuitMessage)
		PostQuitMessage(0);
	return CallNextHookEx(g_lowLevelKeyboardHook, nCode, wParam, lParam);
}

const char* WM_MouseToString(WPARAM& wParam, PMSLLHOOKSTRUCT& pMouseStruct)
{
	switch (wParam)
	{
	case WM_LBUTTONDOWN:
		return "\tLeft\tMousedown\tX=";
	case WM_LBUTTONUP:
		return "\tLeft\tMouseup\t\tX=";
	case WM_RBUTTONDOWN:
		return "\tRight\tMousedown\tX=";
	case WM_RBUTTONUP:
		return "\tRight\tMouseup\t\tX=";
	case WM_MBUTTONDOWN:
		return "\tMiddle\tMousedown\tX=";
	case WM_MBUTTONUP:
		return "\tMiddle\tMouseup\t\tX=";
	case WM_XBUTTONDOWN:
		if (pMouseStruct->mouseData >> 16 == XBUTTON1)
			return  "\tX1\tMousedown\tX=";
		return  "\tX2\tMousedown\tX=";
	case WM_XBUTTONUP:
		if (pMouseStruct->mouseData >> 16 == XBUTTON1)
			return  "\tX1\tMouseup\t\tX=";
		return  "\tX2\tMouseup\t\tX=";
	}
}

LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	std::chrono::system_clock::time_point now_system_clock = std::chrono::system_clock::now();

	if (!g_running || nCode)
		return CallNextHookEx(g_lowLevelMouseHook, nCode, wParam, lParam);

	if (wParam != WM_MOUSEMOVE && wParam != WM_MOUSEWHEEL)
	{
		PMSLLHOOKSTRUCT pMouseStruct = (PMSLLHOOKSTRUCT)lParam;
		if (lastTime)
			std::cout << "\t\t\t\t" << pMouseStruct->time - lastTime << "ms" << std::endl;
		std::cout << std::chrono::get_tzdb().current_zone()->to_local(now_system_clock) << WM_MouseToString(wParam, pMouseStruct) << pMouseStruct->pt.x << ",Y=" << pMouseStruct->pt.y << std::endl;
		lastTime = pMouseStruct->time;
	}

	return CallNextHookEx(g_lowLevelKeyboardHook, nCode, wParam, lParam);
}

void handleUserInput()
{
	while (!g_postQuitMessage)
	{
		switch (getchar())
		{
		case 's':case 'S'://suspend
			g_running = false;
			break;
		case 'r':case 'R'://resume
			g_running = true;
			break;
		case 'e':case 'E'://exit
			g_postQuitMessage = true;
			break;
		}
	}
}

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	g_lowLevelKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
	g_lowLevelMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
	if (g_lowLevelKeyboardHook == NULL || g_lowLevelMouseHook == NULL)
	{
		puts("Failed to set hook!");
		return 1;
	}
	std::thread thread(handleUserInput);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	thread.join();
	UnhookWindowsHookEx(g_lowLevelKeyboardHook);
	UnhookWindowsHookEx(g_lowLevelMouseHook);
	return 0;
}