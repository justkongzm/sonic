//sonic test

#include <stdio.h>
#include <stdint.h>

#include "sonic.h"

#define SAMPLE_RATE	48000
#define CHANNEL_NUM 2
#define SPEED 2

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

	do
	{
		nSamplesIn = fread(pInBuf, 2*CHANNEL_NUM, SAMPLE_RATE/100, pFileIn);
		nSamplesTotalIn += nSamplesIn;
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
			if(nSamplesOut > 0)
			{
				fwrite(pOutBuf, 2*CHANNEL_NUM, nSamplesOut, pFileOut);
			}
		} while (nSamplesOut > 0);
		
	} while (nSamplesIn > 0);

	sonicDestroyStream(stream);
	fclose(pFileIn);
	fclose(pFileOut);

	printf("nSamplesTotalIn: %d, nSamplesTotalOut: %d\n", nSamplesTotalIn, nSamplesTotalOut);

	return 0;
}