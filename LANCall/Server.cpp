#include "Server.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

DWORD WINAPI Server::serverListen(LPVOID IpParam)
{
	Server *server = (Server*)IpParam;
	if (WSAStartup(MAKEWORD(2, 2), &(server->wsd)) != 0) {
		WSACleanup();
		return -1;
	}
	server->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server->serverAddr.sin_family = AF_INET;
	server->serverAddr.sin_port = htons(server->port);
	server->serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(server->serverSocket, (struct sockaddr *)&(server->serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		closesocket(server->serverSocket);
		WSACleanup();
		return -3;
	}
	if (listen(server->serverSocket, 2) == SOCKET_ERROR) {
		closesocket(server->serverSocket);
		WSACleanup();
		return -4;
	}
	server->started = true;
	int addrSize = sizeof(SOCKADDR_IN);
	server->clientSocket = accept(server->serverSocket, (struct sockaddr *)&(server->clientAddr), &addrSize);

	int size;
	if (server->clientSocket == INVALID_SOCKET)
	{
		closesocket(server->serverSocket);
		WSACleanup();
		return -5;
	}
	else
	{
		server->connected = true;
		while (server->connected)
		{
			size = recv(server->clientSocket, server->inMSG + server->inMsgCount, 2000, 0);
			if (size == SOCKET_ERROR)
			{
				closesocket(server->clientSocket);
				closesocket(server->serverSocket);
				server->connected = false;
				server->started = false;
				break;
			}
			server->inMsgCount += size;
			if (server->inMsgCount >= BUFSIZE - 2000)
			{
				server->finalPos = server->inMsgCount;
				server->inMsgCount = 0;
			}
		}
	}
}

Server::Server(int port)
{
	this->port = port;
	inMsgCount = 0;
	inMsgPos = 0;
	finalPos = BUFSIZE;
	connected = false;
	started = false;
	memset(inMSG, 0, BUFSIZE);
}

void Server::startServer()
{
	//hThread = CreateThread(NULL, 0, th, NULL, 0, &threadID);
	listenThread = CreateThread(NULL, 0, serverListen, this, 0, &listenThreadID);
}

int Server::fetchData(int size, char *buf)
{
	if (inMsgPos > inMsgCount)
	{
		if (inMsgPos + size < finalPos)
		{
			memcpy(buf, inMSG + inMsgPos, size);
			inMsgPos += size;
			return size;
		}
		else
		{
			size = finalPos - inMsgPos;
			memcpy(buf, inMSG + inMsgPos, size);
			inMsgPos = 0;
			return size;
		}
	}
	if (size <= inMsgCount - inMsgPos)
	{
		memcpy(buf, inMSG + inMsgPos, size);
		inMsgPos += size;
		return size;
	}
	if (size > inMsgCount - inMsgPos)
	{
		size = inMsgCount - inMsgPos;
		memcpy(buf, inMSG + inMsgPos, size);
		inMsgPos += size;
		return size;
	}
}

int Server::sendData(char *buf, int size)
{
	return send(clientSocket, buf, size, 0);
}

char* Server::getClientIP()
{
	if (!connected)
	{
		return NULL;
	}
	return inet_ntoa(clientAddr.sin_addr);
}

bool Server::isConnected()
{
	return connected;
}
bool Server::isStarted()
{
	return started;
}

void Server::closeServer()
{
	closesocket(clientSocket);
	closesocket(serverSocket);
	connected = false;
	started = false;
}
