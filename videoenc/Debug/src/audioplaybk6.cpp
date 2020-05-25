//============================================================================
// Name        : audioplay.cpp

// Author      : Zhangly
// Version     :
// Copyright   : WuhanJinsiDao Company limited!
// Description : Hello World in C++, Ansi-style
//============================================================================
//debug
/*
 * send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
send audio data to ADEC*****************
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "../common/hi_comm_aio.h"
#include "../common/hi_comm_vb.h"
#include "../common/sample_comm.h"

using namespace std;
extern "C"
{
	#include"libavcodec/avcodec.h"
	#include"libavformat/avformat.h"
}

struct _Block;
typedef struct _Block Block;


#define VIDEO_FRAME_FLAG 1
#define AUDIO_FRAME_FLAG 2
#define SAMPLE_AUDIO_AO_DEV 2
AO_CHN AoChn=0;

#define SAMPLE_AUDIO_PTNUMPERFRM   128

static PAYLOAD_TYPE_E gs_enPayloadType = PT_G711U;
static HI_BOOL gs_bMicIn = HI_FALSE;

static HI_BOOL gs_bAiAnr = HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static HI_BOOL gs_bUserGetMode = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;


#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

pthread_t ntid;
typedef struct tag_ADEC_S
{
    HI_BOOL bStart;
    HI_S32 AdChn;
    FILE *pfd;
    pthread_t stAdPid;
} ADEC_S;

typedef struct tagFFMpeg_AO_S
{
    HI_BOOL bStart;
    AUDIO_DEV AoDev;
    AO_CHN AoChn;
    pthread_t stFFMpegAoPid;
} FFMpeg_AO_S;

static FFMpeg_AO_S ffmpeg_AO_S;
/*
void writeI2C(int fd,int device_addr,int regaddr, int regvalue)
{
	int value;
	int ret;
	value = ((device_addr&0xff)<<24) | ((regaddr&0xff)<<16) | (regvalue&0xffff);
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);
}

void initaudiochip(unsigned char dec)
{
	int fd;
	int reg_valueret;
	fd = open("/dev/gpioi2c", 0);
	writeI2C(fd,0x60, 0xff, 0x01);
	close(fd);


}
*/


ADEC_S pscAdec;
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_AUDIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}


HI_VOID SAMPLE_AUDIO_Usage(void)
{
    printf("\n/************************************/\n");
    printf("press sample command as follows!\n");
    printf("6:  read audio stream from file,using FFMpeg to decode and send AO\n");
    printf("q:  quit whole audio sample\n\n");
    printf("sample command:");
}


/******************************************************************************
* function : PT Number to String
******************************************************************************/
static char* SAMPLE_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
    if (PT_G711A == enType)  return "g711a";
    else if (PT_G711U == enType)  return "g711u";
    else if (PT_ADPCMA == enType)  return "adpcm";
    else if (PT_G726 == enType)  return "g726";
    else if (PT_LPCM == enType)  return "pcm";
    else return "data";
}

/******************************************************************************
* function : Open Adec File
******************************************************************************/
static FILE *SAMPLE_AUDIO_OpenAdecFile(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    FILE *pfd;
    HI_CHAR aszFileName[128];

    /* create file for save stream
    sprintf(aszFileName, "audio_chn%d.%s", AdChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "rb");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for adec ok\n", aszFileName);
    return pfd;*/
}



