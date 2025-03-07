//sonic test

#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>    /* timer */
#else   
#include <sys/time.h>
#endif

#include "sonic.h"

#define SAMPLE_RATE	48000
#define CHANNEL_NUM 2
#define SPEED 2

unsigned long GetHighResolutionTime(void) /* O  time in usec*/
{
#if defined(_WIN32)
    /* Returns a time counter in microsec   */
    /* the resolution is platform dependent */
    /* but is typically 1.62 us resolution  */
    LARGE_INTEGER lpPerformanceCount;
    LARGE_INTEGER lpFrequency;
    QueryPerformanceCounter(&lpPerformanceCount);
    QueryPerformanceFrequency(&lpFrequency);
    return (unsigned long)((1000000*(lpPerformanceCount.QuadPart)) / lpFrequency.QuadPart);
#else
    /* Linux or Mac*/
    struct timeval tv;
    gettimeofday(&tv, 0);
    return((tv.tv_sec*1000000)+(tv.tv_usec));
#endif
}

int main()
{
	printf("sonic test start\n");

	int16_t pInBuf[960] = {0};
	int16_t pOutBuf[960] = {0};

	FILE* pFileIn = fopen("input.pcm", "rb");
	FILE* pFileOut = fopen("out.pcm", "wb");
	if(!pFileIn || !pFileOut)
	{
		printf("open pcm file failed\n");
		return -1;
	}

	sonicStream stream;
	stream = sonicCreateStream(SAMPLE_RATE, CHANNEL_NUM);
	sonicSetSpeed(stream, SPEED);
	sonicSetPitch(stream, 1.0);
	sonicSetVolume(stream, 1.0);
	sonicSetRate(stream, 1.0);
	sonicSetChordPitch(stream, 0);
	sonicSetQuality(stream, 0);

	int nSamplesIn = 0;
	int nSamplesOut = 0;
	int nSamplesTotalIn = 0;
	int nSamplesTotalOut = 0;

	unsigned long startTime, totalTime = 0;

	do
	{
		nSamplesIn = fread(pInBuf, 2*CHANNEL_NUM, SAMPLE_RATE/100, pFileIn);
		nSamplesTotalIn += nSamplesIn;

		startTime = GetHighResolutionTime();

		if(nSamplesIn == 0)
		{
			sonicFlushStream(stream);
		}
		else
		{
			sonicWriteShortToStream(stream, pInBuf, nSamplesIn);
		}
		do
		{
			nSamplesOut = sonicReadShortFromStream(stream, pOutBuf, SAMPLE_RATE/100);
			nSamplesTotalOut += nSamplesOut;
			/*
			if(nSamplesOut > 0)
			{
				fwrite(pOutBuf, 2*CHANNEL_NUM, nSamplesOut, pFileOut);
			}
				*/
		} while (nSamplesOut > 0);

		totalTime += GetHighResolutionTime() - startTime;
		
	} while (nSamplesIn > 0);

	printf("total time: %d\n", totalTime);

	sonicDestroyStream(stream);
	fclose(pFileIn);
	fclose(pFileOut);

	printf("nSamplesTotalIn: %d, nSamplesTotalOut: %d\n", nSamplesTotalIn, nSamplesTotalOut);

	return 0;
}