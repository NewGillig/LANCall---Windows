#include "WindowManager.h"

WindowManager::WindowManager(HINSTANCE *hInstance, PWSTR *pCmdLine, int nCmdShow, const wchar_t CLASS_NAME[], WNDPROC WindowProc, int width, int height)
{
	this->hInstance = hInstance;
	this->pCmdLine = pCmdLine;
	this->nCmdShow = nCmdShow;
	this->CLASS_NAME = CLASS_NAME;
	this->WindowProc = WindowProc;
	this->width = width;
	this->height = height;
}

int WindowManager::startWindow()
{
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = *hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);

	hwndMain = CreateWindowEx(0, CLASS_NAME, L"LANCall", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 300, 400,//CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, *hInstance, NULL);
	if (hwndMain == NULL)
	{
		return 0;
	}
	ShowWindow(hwndMain, nCmdShow);




	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}