void *FFMPEG_AUDIO_AdecProc(void *args) {
	HI_S32 s32Ret;
	ADEC_S *pstAdecCtl = (ADEC_S *) args;
	HI_S32 s32AdecChn;
	FILE *pfd = pstAdecCtl->pfd;
	s32AdecChn = pstAdecCtl->AdChn;
	unsigned char samplepoint=0x50;

    unsigned char framecnt =0x00;

     char audio_buffer[0x100*2+4] = {0x00, 0x01, samplepoint, 0x00};//80 sample points

   // memset(audio_buffer,0,sizeof(audio_buffer));
	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	static short audio_len;
	AUDIO_STREAM_S stAudioStream;
	HI_S32 s32ret;

	char filepath[] = "baoxian.wav";
	//Register all available file formats and codecs
	av_register_all();
	//把结构体改为指针
	AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//音频和视频解码更加统一！
	//新加
	AVFrame *pFrame;
	pFrame = avcodec_alloc_frame();

	AVFormatContext *formatContext;
	if (avformat_open_input(&formatContext, filepath, NULL, NULL) != 0) {
		printf("无法打开文件\n");
		//return -1;
		pthread_exit(NULL);
	}
	if (av_find_stream_info(formatContext) < 0) {
		printf("Couldn't find stream information.\n");
		//return -1;
		pthread_exit(NULL);
	}
	for (i = 0; i < formatContext->nb_streams; i++)
		//原为codec_type==CODEC_TYPE_AUDIO
		if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}
	printf("Found  a audio stream already audioStream is %d.\n",audioStream);
	if (audioStream == -1) {
		printf("Didn't find a audio stream.\n");
		//return -1;
		pthread_exit(NULL);
	}
	// Get a pointer to the codec context for the audio stream
	pCodecCtx = formatContext->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	   /* open it */
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	        fprintf(stderr, "Could not open codec\n");
	        exit(1);
	}

	printf("codec_id is %d\n",pCodecCtx->codec_id);
	av_dump_format(formatContext,-1,filepath,0);

	if (pCodec == NULL) {
		printf("Codec not found.\n");
		//return -1;
		pthread_exit(NULL);
	}
	uint32_t ret, len = 0;
	av_read_frame(formatContext, packet);

	int got_picture;
	int index = 0;
	//
	printf("Start Read frame \n");
	while (av_read_frame(formatContext, packet) >= 0) {
		printf(" frame length is %d",packet->size);
		if (packet->stream_index == audioStream) {
			//* don't decode
			ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture,
					packet);

			if (ret < 0) // if error len = -1
			{
				printf("Error in decoding audio frame.\n");
				pthread_exit(NULL);
			}/**/

			if (got_picture > 0) {
#if 1
				printf("index %3d\n", index);
				printf("pts %5d\n", packet->pts);
				printf("dts %5d\n", packet->dts);
				printf("packet_size %5d\n", packet->size);
				framecnt=10;
				//printf("begin....\n");
				//设置音频数据缓冲,PCM数据
				unsigned short buffsize=samplepoint*sizeof(unsigned short);
				unsigned char *faudio=(unsigned char*) pFrame->data[0];

				/**/for(unsigned short fcnt=0;fcnt<framecnt;fcnt++)
				{
					memcpy(audio_buffer+4,faudio+fcnt*buffsize,buffsize);
					stAudioStream.pStream = (HI_U8 *)audio_buffer;
			    	stAudioStream.u32Len = buffsize+4;//for debug zly
			    	//***************s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream,HI_TRUE);/**/
					printf("send audio data to ADEC*****************\n");
					s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream,
										HI_TRUE);

					if (s32Ret) {
						printf("%s: HI_MPI_ADEC_SendStream(%d) failed with %#x!\n",
								__FUNCTION__, s32AdecChn, s32Ret);
						break;
					}
				}


				//设置音频数据长度
				audio_len = pFrame->linesize[0];
				printf("audio_len is %d\n", audio_len);
				av_free_packet(packet);
#endif
			}

		}
	}
	printf("send audio data to ADEC thread exit*****************\n");
}

