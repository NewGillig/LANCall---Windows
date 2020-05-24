#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib,"Ws2_32.lib")
#define BUFSIZE 1000000

class Server
{
private:
	SOCKET serverSocket;
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	SOCKADDR_IN serverAddr;
	WSADATA wsd;
	int port;
	DWORD listenThreadID;
	HANDLE listenThread;
	char inMSG[BUFSIZE];
	volatile int inMsgCount;			//The position that is going to be write
	volatile int inMsgPos;			//The position that is going to be sent
	volatile int finalPos;
	bool connected;
	bool started;

	static DWORD WINAPI serverListen(LPVOID IpParam);

public:
	Server(int port);
	void startServer();
	int fetchData(int size, char *buf);
	int sendData(char *buf, int size);
	char* getClientIP();
	bool isConnected();
	bool isStarted();
	void closeServer();
};