#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define BUFSIZE 1000000
#include <windows.h>
#include <mmsystem.h>
#include <map>
#pragma comment(lib,"winmm.lib")

class AudioPlayer
{
private:
	WAVEHDR wavhdr;
	HWAVEOUT hWaveOut;
	WAVEFORMATEX wavform;
	char audioBuf[BUFSIZE];
	volatile int playPos;
	volatile int writePos;
	volatile int finalPos;
	volatile bool play;
	char delayBuf[1000];
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
public:
	AudioPlayer();
	void startPlay();
	void stopPlay();
	void writeData(int size, char *buf);

};

extern int playNum;
extern WAVEHDR* playHdrMapper[20];
extern AudioPlayer *playMapper[20];
