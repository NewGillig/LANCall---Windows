#include "UIManager.h"

int page;
//Window
HWND hwndMain;
//Main page
HWND p2pButton;
HWND multiButton;
//P2P_INIT
HWND p2pIpText;
HWND p2pEnterText;
HWND p2pIpEdit;
HWND p2pIpOkButton;
HWND p2pBackButton;
//P2P_CALLING
HWND p2pCallingTargetIpText;
HWND p2pCallingCancelButton;
//P2P_RINGING
HWND p2pRingingTargetIpText;
HWND p2pRingingAcceptButton;
HWND p2pRingingRefuseButton;
//P2P_TALKING
HWND p2pTalkingTargetIpText;
HWND p2pTalkingEndButton;


struct IPv4 ip;

Server *msgServer = NULL;
Server *voiceServer = NULL;
Client *msgClient = NULL;
Client *voiceClient = NULL;

volatile int mode = 0;		//0-Server, 1-Client
volatile int serverStatus = 0;		// 0 not work, 1 waiting, 2 ringing, 3 talking -1 destroyed
volatile int clientStatus = -1;		// 0 calling, 1 talking -1 destroyed
AudioRecorder audioRecorder;
AudioPlayer audioPlayer;

char acceptCall[10] = { 1,8,9,3,1,2,2,6 };
char endCall[10] = { 1,9,0,4,0,8,2,2 };

DWORD serverListenerThreadID;
HANDLE serverListenerThread;
DWORD clientListenerThreadID;
HANDLE clientListenerThread;

void switchPage(int pagex);


AutoSeededRandomPool prng;
CryptoPP::byte key[AES::MAX_KEYLENGTH];
CryptoPP::byte iv[AES::BLOCKSIZE];

CFB_Mode<AES>::Encryption *cfbEncryption;
CFB_Mode<AES>::Decryption *cfbDecryption;










DWORD WINAPI serverListener(LPVOID IpParam)
{
	char cipherDataBuf[200000];
	memset(cipherDataBuf, 0, 200000);
	int place = 0;
	while (1)
	{
		Sleep(1);
		char msg[10];
		memset(msg, 0, 10);
		if (serverStatus != -1&&msgServer != NULL && voiceServer != NULL && msgServer->isConnected() && voiceServer->isConnected())
		{	
			msgServer->fetchData(8, msg);
			if (mode == 0 && serverStatus == 1 )
			{
				switchPage(P2P_RINGING);
				serverStatus = 2;
			}
			if (mode == 0 && serverStatus == 2 )
			{
				if (memcmp(msg, endCall, 8) == 0)
				{
					switchPage(P2P_INIT);
				}
			}
			if (mode == 0 && serverStatus == 3 )
			{
				char voiceBuf[4000];
				char encryptVoiceBuf[4000];

				int size = 0;
				memset(voiceBuf, 0, 4000);
				memset(encryptVoiceBuf, 0, 4000);
				size = voiceServer->fetchData(4000, encryptVoiceBuf);
				if (place > 180000)
				{
					place = 0;
					memset(cipherDataBuf, 0, 200000);
				}
				if (size > 0)
				{
					memcpy(cipherDataBuf + place, encryptVoiceBuf, size);
					place += size;
				}
				int start = -1;
				int end = -1;
				for (int i = 0; i < place - 9; i++)
				{
					if (cipherDataBuf[i] == '1'&&cipherDataBuf[i + 1] == '2'&&cipherDataBuf[i + 2] == '3'&&cipherDataBuf[i + 3] == '4'&&cipherDataBuf[i + 4] == '5'&&cipherDataBuf[i + 5] == '6'&&cipherDataBuf[i + 6] == '7'&&cipherDataBuf[i + 7] == '8'&&cipherDataBuf[i + 8] == '9')
					{
						start = i + 9;
					}
					if (start != -1 && cipherDataBuf[i] == '9'&&cipherDataBuf[i + 1] == '8'&&cipherDataBuf[i + 2] == '7'&&cipherDataBuf[i + 3] == '6'&&cipherDataBuf[i + 4] == '5'&&cipherDataBuf[i + 5] == '4'&&cipherDataBuf[i + 6] == '3'&&cipherDataBuf[i + 7] == '2'&&cipherDataBuf[i + 8] == '1')
					{
						end = i;
						cfbDecryption = new CFB_Mode<AES>::Decryption(key, AES::MAX_KEYLENGTH, iv);
						cfbDecryption->ProcessData((byte*)voiceBuf, (byte*)cipherDataBuf + start, end - start);
						delete cfbDecryption;
						audioPlayer.writeData(end - start, voiceBuf);
						memcpy(cipherDataBuf, cipherDataBuf + end + 9, place - end - 9);
						place -= end + 9;
						break;
					}
				}
				char recBuf[4000];
				char encryptRecBuf[4000];
				char sendBuf[4018];
				size = 0;
				memset(recBuf, 0, 4000);
				memset(encryptRecBuf, 0, 4000);
				memset(sendBuf, 0, 4018);
				size = audioRecorder.fetchData(4000, recBuf);
				if (size > 0) {
					cfbEncryption = new CFB_Mode<AES>::Encryption(key, AES::MAX_KEYLENGTH, iv);
					cfbEncryption->ProcessData((byte*)encryptRecBuf, (byte*)recBuf, size);
					delete cfbEncryption;
					memcpy(sendBuf, "123456789", 9);
					memcpy(sendBuf + 9, encryptRecBuf, size);
					memcpy(sendBuf + 9 + size, "987654321", 9);
					voiceServer->sendData(sendBuf, size+18);
				}

				if (memcmp(msg, endCall, 8) == 0)
				{
					switchPage(P2P_INIT);
				}
			}
		}
	}
	return 0;
}