/****************************************************/
void *FFMPEG_AUDIO_AO_Proc(void *args) {

	HI_S32 s32Ret;
	/*ADEC_S *pstAdecCtl = (ADEC_S *) args;*/
	HI_S32 s32AdecChn;
	//FILE *pfd = pstAdecCtl->pfd;
	//s32AdecChn = pstAdecCtl->AdChn;
	AUDIO_FRAME_S   audioframe;
	audioframe.enBitwidth=AUDIO_BIT_WIDTH_16;
	audioframe.enSoundmode=AUDIO_SOUND_MODE_MONO;


	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	static short audio_len;
	AUDIO_STREAM_S stAudioStream;
	HI_S32 s32ret;

	char filepath[] = "lathe.wav";
	//Register all available file formats and codecs
	av_register_all();
	//把结构体改为指针
	AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//音频和视频解码更加统一！
	//新加
	AVFrame *pFrame;
	pFrame = avcodec_alloc_frame();

	AVFormatContext *formatContext;
	if (avformat_open_input(&formatContext, filepath, NULL, NULL) != 0) {
		printf("无法打开文件\n");
		//return -1;
		pthread_exit(NULL);
	}
	if (av_find_stream_info(formatContext) < 0) {
		printf("Couldn't find stream information.\n");
		//return -1;
		pthread_exit(NULL);
	}
	for (i = 0; i < formatContext->nb_streams; i++)
		//原为codec_type==CODEC_TYPE_AUDIO
		if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}
	printf("Found  a audio stream already audioStream is %d.\n",audioStream);
	if (audioStream == -1) {
		printf("Didn't find a audio stream.\n");
		//return -1;
		pthread_exit(NULL);
	}
	// Get a pointer to the codec context for the audio stream
	pCodecCtx = formatContext->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	   /* open it */
	 if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	        fprintf(stderr, "Could not open codec\n");
	        exit(1);
	    }


	printf("codec_id is %d\n",pCodecCtx->codec_id);
	av_dump_format(formatContext,-1,filepath,0);

	if (pCodec == NULL) {
		printf("Codec not found.\n");
		//return -1;
		pthread_exit(NULL);
	}
	uint32_t ret, len = 0;
	av_read_frame(formatContext, packet);

	int got_picture;
	int index = 0;
	//
	printf("Start Read frame \n");
	while (av_read_frame(formatContext, packet) >= 0) {
		printf(" frame length is %d",packet->size);
		if (packet->stream_index == audioStream) {
			//* don't decode
			ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture,
					packet);

			/*if (ret < 0) // if error len = -1
			{
				printf("Error in decoding audio frame.\n");
				pthread_exit(NULL);
			}*/

			if (got_picture > 0) {
#if 1
				printf("index %3d\n", index);
				printf("pts %5d\n", packet->pts);
				printf("dts %5d\n", packet->dts);
				printf("packet_size %5d\n", packet->size);
				//printf("begin....\n");
				//设置音频数据缓冲,PCM数据
				stAudioStream.pStream = (unsigned char*) pFrame->data[0];
				stAudioStream.u32Len = pFrame->linesize[0];//4096
				stAudioStream.u32Len = 1024;//for debug zly
				stAudioStream.u64TimeStamp =pFrame->pts;
				// 	stAudioStream.u64TimeStamp = block->pts;
				audioframe.pVirAddr[0]=(unsigned char*) pFrame->data[0];
				audioframe.u32Seq=pFrame->pts;
				audioframe.u32Len=pFrame->linesize[0];//4096

		        /* send frame to ao
		            if (HI_TRUE )
		            {
		                HI_MPI_AO_SendFrame(pstAiCtl->AoDev, pstAiCtl->AoChn,
		                    &audioframe, HI_TRUE);
		            }*/

				/*s32Ret = HI_MPI_ADEC_SendStream(s32AdecChn, &stAudioStream,
						HI_TRUE);*/
				//***************
				//s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream,
				//					HI_TRUE);
				if (s32Ret) {
					printf("%s: HI_MPI_ADEC_SendStream(%d) failed with %#x!\n",
							__FUNCTION__, s32AdecChn, s32Ret);
					break;
				}

				//设置音频数据长度
				audio_len = pFrame->linesize[0];
				printf("audio_len is %d\n", audio_len);
#endif
			}

		}
	}

}
/******************************************************************************
* function : Create the thread to get stream from file and send to adec
* * Parse file by FFmpge!
******************************************************************************/
HI_S32 SAMPLE_COMM_AUDIO_CreatFFmpegFileAdec(ADEC_CHN AdChn, FILE *pAdcFd)
{

	ADEC_S *pstAdec = &pscAdec;

   /* if (NULL == pAdcFd)
    {
        return HI_FAILURE;
    }*/
    pstAdec->AdChn=AdChn;
    pstAdec->pfd=pAdcFd;
    pthread_create(&ntid, 0, FFMPEG_AUDIO_AdecProc, pstAdec);


    return HI_SUCCESS;
}

/******************************************************************************
* function : Destory the thread to get stream from file and send to adec
******************************************************************************/
HI_S32 DestoryFFmpegFileAdec(ADEC_CHN AdChn)
{
    pthread_join(ntid, 0);

    return HI_SUCCESS;
}


