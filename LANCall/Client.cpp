#include "Client.h"



DWORD WINAPI Client::clientListen(LPVOID IpParam)
{
	Client *client = (Client*)IpParam;
	if (WSAStartup(MAKEWORD(2, 2), &(client->wsd)) != 0) {
		WSACleanup();
		return -1;
	}
	client->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (client->serverSocket == INVALID_SOCKET) {
		WSACleanup();
		return -2;
	}
	client->serverAddr.sin_family = AF_INET;
	client->serverAddr.sin_port = htons(client->port);
	client->serverAddr.sin_addr.S_un.S_addr = inet_addr(client->ip);
	if (connect(client->serverSocket, (struct sockaddr*)&(client->serverAddr), sizeof(client->serverAddr)) < 0) {
		client->connected = -1;
		closesocket(client->serverSocket);
		WSACleanup();
		return -3;
	}
	else {
		client->connected = CONNECTED;
		while (client->connected == CONNECTED)
		{
			int size = recv(client->serverSocket, client->inMSG + client->inMsgCount, 2000, 0);
			if (size == SOCKET_ERROR)
			{
				closesocket(client->serverSocket);
				client->connected = -1;
				break;
			}
			client->inMsgCount += size;
			if (client->inMsgCount >= BUFSIZE - 2000)
			{
				client->finalPos = client->inMsgCount;
				client->inMsgCount = 0;
			}
		}
	}
}

Client::Client(int port, char *ip)
{
	this->port = port;
	strcpy_s(this->ip, ip);
	inMsgCount = 0;
	inMsgPos = 0;
	finalPos = BUFSIZE;
	connected = false;
	memset(inMSG, 0, BUFSIZE);
}

void Client::startClient()
{
	listenThread = CreateThread(NULL, 0, clientListen, this, 0, &listenThreadID);
}

int Client::fetchData(int size, char *buf)
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

int Client::sendData(char *buf, int size)
{
	return send(serverSocket, buf, size, 0);
}

const char* Client::getServerIP()
{
	if (!connected)
	{
		return NULL;
	}
	return ip;
}

int Client::isConnected()
{
	return connected;
}

void Client::closeClient()
{
	connected = -1;
	closesocket(serverSocket);
	WSACleanup();
}
