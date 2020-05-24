#pragma once
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class WindowManager
{
private:
	HWND hwndMain;
	HINSTANCE *hInstance;
	PWSTR *pCmdLine;
	int nCmdShow;
	const wchar_t *CLASS_NAME;
	WNDPROC WindowProc;
	int width;
	int height;
public:
	WindowManager(HINSTANCE *hInstance, PWSTR *pCmdLine, int nCmdShow, const wchar_t CLASS_NAME[], WNDPROC WindowProc, int width, int height);
	int startWindow();

};