/******************************************************************************
* function : file ->FFMpeg---->ADec -> Ao
******************************************************************************/
HI_S32 SAMPLE_FFMpeg_AdecAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32      s32Ret;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    FILE        *pfd = NULL;

    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }
    //enSamplerate);
    //enWorkmode);
    //enBitwidth);
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);//setup nxp1918 samplerate workmode,bitwidth
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
     //gs_enPayloadType = PT_ADPCMA;
    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
    printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
    printf("AdChn is %d*********\n",AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

  /* pfd = SAMPLE_AUDIO_OpenAdecFile(AdChn, gs_enPayloadType);
    if (!pfd)
    {
        SAMPLE_DBG(HI_FAILURE);
        return HI_FAILURE;
    } */
    s32Ret = SAMPLE_COMM_AUDIO_CreatFFmpegFileAdec(AdChn, pfd);
   // s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AdChn, pfd);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    DestoryFFmpegFileAdec(AdChn);//
    SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
    SAMPLE_COMM_AUDIO_StopAdec(AdChn);
    SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);

    return HI_SUCCESS;
}

#define AUDIO_POINT_NUM 160




void *FFMpeg_thread(void *args)
{
	HI_S32 s32Ret;
	FFMpeg_AO_S *pstAdecCtl = (FFMpeg_AO_S *) args;
    AUDIO_DEV AoDev=pstAdecCtl->AoDev;
	AUDIO_FRAME_S   audioframe;
	audioframe.enBitwidth=AUDIO_BIT_WIDTH_16;
	audioframe.enSoundmode=AUDIO_SOUND_MODE_MONO;

	//initaudio(0x60,16,0,0);//working chip second stage
	//initaudio(0x64,16,0,0);
	//initaudio(0x62,16,0,0);//working chip audio connect to output speaker first stage

	//HI_U32 framecnt =0x00;


	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	static short audio_len;
	AUDIO_STREAM_S stAudioStream;
	HI_S32 s32ret;

	char filepath[] = "lathe.wav";
	//Register all available file formats and codecs
	av_register_all();
	//把结构体改为指针
	AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//音频和视频解码更加统一！
	//新加
	AVFrame *pFrame;
	pFrame = avcodec_alloc_frame();

	AVFormatContext *formatContext;
	if (avformat_open_input(&formatContext, filepath, NULL, NULL) != 0) {
		printf("无法打开文件\n");
		//return -1;
		pthread_exit(NULL);
	}
	if (av_find_stream_info(formatContext) < 0) {
		printf("Couldn't find stream information.\n");
		//return -1;
		pthread_exit(NULL);
	}
	for (i = 0; i < formatContext->nb_streams; i++)
		//原为codec_type==CODEC_TYPE_AUDIO
		if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}
	printf("Found  a audio stream already audioStream is %d.\n",audioStream);
	if (audioStream == -1) {
		printf("Didn't find a audio stream.\n");
		//return -1;
		pthread_exit(NULL);
	}
	// Get a pointer to the codec context for the audio stream
	pCodecCtx = formatContext->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	   /* open it */
	 if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	        fprintf(stderr, "Could not open codec\n");
	        exit(1);
	    }


	printf("codec_id is %d\n",pCodecCtx->codec_id);
	av_dump_format(formatContext,-1,filepath,0);

	if (pCodec == NULL) {
		printf("Codec not found.\n");
		//return -1;
		pthread_exit(NULL);
	}
	uint32_t ret, len = 0;
	av_read_frame(formatContext, packet);

	int got_picture;
	int index = 0;
	//
	printf("Start Read frame \n");

	while (av_read_frame(formatContext, packet) >= 0) {
		printf(" frame length is %d",packet->size);
		if (packet->stream_index == audioStream) {
			//* don't decode
			ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture,
					packet);

			/*if (ret < 0) // if error len = -1
			{
				printf("Error in decoding audio frame.\n");
				pthread_exit(NULL);
			}*/

			if (got_picture > 0) {
#if 1
				printf("index %3d\n", index);
				printf("pts %5d\n", packet->pts);
				printf("dts %5d\n", packet->dts);
				printf("packet_size %5d\n", packet->size);
				//printf("begin....\n");
				//设置音频数据缓冲,PCM数据
				stAudioStream.pStream = (unsigned char*) pFrame->data[0];
				stAudioStream.u32Len = pFrame->linesize[0];//4096
				stAudioStream.u64TimeStamp =pFrame->pts;
				// 	stAudioStream.u64TimeStamp = block->pts;
				audioframe.pVirAddr[0]=(unsigned char*) pFrame->data[0];
				audioframe.u32Seq=pFrame->pts;
				audioframe.u32Len=pFrame->linesize[0];//4096
				audioframe.u32Len=160;//4096
			//	audioframe.u32Seq=framecnt;
       //  send frame to ao
		    if (HI_TRUE )
		    {
		                //HI_MPI_AO_SendFrame(AoDev, AoChn,&audioframe, HI_TRUE);
		    	s32Ret= HI_MPI_AO_SendFrame(2, AoChn,&audioframe, HI_TRUE);
		    }/**/

				/*s32Ret = HI_MPI_ADEC_SendStream(s32AdecChn, &stAudioStream,
						HI_TRUE);*/
				//***************
				//s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream,
				//					HI_TRUE);
				if (s32Ret) {
					printf("%s: HI_MPI_AO_SendFrame(%d) failed with %#x!\n",
							__FUNCTION__, AoDev, s32Ret);
					break;
				}
				//framecnt++;

				//设置音频数据长度
				audio_len = pFrame->linesize[0];
				printf("audio_len is %d\n", audio_len);
				av_free_packet(packet);
#endif
			}

		}
	}

}


