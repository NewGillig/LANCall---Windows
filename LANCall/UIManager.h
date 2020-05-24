#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include "NetUtil.h"
#include "Client.h"
#include "Server.h"
#include "AudioPlayer.h"
#include "AudioRecorder.h"
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Ws2_32.lib")

#define MAIN_PAGE 0
#define P2P_INIT 1
#define P2P_CALLING 2
#define P2P_RINGING 3
#define P2P_TALKING 4

extern int page;
//Window
extern HWND hwndMain;
//Main page
extern HWND p2pButton;
extern HWND multiButton;
#define P2P_BUTTON 1
#define MULTI_BUTTON 2
//P2P_INIT
extern HWND p2pIpText;
extern HWND p2pEnterText;
extern HWND p2pIpEdit;
extern HWND p2pIpOkButton;
extern HWND p2pBackButton;
#define P2P_IP_TEXT 3
#define P2P_ENTER_TEXT 14
#define P2P_IP_EDIT 4
#define P2P_IP_OK_BUTTON 5
#define P2P_BACK_BUTTON 6
//P2P_CALLING
extern HWND p2pCallingTargetIpText;
extern HWND p2pCallingCancelButton;
#define P2P_CALLING_TARGET_IP_TEXT 7
#define P2P_CALLING_CANCEL_BUTTON 8
//P2P_RINGING
extern HWND p2pRingingTargetIpText;
extern HWND p2pRingingAcceptButton;
extern HWND p2pRingingRefuseButton;
#define P2P_RINGING_TARGET_IP_TEXT 9
#define P2P_RINGING_ACCEPT_BUTTON 10
#define P2P_RINGING_REFUSE_BUTTON 11
//P2P_TALKING
extern HWND p2pTalkingTargetIpText;
extern HWND p2pTalkingEndButton;
#define P2P_TALKING_TARGET_IP_TEXT 12
#define P2P_TALKING_END_BUTTON 13

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
