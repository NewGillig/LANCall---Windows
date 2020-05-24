// LANCall.cpp : Defines the entry point for the application.
//

#include"LANCall.h"

WindowManager *windowManager;
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	windowManager = new WindowManager(&hInstance,&pCmdLine,nCmdShow,L"LanCall", WindowProc,300,300);
	windowManager->startWindow();
}


