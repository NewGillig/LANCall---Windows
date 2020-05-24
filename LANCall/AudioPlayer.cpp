#include "AudioPlayer.h"

int playNum=0;
WAVEHDR* playHdrMapper[20];
AudioPlayer *playMapper[20];

int findPlay(WAVEHDR* p)
{
	for (int i = 0; i < 20; i++)
	{
		if (playHdrMapper[i] == p)
			return i;
	}
	return -1;
}

void CALLBACK AudioPlayer::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	WAVEHDR *hdr = (WAVEHDR*)dwParam1;
	if (WOM_DONE == uMsg)
	{
		AudioPlayer *player = playMapper[findPlay(hdr)];
		if (player->play == false)
			return;
		if (player->playPos > player->writePos)
		{
			hdr->dwBufferLength = player->finalPos - player->playPos;
			hdr->lpData = player->audioBuf + player->playPos;
			waveOutWrite(hwo, hdr, sizeof(WAVEHDR));
			player->playPos = 0;
		}
		if (player->writePos - player->playPos > 4000)
		{
			player->playPos += 1000;
		}
		if (player->writePos - player->playPos < 2000)
		{
			hdr->dwBufferLength = 1000;
			hdr->lpData = player->delayBuf;
			waveOutWrite(hwo, hdr, sizeof(WAVEHDR));
			return;
		}
		hdr->dwBufferLength = player->writePos - player->playPos-500;
		hdr->lpData = player->audioBuf + player->playPos;
		waveOutWrite(hwo, hdr, sizeof(WAVEHDR));
		player->playPos += hdr->dwBufferLength-500;
	}
}

AudioPlayer::AudioPlayer()
{
	memset(audioBuf, 0, BUFSIZE);
	memset(delayBuf, 127, 1000);
	playPos = 0;
	writePos = 0;
	finalPos = BUFSIZE;
	play = false;
	wavhdr.lpData = audioBuf;
	wavhdr.dwBufferLength = writePos - playPos;
	wavhdr.dwFlags = 0;
	wavhdr.dwLoops = 0;
	playHdrMapper[playNum] = &wavhdr;
	playMapper[playNum] = this;
}

void AudioPlayer::startPlay()
{
	wavform.wFormatTag = WAVE_FORMAT_PCM;
	wavform.nChannels = 1;
	wavform.nSamplesPerSec = 8000;
	wavform.nAvgBytesPerSec = 8000;
	wavform.nBlockAlign = 1;
	wavform.wBitsPerSample = 8;
	wavform.cbSize = 0;
	int oNum = waveOutGetNumDevs();
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	wavhdr.lpData = delayBuf;
	wavhdr.dwBufferLength = 1000;
	wavhdr.dwFlags = 0;
	wavhdr.dwLoops = 0;
	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));
	play = true;
}

void AudioPlayer::stopPlay()
{
	play = false;
	waveOutClose(hWaveOut);
}

void AudioPlayer::writeData(int size, char *buf)
{
	if (size > BUFSIZE - writePos)
	{
		finalPos = writePos;
		writePos = 0;
		memcpy(audioBuf + writePos, buf, size);
		writePos += size;
		return;
	}
	memcpy(audioBuf + writePos, buf, size);
	writePos += size;
}