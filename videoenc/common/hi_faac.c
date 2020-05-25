/*
 * hi_faac.c
 *
 *  Created on: May 14, 2014
 *      Author: root
 */

//============================================================================
// Name        : test.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <hi_type.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "avstream.h"
#include "samplerate.h"
#include "hi_faac.h"
#if 1
#include "videobuf.h"
#endif
#define MP3ENCODE 0
#if MP3ENCODE
#include "layer3.h"
#endif
#define AACENCODE 1
#if AACENCODE
#include <common/include/voAAC.h>
#include <common/include/cmnMemory.h>
#endif

#define DEFAULT_CONVERTER SRC_SINC_FASTEST
#define DEFAULT_TNS     0
//samplerate related
double		src_ratio;
int oldSampleRate = AUDIOINFREQ;
int new_sample_rate = AUDIOENCFREQ;
int converter;
SRC_STATE	*src_state ;
SRC_DATA	src_data ;

float   Ratedataout[FRAMEINLENGTH*2];
short   Ratedatashortout[FRAMEINLENGTH*2];

int cutOff = AUDIOENCFREQ;
int bitRate = 0;
extern HI_BOOL audiostreamingflag;
extern videobuffer m_videobuffer;
extern pthread_mutex_t streammutex;
FILE *infile, *outfile;

//shine MP3 cencode related
PcmBuff   pcmSendBuf;
shine_config_t config;
shine_t 	   s;

unsigned char  *data;


int16_t        *buffer[2];
int16_t        chan1[SHINE_MAX_SAMPLES], chan2[SHINE_MAX_SAMPLES];
int samples_per_pass;
#define MONOMODE 1

struct sockaddr_in audoaddr;
int audoaddrlen, audosock;

#if 1
	shine_t        audiostreamings;
	audiobuffer m_audiobuffer;
	unsigned char  *audiodata;

	unsigned char* tmpmessage;
	videobitstream m_audiobitstream;
#endif
#if AACENCODE
	audioconfig aacaudioconfig;
	VO_AUDIO_CODECAPI codec_api = { 0 };
	VO_HANDLE handle = 0;
	VO_MEM_OPERATOR mem_operator = { 0 };
	VO_CODEC_INIT_USERDATA user_data;
	AACENC_PARAM params = { 0 };
	int inputSize;
	uint8_t* inputBuf;
	int16_t* convertBuf;



#endif

/* Write out the MP3 file */
int write_mp3(long bytes, void *buffer, void *config)
{
	//pthread_mutex_lock(&recordmutex);
	return 	fwrite(buffer, sizeof(unsigned char), bytes, outfile);
	//pthread_mutex_unlock(&recordmutex);
}



short encodeG711ToShort(unsigned char alaw) {
	alaw ^= 0xD5;
	int sign = alaw & 0x80;
	int exponent = (alaw & 0x70) >> 4;
	int data = alaw & 0x0f;
	data <<= 4;
	data += 8;
	if (exponent != 0)
		data += 0x100;
	if (exponent > 1)
		data <<= (exponent - 1);
	return (short) (sign == 0 ? data : -data);

}

/*
 * Convert 16 bit linear sample to 8-bit ulaw sample.
 */
unsigned char l16_to_pcmu(int sample) {
	static int exp_lut[256] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7 };
	int sign, exponent, mantissa;
	unsigned char ulawbyte;

	/* Get the sample into sign-magnitude. */
	sign = (sample >> 8) & 0x80; /* set aside the sign */
	if (sign)
		sample = -sample; /* get magnitude */
	if (sample > CLIP)
		sample = CLIP; /* clip the magnitude */

	/* Convert from 16 bit linear to ulaw. */
	sample = sample + BIAS;
	exponent = exp_lut[(sample >> 7) & 0xFF];
	mantissa = (sample >> (exponent + 3)) & 0x0F;
	ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
	if (ulawbyte == 0) ulawbyte = 0x02; /* optional CCITT trap */
#endif

	return ulawbyte;
} /* l16_to_pcmu */