DWORD WINAPI clientListener(LPVOID IpParam)
{
	char cipherDataBuf[200000];
	memset(cipherDataBuf, 0, 200000);
	int place = 0;
	while (1)
	{
		Sleep(1);
		char msg[10];
		memset(msg, 0, 10);
		if (clientStatus != -1&& msgClient != NULL && voiceClient != NULL)
		{
			msgClient->fetchData(8, msg);
		}
		if (mode == 1)
		{
			if (memcmp(msg, endCall, 8) == 0)
			{
				switchPage(P2P_INIT);
			}
			if (clientStatus==0 && memcmp(msg, acceptCall, 8) == 0)
			{
				clientStatus = 1;
				switchPage(P2P_TALKING);
			}
			if (clientStatus == 1)
			{
				char voiceBuf[4000];
				char encryptVoiceBuf[4018];
				memset(voiceBuf, 0, 4000);
				memset(encryptVoiceBuf, 0, 4018);
				int size = 0;
				size = voiceClient->fetchData(4018, encryptVoiceBuf);
				if (size > 0) {
					memcpy(cipherDataBuf + place, encryptVoiceBuf, size);
					place += size;
				}
				int start = -1;
				int end = -1;
				for (int i = 0; i < place - 9; i++)
				{
					if (cipherDataBuf[i] == '1'&&cipherDataBuf[i + 1] == '2'&&cipherDataBuf[i + 2] == '3'&&cipherDataBuf[i + 3] == '4'&&cipherDataBuf[i + 4] == '5'&&cipherDataBuf[i + 5] == '6'&&cipherDataBuf[i + 6] == '7'&&cipherDataBuf[i + 7] == '8'&&cipherDataBuf[i + 8] == '9')
					{
						start = i + 9;
					}
					if (start != -1 && cipherDataBuf[i] == '9'&&cipherDataBuf[i + 1] == '8'&&cipherDataBuf[i + 2] == '7'&&cipherDataBuf[i + 3] == '6'&&cipherDataBuf[i + 4] == '5'&&cipherDataBuf[i + 5] == '4'&&cipherDataBuf[i + 6] == '3'&&cipherDataBuf[i + 7] == '2'&&cipherDataBuf[i + 8] == '1')
					{
						end = i;
						cfbDecryption = new CFB_Mode<AES>::Decryption(key, AES::MAX_KEYLENGTH, iv);
						cfbDecryption->ProcessData((byte*)voiceBuf, (byte*)cipherDataBuf + start, end - start);
						delete cfbDecryption;
						audioPlayer.writeData(end - start, voiceBuf);
						memcpy(cipherDataBuf, cipherDataBuf + end + 9, place - end - 9);
						place -= end + 9;
						break;
					}
				}
					
				char recBuf[4000];
				char encryptRecBuf[4000];
				char sendBuf[4018];
				memset(recBuf, 0, 4000);
				memset(encryptRecBuf, 0, 4000);
				memset(sendBuf, 0, 4018);
				size = 0;
				size = audioRecorder.fetchData(4000, recBuf);
				if (size > 0)
				{
					cfbEncryption = new CFB_Mode<AES>::Encryption(key, AES::MAX_KEYLENGTH, iv);
					cfbEncryption->ProcessData((byte*)encryptRecBuf, (byte*)recBuf, size);
					delete cfbEncryption;
					memcpy(sendBuf, "123456789", 9);
					memcpy(sendBuf + 9, encryptRecBuf, size);
					memcpy(sendBuf + 9 + size, "987654321", 9);
					voiceClient->sendData(sendBuf, size+18);
				}
			}
		}
	}
	return 0;
}




