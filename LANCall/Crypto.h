#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "Crypto++/osrng.h"
#include <string>
#include <iostream>
#include <fstream>
#include "Crypto++/cryptlib.h"
#include "Crypto++/hex.h"
#include "Crypto++/filters.h"
#include "Crypto++/aes.h"
#include "Crypto++/modes.h"
using namespace CryptoPP;
using namespace std;

void hex2bytes(char *dst, char* src);
void bytes2hex(char *dst, byte *src, size_t size);
void readKey(string fileName, char *line);

