#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")

struct IPv4
{
	unsigned char b1, b2, b3, b4;
};
bool getMyIP(IPv4 & myIP);
