#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define BUFSIZE 1000000
#include <windows.h>
#include <mmsystem.h>
#include <map>
#pragma comment(lib,"winmm.lib")




class AudioRecorder {
private:
	HWAVEIN phwi;
	HWAVEIN hWaveIn;
	WAVEINCAPS wic;
	WAVEHDR wh;
	WAVEFORMATEX wavform;
	char audioBuf[BUFSIZE];
	char whBuf[2000];
	LPWAVEHDR p;
	volatile int recPos;
	volatile int readPos;
	volatile int finalPos;
	volatile bool record;
	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
public:
	
	AudioRecorder();
	void startRecord();
	void stopRecord();
	int fetchData(int size, char* buf);
	bool isRecord();


};

extern int recNum;
extern LPWAVEHDR hdrMapper[20];
extern AudioRecorder *recMapper[20];