int g711_encode2(void *in_buf, int in_size, void *out_buf, int *out_size) {
	int i;
	unsigned char *out_c;

	out_c = (unsigned char *) out_buf;
	in_size /= 2; /* convert to units */
	for (i = 0; i < in_size; i++)
		out_c[i] = l16_to_pcmu(((short *) in_buf)[i]);
	*out_size = in_size;
	return 0;
}

void encodeG711Packet(unsigned char* buf, short* outPacket) {
	int i = 0;

	for (i = 0; i < 160; i++) {
		outPacket[i] = encodeG711ToShort(buf[i]);
	}

}

HI_BOOL readOneG711Packet(FILE* fp, unsigned char * buf) {
	int n;
	n = fread(buf, 164, 1, fp);
	if (n > 0)
		return HI_TRUE;

	else
		return HI_FALSE;
}
#if AACENCODE

void ReinitAAC()
{
    voAACEncUninit(handle);
    Init_Voaac();
}


void Init_Voaac() {
	aacaudioconfig.channels =1;
	aacaudioconfig.bitsPerSample = 16;//short
	aacaudioconfig.sampleRate = 16000;
//	aacaudioconfig.sampleRate = 32000;
	//aacaudioconfig.sampleRate = 11025;
	aacaudioconfig.bitrate=16;//16kbs

	inputSize = aacaudioconfig.channels * 2 * 1024;
	//inputSize = channels*1024(short);
	inputBuf = (uint8_t*) malloc(AACFRAMELENGTH);
	convertBuf = (int16_t*) malloc(2*AACFRAMELENGTH);

	voGetAACEncAPI(&codec_api);

	mem_operator.Alloc = cmnMemAlloc;
	mem_operator.Copy = cmnMemCopy;
	mem_operator.Free = cmnMemFree;
	mem_operator.Set = cmnMemSet;
	mem_operator.Check = cmnMemCheck;
	user_data.memflag = VO_IMF_USERMEMOPERATOR;
	user_data.memData = &mem_operator;
	codec_api.Init(&handle, VO_AUDIO_CodingAAC, &user_data);

	params.sampleRate = aacaudioconfig.sampleRate;
	params.bitRate = aacaudioconfig.bitrate;
	params.nChannels = aacaudioconfig.channels;
	params.adtsUsed = 1;
	printf("params sampleRate bitRate nChannels adtsUsed %d %d %d %d\n",params.sampleRate,params.bitRate,params.nChannels,params.adtsUsed);
	if (codec_api.SetParam(handle, VO_PID_AAC_ENCPARAM, &params) != VO_ERR_NONE) {
		fprintf(stderr, "Unable to set encoding parameters\n");
		exit(1);
	}
}
#endif
/*************************************************************/
void Init_AudioSocket() {

	/* set up socket */
	audosock = socket(AF_INET, SOCK_DGRAM, 0);
	if (audosock < 0) {
		perror("socket");
		exit(1);
	}
	/* set up socket */
	audosock = socket(AF_INET, SOCK_DGRAM, 0);
	if (audosock < 0) {
		perror("socket");
		exit(1);
	}
	bzero((char *)&audoaddr, sizeof(audoaddr));
	audoaddr.sin_family = AF_INET;
	audoaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	audoaddr.sin_port = htons(AudioBasePort);
	audoaddr.sin_addr.s_addr = inet_addr(GROUPIP);
	audoaddrlen = sizeof(audoaddr);
	int reuse = 1;
	setsockopt(audosock, SOL_SOCKET, SO_REUSEADDR, (int *) &reuse,
			sizeof(reuse));

}

/* Use these default settings, can be overridden */
static void set_defaults(shine_config_t *config)
{
  shine_set_config_mpeg_defaults(&config->mpeg);
}


