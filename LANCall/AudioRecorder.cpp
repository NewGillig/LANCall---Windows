#include "AudioRecorder.h"

int recNum = 0;
LPWAVEHDR hdrMapper[20];
AudioRecorder *recMapper[20];

int findRec(LPWAVEHDR p)
{
	for (int i = 0; i < 20; i++)
	{
		if (hdrMapper[i] == p)
			return i;
	}
	return -1;
}

void CALLBACK AudioRecorder::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;
	if ((WIM_DATA == uMsg))
	{
		AudioRecorder *rec = recMapper[findRec(pwh)];
		memcpy(rec->audioBuf + rec->recPos, pwh->lpData, pwh->dwBytesRecorded);
		rec->recPos += pwh->dwBytesRecorded;
		if (rec->recPos >= BUFSIZE - 2000)
		{
			rec->finalPos = rec->recPos;
			rec->recPos = 0;
		}
		pwh->dwBytesRecorded = 0;
		if (rec->record)
			waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));
	}
}

AudioRecorder::AudioRecorder()
{
	memset(audioBuf, 0, BUFSIZE);
	recPos = 0;
	readPos = 0;
	finalPos = BUFSIZE;
	wh.lpData = whBuf;
	wh.dwBufferLength = 640;
	wh.dwBytesRecorded = 0;
	wh.dwUser = NULL;
	wh.dwFlags = 0;
	wh.dwLoops = 0;
	p = &wh;
	p = new WAVEHDR();
	hdrMapper[recNum] = &wh;
	recMapper[recNum] = this;
	recNum++;
	record = false;
}

void AudioRecorder::startRecord()
{
	wavform.wFormatTag = WAVE_FORMAT_PCM;
	wavform.nChannels = 1;
	wavform.nSamplesPerSec = 8000;
	wavform.nAvgBytesPerSec = 8000;
	wavform.nBlockAlign = 1;
	wavform.wBitsPerSample = 8;
	wavform.cbSize = 0;
	if (!MMSYSERR_NOERROR == waveInOpen(&hWaveIn, WAVE_MAPPER, &wavform, (DWORD_PTR)waveInProc, 0, CALLBACK_FUNCTION))
	{
		return;
	}
	waveInPrepareHeader(hWaveIn, &wh, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &wh, sizeof(WAVEHDR));
	waveInStart(hWaveIn);
	record = true;
}

void AudioRecorder::stopRecord()
{
	record = false;
	waveInStop(hWaveIn);
}

int AudioRecorder::fetchData(int size, char* buf)
{
	if (readPos > recPos)
	{
		if (readPos + size < finalPos)
		{
			memcpy(buf, audioBuf + readPos, size);
			readPos += size;
			return size;
		}
		else
		{
			size = finalPos - readPos;
			memcpy(buf, audioBuf + readPos, size);
			readPos = 0;
			return size;
		}
	}
	if (size <= recPos - readPos)
	{
		memcpy(buf, audioBuf + readPos, size);
		readPos += size;
		return size;
	}
	if (size > recPos - readPos)
	{
		size = recPos - readPos;
		memcpy(buf, audioBuf + readPos, size);
		readPos += size;
		return size;
	}
}

bool AudioRecorder::isRecord()
{
	return record;
}