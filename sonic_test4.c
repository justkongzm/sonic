//sonic test, change speed midway, fade in and fade out when changing speed

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


void generateSilentFrameFadeOut(short *lastFrame, int frameSize, short *silentFrame) {
    int i;
    float fadeFactor;

    for (i = 0; i < frameSize; i++) {
        // Calculate fade-out factor (linear fade-out)
        fadeFactor = 1.0f - ((float)i / (float)frameSize);

        // Apply fade-out to each sample
        silentFrame[i] = (short)(lastFrame[i] * fadeFactor);
    }
}

void generateSilentFrameFadeIn(short *lastFrame, int frameSize, short silentFrame[]) {
    int i;
    float fadeFactor;

    for (i = 0; i < frameSize; i++) {
        // Calculate fade-out factor (linear fade-out)
        fadeFactor = ((float)i / (float)frameSize);

        // Apply fade-out to each sample
        silentFrame[i] = (short)(lastFrame[i] * fadeFactor);
    }
}

int main()
{
	printf("sonic test start\n");

	int16_t pLastFrameBuf[FRAMES_10MS*2] = {0};
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

	FILE* pFileTem = fopen("tem.pcm", "wb");

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
	bool bNeedFadeOut = false;
	bool bNeedFadeIn = false;
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
			if(nLastWriteLen > DATA_LEN_10MS)
			{
				printf("fade out\n");
				int16_t temFrame[FRAMES_10MS] = {0};
				generateSilentFrameFadeOut(pLastFrameBuf, FRAMES_10MS, temFrame);
				fwrite(temFrame, 1, DATA_LEN_10MS, pFileOut);

				int nDiffSamples = nReadSamples/SPEED - nWriteSamples;
				int nBlock = nDiffSamples*100/SAMPLE_RATE;
				printf("silence frame count: %d\n", nBlock);
				for(int i = 0; i< nBlock; i++)
				{
					memset((void*)temFrame, 0, DATA_LEN_10MS);
					fwrite(temFrame, 1, DATA_LEN_10MS, pFileOut);
				}

				nLastWriteLen = 0;
				bNeedFadeIn = true;
			}			
			
			bResetSpeed = true;
			printf("reset speed\n");
			sonicDestroyStream(stream);
			stream = sonicCreateStream(SAMPLE_RATE, CHANNEL_NUM);
			sonicSetSpeed(stream, 1.25);
			sonicSetPitch(stream, 1.0);
			sonicSetVolume(stream, 1.0);
			sonicSetRate(stream, 1.0);
			sonicSetChordPitch(stream, 0);
			sonicSetQuality(stream, 0);
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

				// memcpy((void*)pLastFrameBuf + nLastWriteLen, pOutBuf, nOutDataLen);
				// nLastWriteLen += nOutDataLen;
				// if(nLastWriteLen >= DATA_LEN_10MS)
				// {
				// 	fwrite(pLastFrameBuf, 1, DATA_LEN_10MS, pFileOut);
				// 	memmove(pLastFrameBuf, (void*)pLastFrameBuf+DATA_LEN_10MS, nLastWriteLen-DATA_LEN_10MS);
				// 	nLastWriteLen -= DATA_LEN_10MS;
				// }

				if(nOutDataLen > (LAST_FRAME_BUF_LEN - nLastWriteLen))
				{
					nWriteSamples += SAMPLE_RATE/100;
					if(bNeedFadeIn)
					{
						printf("fade in\n");
						int16_t temFrame[FRAMES_10MS] = {0};
						generateSilentFrameFadeIn(pLastFrameBuf, FRAMES_10MS, temFrame);
						fwrite(temFrame, 1, DATA_LEN_10MS, pFileOut);

						memmove(pLastFrameBuf, (void*)pLastFrameBuf+DATA_LEN_10MS, nLastWriteLen-DATA_LEN_10MS);
						nLastWriteLen -= DATA_LEN_10MS;
						bNeedFadeIn = false;
					}
					else
					{
						fwrite(pLastFrameBuf, 1, DATA_LEN_10MS, pFileOut);
						memmove(pLastFrameBuf, (void*)pLastFrameBuf+DATA_LEN_10MS, nLastWriteLen-DATA_LEN_10MS);
						nLastWriteLen -= DATA_LEN_10MS;
					}
				}

				memcpy((void*)pLastFrameBuf + nLastWriteLen, pOutBuf, nOutDataLen);
				nLastWriteLen += nOutDataLen;
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