/* Print some info about what we're going to encode */
static void check_config(shine_config_t *config)
{
  static char *version_names[4] = { "2.5", "reserved", "II", "I" };
  static char *mode_names[4]    = { "stereo", "joint-stereo", "dual-channel", "mono" };
  static char *demp_names[4]    = { "none", "50/15us", "", "CITT" };

  printf("MPEG-%s layer III, %s  Psychoacoustic Model: Shine\n",
    version_names[shine_check_config(config->wave.samplerate, config->mpeg.bitr)],
    mode_names[config->mpeg.mode]);
  printf("Bitrate: %d kbps  ", config->mpeg.bitr);
  printf("De-emphasis: %s   %s %s\n",
    demp_names[config->mpeg.emph],
    ((config->mpeg.original) ? "Original" : ""),
    ((config->mpeg.copyright) ? "(C)" : ""));
}
void initaudiobuf()
{
//	init_audio_buffer(&m_audiobuffer, "/lib/tmaudiobuf0", 3, 21);
//	init_video_buffer(&m_audiobuffer, "/lib/tmaudiobuf0", 2, 30);
}


//16bit ---->short----->resamplerate----->encode---->out
void initshine()
{
	/* Set the default MPEG encoding paramters - basically init the struct */
	set_defaults(&config);
	config.wave.channels = 1;
	config.wave.samplerate = AUDIOENCFREQ;
	config.mpeg.bitr = 16;
	config.mpeg.emph = 0;
	config.mpeg.mode = MONO;//zly 单声道
#if MONOMODE
	config.mpeg.mode =STEREO;
#endif
	config.mpeg.copyright = 0;
	config.mpeg.original=1;
	buffer[0] = chan1, buffer[1] = chan2;

	 /* Initiate encoder */
	s = shine_initialise(&config);
	audiostreamings=shine_initialise(&config);
#if 1

#endif
	samples_per_pass = shine_samples_per_pass(s);;

}


void SBSInit() {
	int error;
	converter = DEFAULT_CONVERTER;
	src_ratio = (1.0 * new_sample_rate) / oldSampleRate;
	printf("Converter     : %s\n\n", src_get_name(converter));
	pcmSendBuf.iCount = 0;
	if ((src_state = src_new(converter, 1, &error)) == NULL) {
		printf("\n\nError : src_new() failed : %s.\n\n", src_strerror(error));
		return;
	};
}

