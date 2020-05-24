#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define CONNECTED 1
#define NOT_CONNECTED 0
#define FAILED -1
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib,"Ws2_32.lib")
#define BUFSIZE 1000000

class Client
{
private:
	SOCKET serverSocket;
	SOCKADDR_IN serverAddr;
	WSADATA wsd;
	int port;
	DWORD listenThreadID;
	HANDLE listenThread;
	char inMSG[BUFSIZE];
	volatile int inMsgCount;			//The position that is going to be write
	volatile int inMsgPos;			//The position that is going to be sent
	volatile int finalPos;
	int connected;
	char ip[20];

	static DWORD WINAPI clientListen(LPVOID IpParam);

public:
	Client(int port, char *ip);
	void startClient();
	int fetchData(int size, char *buf);
	int sendData(char *buf, int size);
	const char* getServerIP();
	int isConnected();
	void closeClient();
};