void reset()
{
	audioPlayer.stopPlay();
	audioRecorder.stopRecord();
	mode = 0;
	serverStatus = -1;
	clientStatus = -1;
	if (msgServer != NULL && voiceServer != NULL)
	{
		msgServer->closeServer();
		voiceServer->closeServer();
		Sleep(500);
		delete msgServer;
		delete voiceServer;
		msgServer = NULL;
		voiceServer = NULL;
	}
	if (msgClient != NULL && voiceClient != NULL)
	{
		msgClient->closeClient();
		voiceClient->closeClient();
		Sleep(500);
		delete msgClient;
		delete voiceClient;
		msgClient = NULL;
		voiceClient = NULL;
	}
}

void switchPage(int pagex)
{
	page = pagex;
	ShowWindow(p2pButton, SW_HIDE);
	ShowWindow(multiButton, SW_HIDE);
	ShowWindow(p2pIpText, SW_HIDE);
	ShowWindow(p2pEnterText, SW_HIDE);
	ShowWindow(p2pIpEdit, SW_HIDE);
	ShowWindow(p2pIpOkButton, SW_HIDE);
	ShowWindow(p2pBackButton, SW_HIDE);
	ShowWindow(p2pCallingTargetIpText, SW_HIDE);
	ShowWindow(p2pCallingCancelButton, SW_HIDE);
	ShowWindow(p2pRingingTargetIpText, SW_HIDE);
	ShowWindow(p2pRingingAcceptButton, SW_HIDE);
	ShowWindow(p2pRingingRefuseButton, SW_HIDE);
	ShowWindow(p2pTalkingTargetIpText, SW_HIDE);
	ShowWindow(p2pTalkingEndButton, SW_HIDE);
	if (pagex == MAIN_PAGE)
	{
		ShowWindow(p2pButton, SW_SHOW);
		//ShowWindow(multiButton, SW_SHOW);
	}
	else if (pagex == P2P_INIT)
	{
		reset();
		msgServer = new Server(27777);
		voiceServer = new Server(28888);
		msgServer->startServer();
		voiceServer->startServer();
		serverStatus = 1;
		getMyIP(ip);
		wchar_t ipStr[64];
		wsprintf(ipStr, L"Your IP:\n%d,%d,%d,%d", ip.b1, ip.b2, ip.b3, ip.b4);
		SetWindowText(p2pIpText, ipStr);
		SetWindowText(p2pEnterText, L"Enter Peer's IP");
		ShowWindow(p2pIpText, SW_SHOW);
		ShowWindow(p2pEnterText, SW_SHOW);
		ShowWindow(p2pIpEdit, SW_SHOW);
		ShowWindow(p2pIpOkButton, SW_SHOW);
		ShowWindow(p2pBackButton, SW_SHOW);
	}
	else if (pagex == P2P_CALLING)
	{
		mode = 1;
		const char *ip = msgClient->getServerIP();
		wchar_t text[64];
		wchar_t ipStr[64];
		size_t ipLen = strlen(ip);
		mbstowcs(ipStr, ip, ipLen);
		ipStr[ipLen] = 0;
		wsprintf(text, L"Calling: %s\0", ipStr);
		SetWindowText(p2pCallingTargetIpText, text);
		ShowWindow(p2pCallingTargetIpText, SW_SHOW);
		ShowWindow(p2pCallingCancelButton, SW_SHOW);
	}
	else if (pagex == P2P_RINGING)
	{
		mode = 0;
		const char *ip = msgServer->getClientIP();
		wchar_t text[64];
		wchar_t ipStr[64];
		size_t ipLen = strlen(ip);
		mbstowcs(ipStr, ip, ipLen);
		ipStr[ipLen] = 0;
		wsprintf(text, L"%s\nis calling you\0", ipStr);
		SetWindowText(p2pRingingTargetIpText, text);
		ShowWindow(p2pRingingTargetIpText, SW_SHOW);
		ShowWindow(p2pRingingAcceptButton, SW_SHOW);
		ShowWindow(p2pRingingRefuseButton, SW_SHOW);
	}
	else if (pagex == P2P_TALKING)
	{
		audioPlayer.startPlay();
		audioRecorder.startRecord();
		wchar_t text[64];
		if (mode == 0)
		{
			const char *ip = msgServer->getClientIP();
			wchar_t ipStr[64];
			size_t ipLen = strlen(ip);
			mbstowcs(ipStr, ip, ipLen);
			ipStr[ipLen] = 0;
			wsprintf(text, L"Talking: %s\0", ipStr);
		}
		if (mode == 1)
		{
			const char *ip = msgClient->getServerIP();
			wchar_t ipStr[64];
			size_t ipLen = strlen(ip);
			mbstowcs(ipStr, ip, ipLen);
			ipStr[ipLen] = 0;
			wsprintf(text, L"Talking: %s\0", ipStr);
		}
		SetWindowText(p2pTalkingTargetIpText, text);
		ShowWindow(p2pTalkingTargetIpText, SW_SHOW);
		ShowWindow(p2pTalkingEndButton, SW_SHOW);
	}
}





LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{


	switch (uMsg)
	{
		case WM_CREATE:
		{
			hwndMain = hwnd;

			memset(iv, 0, 16);
			char keyHex[128];
			memset(keyHex, 0, 128);
			readKey("key.ini", keyHex);
			char keyChar[32];
			hex2bytes(keyChar, keyHex);
			memcpy(key, keyChar, 32);




			HFONT defaultFont;
			//defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			defaultFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

			p2pButton = CreateWindow(L"BUTTON", L"P2PCall", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 140, 200, 30, hwndMain, (HMENU)P2P_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			multiButton = CreateWindow(L"BUTTON", L"MultiCall", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 190, 200, 30, hwndMain, (HMENU)MULTI_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(p2pButton, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(multiButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

			p2pIpText = CreateWindow(L"STATIC", L"Your IP: 255.255.255.255", ES_CENTER | WS_VISIBLE | WS_CHILD, 43, 50, 200, 30, hwndMain, (HMENU)P2P_IP_TEXT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pEnterText = CreateWindow(L"STATIC", L"Enter Peer's IP", WS_VISIBLE | WS_CHILD, 100, 130, 200, 40, hwndMain, (HMENU)P2P_ENTER_TEXT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pIpEdit = CreateWindow(L"EDIT", L"", ES_CENTER | WS_VISIBLE | WS_CHILD, 67, 170, 150, 16, hwndMain, (HMENU)P2P_IP_EDIT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pIpOkButton = CreateWindow(L"BUTTON", L"Call", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 250, 80, 25, hwndMain, (HMENU)P2P_IP_OK_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pBackButton = CreateWindow(L"BUTTON", L"Back", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 150, 250, 80, 25, hwndMain, (HMENU)P2P_BACK_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(p2pIpText, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pEnterText, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pIpEdit, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pIpOkButton, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pBackButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

			p2pCallingTargetIpText = CreateWindow(L"STATIC", L"Calling: 255.255.255.255", ES_CENTER | WS_VISIBLE | WS_CHILD, 45, 80, 200, 30, hwndMain, (HMENU)P2P_CALLING_TARGET_IP_TEXT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pCallingCancelButton = CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 105, 250, 80, 25, hwndMain, (HMENU)P2P_CALLING_CANCEL_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(p2pCallingTargetIpText, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pCallingCancelButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

			p2pRingingTargetIpText = CreateWindow(L"STATIC", L"192.168.2.2\n is calling you", ES_CENTER | WS_VISIBLE | WS_CHILD, 40, 60, 200, 40, hwndMain, (HMENU)P2P_RINGING_TARGET_IP_TEXT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pRingingAcceptButton = CreateWindow(L"BUTTON", L"Accept", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 250, 80, 25, hwndMain, (HMENU)P2P_RINGING_ACCEPT_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pRingingRefuseButton = CreateWindow(L"BUTTON", L"Refuse", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 150, 250, 80, 25, hwndMain, (HMENU)P2P_RINGING_REFUSE_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(p2pRingingTargetIpText, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pRingingAcceptButton, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pRingingRefuseButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

			p2pTalkingTargetIpText = CreateWindow(L"STATIC", L"Talking: 255.255.255.255", ES_CENTER | WS_VISIBLE | WS_CHILD, 45, 80, 200, 30, hwndMain, (HMENU)P2P_TALKING_TARGET_IP_TEXT, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			p2pTalkingEndButton = CreateWindow(L"BUTTON", L"Hang up", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 105, 250, 80, 25, hwndMain, (HMENU)P2P_TALKING_END_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(p2pTalkingTargetIpText, WM_SETFONT, WPARAM(defaultFont), TRUE);
			SendMessage(p2pTalkingEndButton, WM_SETFONT, WPARAM(defaultFont), TRUE);

			switchPage(MAIN_PAGE);

			serverListenerThread = CreateThread(NULL, 0, serverListener, 0, 0, &serverListenerThreadID);
			clientListenerThread = CreateThread(NULL, 0, clientListener, 0, 0, &clientListenerThreadID);

			

			return 0;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case P2P_BUTTON:
				{
					switchPage(P2P_INIT);
					break;
					audioRecorder.startRecord();
					audioPlayer.startPlay();
					while (1)
					{
						char buf[640];
						audioRecorder.fetchData(640, buf);
						audioPlayer.writeData(640, buf);
					}

				}
				case P2P_BACK_BUTTON:
				{
					reset();
					switchPage(MAIN_PAGE);
					break;
				}
				case P2P_IP_OK_BUTTON:
				{

					wchar_t ipStrw[20];
					char ipStr[20];
					GetWindowText(p2pIpEdit, ipStrw, 20);
					SetWindowText(p2pEnterText, L"Enter Peer's IP\n  Connecting...");
					wcstombs(ipStr, ipStrw, 20);
					msgClient = new Client(27777, ipStr);
					voiceClient = new Client(28888, ipStr);
					msgClient->startClient();
					while (msgClient->isConnected() == NOT_CONNECTED);
					Sleep(500);
					voiceClient->startClient();
					while (voiceClient->isConnected() == NOT_CONNECTED);
					if (msgClient->isConnected() != CONNECTED || voiceClient->isConnected() != CONNECTED)
					{
						SetWindowText(p2pEnterText, L"Enter Peer's IP\nConnect Failed");
						Sleep(1000);
						switchPage(P2P_INIT);
						break;
					}
					mode = 1;		//client mode
					clientStatus = 0;	//calling

					switchPage(P2P_CALLING);
					break;
				}
				case P2P_CALLING_CANCEL_BUTTON:
				{
					msgClient->sendData(endCall, 8);
					switchPage(P2P_INIT);
					break;
				}
				case P2P_RINGING_ACCEPT_BUTTON:
				{
					msgServer->sendData(acceptCall, 8);
					serverStatus = 3;
					switchPage(P2P_TALKING);
					break;
				}
				case P2P_RINGING_REFUSE_BUTTON:
				{
					msgServer->sendData(endCall, 8);
					switchPage(P2P_INIT);
					break;
				}
				case P2P_TALKING_END_BUTTON:
				{
					if (mode == 1)
						msgClient->sendData(endCall,8);
					else
						msgServer->sendData(endCall,8);
					switchPage(P2P_INIT);
					break;
				}
			}
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			COLORREF color = RGB(240, 240, 240);
			HBRUSH hcolor = CreateSolidBrush(color);

			FillRect(hdc, &ps.rcPaint, hcolor);

			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_CLOSE:
		{
			if (MessageBox(hwnd, L"Quit?", L"LANCall", MB_OKCANCEL) == IDOK)
			{
				DestroyWindow(hwnd);
			}
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}













////HWND hwndMain;
//
////HWND hwndButton;
//
////HWND hwndText;
//
//DWORD threadID;
//DWORD threadID2;
//HANDLE hThread;
//HANDLE hThread2;
//
//HWAVEIN phwi;
//
//WAVEHDR wh;
//HWAVEIN hWaveIn;
//
//WAVEHDR wavhdr;
//HWAVEOUT hWaveOut;
//WAVEHDR wavhdr2;
//HWAVEOUT hWaveOut2;
//
//SOCKET serverSocket;
//SOCKET cSocket;
//SOCKADDR_IN cAddr;
//
//SOCKET clientSocket;
//SOCKADDR_IN client;
//
//
//
//char cOutMSG[20000];
//bool first;
//
//
//
//char playBuffer[200000];
////std::queue<char> playQueue;
//
//volatile int count = 0;
//volatile int pos = 0;


//void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2)
//{
//	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;
//	
//	if ((WIM_DATA == uMsg))
//	{
//		/*for (int i = 0; i < pwh->dwBytesRecorded; i++)
//		{
//			playQueue.push(pwh->lpData[i]);
//		}*/
//		/*if (count > 195000)
//		{
//			count = 0;
//			pos = 0;
//		}*/
//		send(clientSocket, pwh->lpData, pwh->dwBytesRecorded, 0);
//		/*memcpy(playBuffer + count, pwh->lpData, pwh->dwBytesRecorded);
//		count += pwh->dwBytesRecorded;*/
//		wh.dwBytesRecorded = 0;
//		waveInAddBuffer(hWaveIn, &wh, sizeof(WAVEHDR));
//		
//	}
//}
//
//void CALLBACK waveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2)
//{
//	if (WOM_DONE == uMsg)
//	{
//		//waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//		/*while (playQueue.size() < 1024);
//		for (int i = 0; i < 768; i++)
//		{
//			playBuffer[i] = playQueue.front();
//			playQueue.pop();
//		}*/
//		//wavhdr.lpData = playBuffer+pos;
//		if (!first)
//		{
//			wavhdr.dwBufferLength = count - pos;
//			wavhdr.lpData = playBuffer + pos;
//			waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//			pos += count - pos;
//		}
//	}
//}
//
//
//DWORD WINAPI th(LPVOID IpParam)
//{
//	int dNum = waveInGetNumDevs();
//	for (int i = 0; i < dNum; i++)
//	{
//		WAVEINCAPS wic;
//		waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
//
//	}
//
//
//	WAVEFORMATEX wavform;
//	wavform.wFormatTag = WAVE_FORMAT_PCM;
//	wavform.nChannels = 1;
//	wavform.nSamplesPerSec = 8000;
//	wavform.nAvgBytesPerSec = 8000;
//	wavform.nBlockAlign = 1;
//	wavform.wBitsPerSample = 8;
//	wavform.cbSize = 0;
//
//	if (!MMSYSERR_NOERROR == waveInOpen(&hWaveIn, WAVE_MAPPER, &wavform, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION))
//	{
//		return 0;
//	}
//
//
//	wh.lpData = new char[640];
//	wh.dwBufferLength = 640;
//	wh.dwBytesRecorded = 0;
//	wh.dwUser = NULL;
//	wh.dwFlags = 0;
//	wh.dwLoops = 0;
//
//	waveInPrepareHeader(hWaveIn, &wh, sizeof(WAVEHDR));
//	waveInAddBuffer(hWaveIn, &wh, sizeof(WAVEHDR));
//
//	waveInStart(hWaveIn);
//
//
//	WSADATA wsd;
//	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
//		WSACleanup();
//		return -1;
//	}
//	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//	if (clientSocket == INVALID_SOCKET) {
//		WSACleanup();
//		return -2;
//	}
//	client.sin_family = AF_INET;
//	client.sin_port = htons(27777);
//	client.sin_addr.S_un.S_addr = inet_addr("192.168.2.100");
//	Sleep(1000);
//	if (connect(clientSocket, (struct sockaddr*) &client, sizeof(client)) < 0) {
//		closesocket(clientSocket);
//		WSACleanup();
//		return -3;
//	}
//	else {
//		memset(cOutMSG, 0, 20000);
//		
//	}
//
//
//
//	/*while (count < 50000);
//
//	waveInStop(hWaveIn);
//	waveInReset(hWaveIn);
//	waveInUnprepareHeader(hWaveIn, &wh, sizeof(WAVEHDR));
//	waveInClose(hWaveIn);*/
//}
//DWORD WINAPI th2(LPVOID IpParam)
//{
//	WAVEFORMATEX wavform;
//	wavform.wFormatTag = WAVE_FORMAT_PCM;
//	wavform.nChannels = 1;
//	wavform.nSamplesPerSec = 8000;
//	wavform.nAvgBytesPerSec = 8000;
//	wavform.nBlockAlign = 1;
//	wavform.wBitsPerSample = 8;
//	wavform.cbSize = 0;
//	int oNum = waveOutGetNumDevs();
//	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
//	
//
//
//
//	WSADATA wsd;
//	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
//		WSACleanup();
//		return -1;
//	}
//
//
//	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	SOCKADDR_IN server;
//	server.sin_family = AF_INET;
//	server.sin_port = htons(27777);
//	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//
//	if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
//	{
//		closesocket(serverSocket);
//		WSACleanup();
//		return -3;
//	}
//	if (listen(serverSocket, 2) == SOCKET_ERROR) {
//		closesocket(serverSocket);
//		WSACleanup();
//		return -4;
//	}
//
//
//	int addrSize = sizeof(SOCKADDR_IN);
//	char inMSG[20000];
//	int size;
//
//	cSocket = accept(serverSocket, (struct sockaddr *)&cAddr, &addrSize);
//	if (cSocket == INVALID_SOCKET)
//	{
//		closesocket(serverSocket);
//		WSACleanup();
//		return -5;
//	}
//	else
//	{
//		first = true;
//		while (1)
//		{
//			memset(inMSG, 0, 20000);
//			size = recv(cSocket, inMSG, 2000, 0);
//			if (size == SOCKET_ERROR)
//			{
//				closesocket(cSocket);
//				break;
//			}
//			if (first)
//			{
//				if (count - pos > 5000)
//				{
//					wavhdr.lpData = playBuffer + pos;
//					wavhdr.dwBufferLength = 4000;
//					wavhdr.dwFlags = 0;
//					wavhdr.dwLoops = 0;
//					waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//					waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//					pos +=4000;
//					first = false;
//				}
//			}
//			if (count > 195000)
//			{
//				count = 0;
//				pos = 0;
//				first = true;
//			}
//			memcpy(playBuffer + count, inMSG, size);
//			count += size;
//		}
//	}
//
//	//for (int i = 0; i < 64; i++)
//	//{
//	//	playBuffer[i] = playQueue.front();
//	//	playQueue.pop();
//	//}
//
//	
//	/*while (count - pos < 4000);
//	wavhdr.lpData = playBuffer + pos;
//	wavhdr.dwBufferLength = 2000;
//	wavhdr.dwFlags = 0;
//	wavhdr.dwLoops = 0;
//	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//
//	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//	pos += 2000;*/
//	//while (1)
//	//{
//	//	while (playQueue.size() < 64);
//	//	for (int i = 0; i < 64; i++)
//	//	{
//	//		playBuffer[i] = playQueue.front();
//	//		playQueue.pop();
//	//	}
//	//	//wavhdr.lpData = playBuffer;
//	//	//waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
//	//}
//
///*
//	waveOutOpen(&hWaveOut2, WAVE_MAPPER, &wavform, 0, 0, CALLBACK_NULL);
//
//
//	wavhdr2.lpData = playBuffer+20000;
//	wavhdr2.dwBufferLength = 30000;
//	wavhdr2.dwFlags = 0;
//	wavhdr2.dwLoops = 0;
//	waveOutPrepareHeader(hWaveOut2, &wavhdr2, sizeof(WAVEHDR));
//
//	waveOutWrite(hWaveOut2, &wavhdr2, sizeof(WAVEHDR));
//	*/
//
//
//	return 0;
//
//}