HI_BOOL FillAACPacket(short * pBuffer, FILE *mp3File,	HI_BOOL audiostreamingflag) {
	VO_CODECBUFFER input = { 0 }, output = { 0 };
	VO_AUDIO_OUTPUTINFO output_info = { 0 };

    uint8_t outbuf[24*1024];
#if AACENCODE
	samples_per_pass =AACFRAMELENGTH/2;//aac 2048 frame point
#if 0
	printf("samples_per_pass is %d \n",samples_per_pass);
#endif

#endif
#define FRAMERESAMPLE 0
#if FRAMERESAMPLE
	src_short_to_float_array(pBuffer, Ratedatain, FRAMEINLENGTH);
	src_data.data_in = Ratedatain;
	src_data.end_of_input = 0;
	src_data.src_ratio = src_ratio;
	src_data.data_out = Ratedataout;
	src_data.input_frames = FRAMEINLENGTH;
	src_data.output_frames = 500;
	if ((error = src_process(src_state, &src_data))) {
		return HI_FALSE;
	}
#endif

	//printf("packet dealed %d\r\n",src_data.output_frames_gen);
	// here to fill acc buffer
	int iTotal = 0;
	int iLeft = 0;
	int iToCopy = 0;
#if 0
	printf("pcmSendBuf.iCount is %d gen is %d \n", pcmSendBuf.iCount,
			src_data.output_frames_gen);
#endif
	int i;
	for(i=0;i<FRAMEINLENGTH/2;i++)
	{
		pBuffer[i]=	pBuffer[2*i]+pBuffer[2*i+1];
	}
#if FRAMERESAMPLE
//
#endif
	src_data.output_frames_gen=FRAMEINLENGTH/2;//No resample
	iTotal = pcmSendBuf.iCount + src_data.output_frames_gen;	//for resample

	if (iTotal > samples_per_pass) {
		iToCopy = samples_per_pass - pcmSendBuf.iCount;
		iLeft = src_data.output_frames_gen - iToCopy;
	} else if (iTotal == samples_per_pass) {
		iToCopy = samples_per_pass - pcmSendBuf.iCount;
		iLeft = 0;
	} else {
		iLeft = 0;
		iToCopy = src_data.output_frames_gen;
	}

	if (iToCopy > 0) {
//		src_float_to_short_array(Ratedataout, Ratedatashortout,
//				src_data.output_frames_gen);
#if 0
		memcpy((void *) (pcmSendBuf.buf + pcmSendBuf.iCount),
				(short *) Ratedatashortout, iToCopy * sizeof(short));//resample
#endif
		//no resample
		memcpy((void *) (pcmSendBuf.buf + pcmSendBuf.iCount),
						 pBuffer, iToCopy * sizeof(short));

		pcmSendBuf.iCount = pcmSendBuf.iCount + iToCopy;
	}
	if (pcmSendBuf.iCount >= samples_per_pass) {
        //for next step wxb 2014-11-25

//			int k;
//			short audiobuffer[2*AACFRAMELENGTH];
//			for( k=0;k<samples_per_pass;k++)
//				audiobuffer[k*2]=pcmSendBuf.buf[k];
			input.Buffer = (unsigned char *)pcmSendBuf.buf;
			input.Length = AACFRAMELENGTH;
			codec_api.SetInputData(handle, &input);

			output.Buffer = outbuf;
			output.Length = sizeof(outbuf);
            #if 1
//			if (codec_api.GetOutputData(handle, &output, &output_info)
//					!= VO_ERR_NONE) {
//				fprintf(stderr, "Unable to encode frame\n");
//				return 1;
//			}


            int status =codec_api.GetOutputData(handle, &output, &output_info);

            if (status != VO_ERR_NONE) {
                fprintf(stderr, "Unable to encode frame\n");
                pcmSendBuf.iCount = 0;
                if (iLeft > 0) {
                    //no resample
                    memcpy((void *) (pcmSendBuf.buf), (void *) (pBuffer + iToCopy),	iLeft * sizeof(short));
                    pcmSendBuf.iCount = pcmSendBuf.iCount + iLeft;
                }
                ReinitAAC();
                return 1;
            }

        #if 1
            pthread_mutex_lock(&streammutex);
            if(mp3File){
                fwrite(outbuf, 1, output.Length, mp3File);
                fflush(mp3File);
            }
            pthread_mutex_unlock(&streammutex);
        #endif

#if 0
			printf("aac out put bytes %d \n", output.Length);
#endif
            if (output.Length > 0 && output.Length <= VIDEOBUFLENGTH) {
				m_audiobitstream.naltype=0x99;
				m_audiobitstream.size = output.Length;
				memcpy(m_audiobitstream.videodata, outbuf, output.Length);
#if 0
				printf("aac out put bytes %d \n", output.Length);
#endif
                if (audiostreamingflag == HI_TRUE)
				videobuffer_write(&m_videobuffer, m_audiobitstream);

            }
        #endif
		pcmSendBuf.iCount = 0;

	}

	if (iLeft > 0) {
#if 0
		memcpy((void *) (pcmSendBuf.buf), (void *) (Ratedatashortout + iToCopy),
				iLeft * sizeof(short));
#endif
		//no resample
		memcpy((void *) (pcmSendBuf.buf), (void *) (pBuffer + iToCopy),
						iLeft * sizeof(short));
		pcmSendBuf.iCount = pcmSendBuf.iCount + iLeft;
	}

//	printf("iLeft --->pcmSendBuf.iCount is %d\n",pcmSendBuf.iCount);
	return HI_TRUE;
}
//HI_BOOL FillPacket(short * pBuffer,FILE *mp3File,HI_BOOL audiostreamingflag) {
//	int error;
//	float   Ratedatain[FRAMEINLENGTH];
//	long           written;
//
//	src_short_to_float_array (pBuffer, Ratedatain, FRAMEINLENGTH);
//	src_data.data_in = Ratedatain;
//	src_data.end_of_input = 0;
//	src_data.src_ratio = src_ratio ;
//	src_data.data_out  = Ratedataout;
//	src_data.input_frames  = FRAMEINLENGTH;
//	src_data.output_frames = 500;
//	if ((error = src_process (src_state, &src_data)))
//	{
//		return HI_FALSE;
//	}
//
//	//printf("packet dealed %d\r\n",src_data.output_frames_gen);
//	// here to fill acc buffer
//	int iTotal =0;
//	int iLeft = 0;
//	int iToCopy = 0;
//	iTotal = pcmSendBuf.iCount + src_data.output_frames_gen ;
//#if 0
//	printf("pcmSendBuf.iCount is %d gen is %d \n",pcmSendBuf.iCount,src_data.output_frames_gen);
//#endif
//	if( iTotal> samples_per_pass)
//	{
//		iToCopy =samples_per_pass - pcmSendBuf.iCount;
//		iLeft  = src_data.output_frames_gen - iToCopy;
//	}
//	else
//	if( iTotal == samples_per_pass)
//	{
//		iToCopy = samples_per_pass - pcmSendBuf.iCount;
//		iLeft = 0;
//	}
//	else
//	{
//		iLeft = 0;
//		iToCopy = src_data.output_frames_gen;
//	}
//
//	if( iToCopy > 0)
//	{
//		src_float_to_short_array(Ratedataout, Ratedatashortout,
//				src_data.output_frames_gen);
//		memcpy((void *) (pcmSendBuf.buf + pcmSendBuf.iCount),
//				(short *) Ratedatashortout, iToCopy * sizeof(short));
//
//		pcmSendBuf.iCount = pcmSendBuf.iCount + iToCopy;
//	}
//	if( pcmSendBuf.iCount >= samples_per_pass)
//	{
//		//start acc coder
//		//encode part
//		int j = 0;
//		memcpy(buffer[0],pcmSendBuf.buf,samples_per_pass*sizeof(short));
//#if MONOMODE
//		memcpy(buffer[1],pcmSendBuf.buf,samples_per_pass*sizeof(short));
//#endif
////		for (j = 0; j < samples_per_pass; j++) {
////			buffer[0][j] = pcmSendBuf.buf[j];
////			buffer[1][j] = 0;
////		}
//
////		memcpy(buffer[1],pcmSendBuf.buf,samples_per_pass*sizeof(short));
//
//		if(audiostreamingflag==HI_TRUE)
//		{
//			audiodata= shine_encode_buffer(audiostreamings, buffer, &written);
//			printf("streaming data mp3 length is %d \n",written);
//
//			if (written > 0) {
//				m_audiobitstream.size=written;
//				memcpy(m_audiobitstream.videodata,audiodata,written);
//#if 1
//				printf("mp3 out put bytes %d\r\n", written);
//#endif
//				videobuffer_write(&m_audiobuffer,m_audiobitstream);
//			}
//		}
//		data = shine_encode_buffer(s, buffer, &written);
//
//		if (written > 0) {
//			fwrite(data, sizeof(unsigned char), written, mp3File);
//			fflush(mp3File);
//#if 0
//			printf("mp3 out put bytes %d\r\n", written);
//			printf("write mp3 \n");
//#endif
//		}
//
//
//		pcmSendBuf.iCount = 0;
//	}
//
//	if( iLeft >0)
//	{
//		memcpy((void *)(pcmSendBuf.buf),(void *)(Ratedatashortout+iToCopy),iLeft* sizeof(short));
//		pcmSendBuf.iCount = pcmSendBuf.iCount +iLeft;
//	}
//
////	printf("iLeft --->pcmSendBuf.iCount is %d\n",pcmSendBuf.iCount);
//	return HI_TRUE;
//}


void CloseRate() {
	src_state = src_delete(src_state);
}

void mp3flush(FILE *mp3File)
{
	long mwritten;
	data = shine_flush(s, &mwritten);
	fwrite(data, sizeof(unsigned char), mwritten, mp3File);
				fflush(mp3File);
}
void CloseShine() {
	  /* Close encoder. */

	  shine_close(s);

}


