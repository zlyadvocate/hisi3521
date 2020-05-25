/*
 * hi_faac.h
 *
 *  Created on: May 14, 2014
 *      Author: root
 */

#ifndef HI_FAAC_H_
#define HI_FAAC_H_

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "samplerate.h"
#include "layer3.h"

#include <hi_type.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

#define AUDIOINFREQ  16000
#define AUDIOENCFREQ 11025
#define FRAMEINLENGTH 480

#define DEFAULT_CONVERTER SRC_SINC_FASTEST
#define AACFRAMELENGTH    2048
typedef struct  _strPcmBuff
{
	short buf[AACFRAMELENGTH+1];
	int iCount;
}  PcmBuff;

typedef struct Audio_config{
	int format;
	int sampleRate;
	int channels;
	int bitrate;
	int bitsPerSample;
}audioconfig;


unsigned char l16_to_pcmu(int sample);
int g711_encode2(void *in_buf, int in_size, void *out_buf, int *out_size);
HI_BOOL  readOneG711Packet(FILE* fp,unsigned  char * buf);
short  encodeG711ToShort( unsigned  char alaw);
void   encodeG711Packet(unsigned char* buf ,short*  outPacket);
void ReinitAAC();
void Init_Voaac();
void SBSInit();
void CloseRate();
void initaudiobuf();
void CloseShine();

void Init_AudioSocket();
//HI_BOOL  FilllInRateChangePacket(short *  pBuffer);

HI_BOOL FillAACPacket(short * pBuffer,FILE *mp3File,HI_BOOL audiostreamingflag);
#if 0
HI_BOOL FillPacket(short * pBuffer,FILE *mp3File,HI_BOOL audiostreamingflag);
#endif
void mp3flush(FILE *mp3File);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* HI_FAAC_H_ */
