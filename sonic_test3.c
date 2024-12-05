//sonic test, change speed midway

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sonic.h"

#define SAMPLE_RATE	48000
#define CHANNEL_NUM 2
#define SPEED 0.75
#define SPEED_CHANGE_10MS_COUNT	100*20
#define FRAMES_10MS	(SAMPLE_RATE/100*CHANNEL_NUM)
#define DATA_LEN_10MS (SAMPLE_RATE/100*CHANNEL_NUM*2)
#define LAST_FRAME_BUF_LEN	(DATA_LEN_10MS*2)
#define FADE_IN_OUT_FRAMES 5


int main()
{
	printf("sonic test start\n");

	int16_t pLastFrameBuf[FRAMES_10MS] = {0};
	int nLastWriteLen = 0;

	int16_t pInBuf[960] = {0};
	int16_t pOutBuf[1920] = {0};
	short buf1[960] = {0};
	for(int i = 0;i<960;i++)
	{
		buf1[i] = 100;
	}
	short buf2[960] = {0};
	for(int i = 0;i<960;i++)
	{
		buf2[i] = 10000;
	}


	FILE* pFileIn = fopen("input.pcm", "rb");
	FILE* pFileOut = fopen("out.pcm", "wb");
	FILE* pFileRef = fopen("ref.pcm", "wb");
	if(!pFileIn || !pFileOut)
	{
		printf("open pcm file failed\n");
		return -1;
	}

	int nSamplesIn = 0;
	int nSamplesOut = 0;
	int n10msCount = 0;

	int nFadingOutFrames = 1;
	int nFadingInFrames = 1;

	sonicStream stream;
	stream = sonicCreateStream(SAMPLE_RATE, CHANNEL_NUM);
	sonicSetSpeed(stream, SPEED);
	sonicSetPitch(stream, 1.0);
	sonicSetVolume(stream, 1.0);
	sonicSetRate(stream, 1.0);
	sonicSetChordPitch(stream, 0);
	sonicSetQuality(stream, 0);


	int nReadSamples = 0;
	int nWriteSamples = 0;
	bool bResetSpeed = false;
	do
	{
		nSamplesIn = fread(pInBuf, 2*CHANNEL_NUM, SAMPLE_RATE/100, pFileIn);
		if(nSamplesIn == 0)
		{
			break;
		}
		if(!bResetSpeed)
		{
			nReadSamples += nSamplesIn;
		}

		n10msCount++;
		if(n10msCount == 1950)
		{
			// sonicFlushStream(stream);
			// do
			// {
			// 	nSamplesOut = sonicReadShortFromStream(stream, pOutBuf, SAMPLE_RATE/100);
			// 	if(nSamplesOut > 0)
			// 	{
			// 		if(nSamplesOut > 960)
			// 			printf("nSamplesOut exceed 960\n");

			// 		fwrite(pOutBuf, 2*CHANNEL_NUM, nSamplesOut, pFileOut);
			// 		nWriteSamples += nSamplesOut;
					
			// 		fwrite(buf1, 2, nSamplesOut, pFileRef);
			// 	}
			// } while (nSamplesOut > 0);
			int nDiffSamples = nReadSamples/SPEED - nWriteSamples;
			int nBlock = nDiffSamples*100/SAMPLE_RATE;

			nLastWriteLen = 0;			
			
			bResetSpeed = true;
			printf("reset speed\n");
			// sonicDestroyStream(stream);
			// stream = sonicCreateStream(SAMPLE_RATE, CHANNEL_NUM);
			sonicSetSpeed(stream, 1.25);
			// sonicSetPitch(stream, 1.0);
			// sonicSetVolume(stream, 1.0);
			// sonicSetRate(stream, 1.0);
			// sonicSetChordPitch(stream, 0);
			// sonicSetQuality(stream, 0);
		}

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
			if(nSamplesOut > 0)
			{
				if(nSamplesOut > 960)
					printf("nSamplesOut exceed 960\n");


				int nOutDataLen = nSamplesOut*CHANNEL_NUM*sizeof(int16_t);
				if(nOutDataLen > (LAST_FRAME_BUF_LEN - nLastWriteLen))
				{
					memmove(pLastFrameBuf, (void*)pLastFrameBuf+DATA_LEN_10MS, nLastWriteLen-DATA_LEN_10MS);
					nLastWriteLen -= DATA_LEN_10MS;
				}
				memcpy((void*)pLastFrameBuf + nLastWriteLen, pOutBuf, nOutDataLen);
				nLastWriteLen += nOutDataLen;


				if(!bResetSpeed)
				{
					nWriteSamples += nSamplesOut;

					fwrite(buf1, 2, nSamplesOut, pFileRef);
				}
				else
				{
					fwrite(buf2, 2, nSamplesOut, pFileRef);
				}


				fwrite(pOutBuf, 2*CHANNEL_NUM, nSamplesOut, pFileOut);
			}
		} while (nSamplesOut > 0);
		
	} while (nSamplesIn > 0);

	printf("read samples: %d, write samples: %d\n", nReadSamples, nWriteSamples);

	sonicDestroyStream(stream);
	fclose(pFileIn);
	fclose(pFileOut);
	fclose(pFileRef);

	return 0;
}