HI_S32 AO_Init()
{
    HI_S32 s32ret;
    AIO_ATTR_S stAoAttr;
    AIO_ATTR_S  stAttr;
    AUDIO_DEV   AoDevId=2;

    /* init stAio. all of cases will use it */
    stAoAttr.enBitwidth = AUDIO_BIT_WIDTH_16;/* should equal to DA */
    stAoAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
    stAoAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAoAttr.enWorkmode = AIO_MODE_I2S_SLAVE;        //AIO_MODE_I2S_MASTER;//
    stAoAttr.u32EXFlag = 1;
    stAoAttr.u32FrmNum = 30;
    stAoAttr.u32PtNumPerFrm =  AUDIO_POINT_NUM;//AACLC_SAMPLES_PER_FRAME;//AACPLUS_SAMPLES_PER_FRAME;
    stAoAttr.u32ChnCnt = 2;
    stAoAttr.u32ClkSel = 1;

    /* set ao public attr*/
    printf("");
    s32ret = HI_MPI_AO_SetPubAttr(AoDevId, &stAoAttr);
    if(HI_SUCCESS != s32ret)
    {
        printf("set ao %d attr err:0x%x\n", AoDevId,s32ret);
        return s32ret;
    }
    s32ret = HI_MPI_AO_GetPubAttr(AoDevId, &stAttr);
    if(HI_SUCCESS != s32ret)
    {
      printf("get ao %d attr err:0x%x\n", AoDevId,s32ret);
     return s32ret;
    }
    HI_S32 s32AoFd;

    /* enable ao device*/
    s32ret = HI_MPI_AO_Enable(AoDevId);
    if(HI_SUCCESS != s32ret)
    {
        printf("enable ao dev %d err:0x%x\n", AoDevId, s32ret);
        return s32ret;
    }
    /* enable ao chnnel*/
    printf("Please select AoChn *****\n");

    s32ret = HI_MPI_AO_EnableChn(AoDevId, AoChn);
    if(HI_SUCCESS != s32ret)
    {
        printf("enable ao chn %d err:0x%x\n", AoChn, s32ret);
        return s32ret;
    }

    s32AoFd = HI_MPI_AO_GetFd(AoDevId, AoChn);
    if(s32AoFd <= 0)
    {
     return HI_FAILURE;
    }
    ffmpeg_AO_S.AoDev=AoDevId;
    ffmpeg_AO_S.AoChn=AoChn;

    pthread_create(&(ffmpeg_AO_S.stFFMpegAoPid), 0, FFMpeg_thread, &ffmpeg_AO_S);
    /* send audio frme to ao
            s32ret = HI_MPI_AO_SendFrame(AoDevId, AoChn, &decodedAudioInfo, HI_TRUE);
            if (HI_SUCCESS != s32ret)
            {
                printf("ao send frame err:0x%x\n",s32ret);
                break;
            }*/

    return HI_SUCCESS;
}

