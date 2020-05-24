#include "NetUtil.h"
bool getMyIP(IPv4 & myIP)
{
	char szBuffer[1024];

#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 0);
	if (::WSAStartup(wVersionRequested, &wsaData) != 0)
		return false;
#endif


	if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
	{
#ifdef WIN32
		WSACleanup();
#endif
		return false;
	}

	struct hostent *host = gethostbyname(szBuffer);
	if (host == NULL)
	{
#ifdef WIN32
		WSACleanup();
#endif
		return false;
	}

	//Obtain the computer's IP
	myIP.b1 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1;
	myIP.b2 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2;
	myIP.b3 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3;
	myIP.b4 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4;

#ifdef WIN32
	WSACleanup();
#endif
	return true;
}