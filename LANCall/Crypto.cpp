#include "Crypto.h"

void bytes2hex(char *dst, byte *src, size_t size)
{
	string encoded;
	StringSource(src, size, true, new HexEncoder(new StringSink(encoded)));
	const char *hex = encoded.c_str();
	strcpy(dst, hex);
}

void hex2bytes(char *dst, char* src)
{
	string decoded;
	StringSource(src, true, new HexDecoder(new StringSink(decoded)));
	const char *bytes = decoded.c_str();
	memcpy(dst, bytes,strlen(src)/2);
}



void writeALine(string fileName, char *line)
{
	ofstream dstFile(fileName, ios::out);
	dstFile << line;
	dstFile.close();
}

void readKey(string fileName, char *line)
{
	ifstream srcFile(fileName, ios::in);
	string out;
	srcFile >> out;
	srcFile.close();
	const char* temp = out.c_str();
	strcpy(line, temp);
}


//void EncryptTest()
//{
//	//prng.GenerateBlock(key, AES::MAX_KEYLENGTH);
//	memset(iv, 0, AES::BLOCKSIZE);
//	char keyHex[128];
//	memset(keyHex, 0, 128);
//	readALine("key.ini", keyHex);
//	hex2bytes(key, keyHex);
//
//
//	byte plainText[128] = "Hello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello WorldHello World";
//	byte cipherText[128];
//	memset(cipherText, 0, 128);
//	size_t messageLen = strlen((char*)plainText);
//
//	CFB_Mode<AES>::Encryption cfbEncryption(key, AES::MAX_KEYLENGTH, iv);
//	cfbEncryption.ProcessData(cipherText, plainText, messageLen);
//
//	byte plainText2[128];
//	memset(plainText2, 0, 128);
//	CFB_Mode<AES>::Decryption cfbDecryption(key, AES::MAX_KEYLENGTH, iv);
//	cfbDecryption.ProcessData(plainText2, cipherText, 50);
//	cfbDecryption.ProcessData(plainText2 + 50, cipherText + 50, 50);
//
//	//char textHex[64];
//	//bytes2hex(textHex, plainText, messageLen);
//
//
//
//
//	//
//	//byte plainText2[64];
//	//hex2bytes(plainText2, textHex);
//
//
//
//}