/******************************************************************************
* function : file ->FFMpeg--> Ao
******************************************************************************/
HI_S32 FFMpeg_AdecAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32      s32Ret;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;

    FILE        *pfd = NULL;

    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
//no resample
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    ffmpeg_AO_S.AoDev=AoDev;
    ffmpeg_AO_S.AoChn=AoChn;

	pthread_create(&(ffmpeg_AO_S.stFFMpegAoPid), 0, FFMpeg_thread, &ffmpeg_AO_S);


    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();


    SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);


    return HI_SUCCESS;
}


/******************************************************************************
* function : main
******************************************************************************/
HI_S32 main(int argc, char *argv[])
{
    char ch;
    HI_S32 s32Ret= HI_SUCCESS;
    VB_CONF_S stVbConf;
    AIO_ATTR_S stAioAttr;
    AUDIO_RESAMPLE_ATTR_S stAiReSampleAttr;
    AUDIO_RESAMPLE_ATTR_S stAoReSampleAttr;
    //gs_enPayloadType = PT_G711U;
    /* arg 1 is audio payload type */
    if (argc >= 2)
    {
    	AoChn= atoi(argv[1]);
       // gs_enPayloadType = atoi(argv[1]);
#if 0
        if (gs_enPayloadType != PT_G711A && gs_enPayloadType != PT_G711U &&\
            gs_enPayloadType != PT_ADPCMA && gs_enPayloadType != PT_G726 &&\
            gs_enPayloadType != PT_LPCM)
        {
            printf("payload type invalid!\n");
            printf("\nargv[1]:%d is payload type ID, suport such type:\n", gs_enPayloadType);
            printf("%d:g711a, %d:g711u, %d:adpcm, %d:g726, %d:lpcm\n",
            PT_G711A, PT_G711U, PT_ADPCMA, PT_G726, PT_LPCM);
            return HI_FAILURE;
        }
#endif

    }

    /* init stAio. all of cases will use it */
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 1;
    stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 1;

    /* config ao resample attr if needed */
    if (HI_TRUE == gs_bAioReSample)
    {
        stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_32000;
        stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM * 4;

        /* ai 32k -> 8k */
        stAiReSampleAttr.u32InPointNum = SAMPLE_AUDIO_PTNUMPERFRM * 4;
        stAiReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_32000;
        stAiReSampleAttr.enReSampleType = AUDIO_RESAMPLE_4X1;
        gs_pstAiReSmpAttr = &stAiReSampleAttr;

        /* ao 8k -> 32k */
        stAoReSampleAttr.u32InPointNum = SAMPLE_AUDIO_PTNUMPERFRM;
        stAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_8000;
        stAoReSampleAttr.enReSampleType = AUDIO_RESAMPLE_1X4;
        gs_pstAoReSmpAttr = &stAoReSampleAttr;
    }
    else
    {
        gs_pstAiReSmpAttr = NULL;
        gs_pstAoReSmpAttr = NULL;
    }

    /* resample and anr should be user get mode */
    gs_bUserGetMode = (HI_TRUE == gs_bAioReSample || HI_TRUE == gs_bAiAnr) ? HI_TRUE : HI_FALSE;

    signal(SIGINT, SAMPLE_AUDIO_HandleSig);
    signal(SIGTERM, SAMPLE_AUDIO_HandleSig);

    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    SAMPLE_AUDIO_Usage();


    while ((ch = getchar()) != 'q')
    {
        switch (ch)
        {
            case '5':
            {
                s32Ret = SAMPLE_FFMpeg_AdecAo(&stAioAttr);/* read audio stream from file,decode and send AO*/
                break;
            }
            case '6':
            {
            	s32Ret = FFMpeg_AdecAo(&stAioAttr);/* read audio stream from file,decode and send AO*/
                         break;
            }
            case '7':
            {
            	AO_Init();
            	break;
            }
            default:
            {
                SAMPLE_AUDIO_Usage();
                break;
            }
        }
        if (s32Ret != HI_SUCCESS)
        {
            break;
        }
    }

    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

