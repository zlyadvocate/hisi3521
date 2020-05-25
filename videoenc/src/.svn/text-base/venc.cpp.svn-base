/******************************************************************************
 A simple program of Hisilicon HI3531 video encode implementation.
 Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
 Modification:  2011-2 Created
 ******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>


#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <getopt.h>
#include "fbsetting.h"
#include "../common/avstream.h"
#include "../common/videobuf.h"


#include "../common/zini.h"

#include <signal.h>



//zly
#include <sys/ioctl.h>
#include "../common/gpio_i2c.h"
//OSD control and thread
#include "osdtime.h"

#include "../common/sample_comm.h"
HI_U8 recordfilepath[]="/mnt/hd1/";
int recordfrmcnt=3500; //for 30min //3500 for 2m12s;
//snap related definition 2014-09-28
int g_snapshot_run = 0;
pthread_mutex_t snapmutex;
//0--one channel snap,1 for all channel snap,2 for one channel continuously snap;3 for all channel continuously snap
//enum box{pencil,pen};
enum snap_type{
	selected_oneshot,
	all_oneshot,
	selected_continuouslyshot,
    all_continuouslyshot,
};
//0--one channel snap,1 for all channel snap,2 for one channel continuously snap;3 for all channel continuously snap
struct snap_pic_contol {
	char snaptype;
    char videochannel;//selected channel
    int piccount;//snap piccount for
    int snapinterval;
};


//command line
char *para = ":c:w:m:r:hf:vtp:n:";
int workmode = 0;

int bitratestream = 60;//streaming channnel bitrate, QCIF-50, CIF-220
//int frameratestream = 8;
int do_help = 0;
int livestreamid=4;
int do_version = 0;
int framerate=10;//string channel framerate
int vomode=0;//VO out mode 0-0,1-1,2-2,3-3,4-4
int volumn=16;
char *file = NULL;
//PIC_SIZE_E picsize = PIC_QCIF;
PIC_SIZE_E picsize = PIC_QVGA;
char mdn[32] = {0};
int snapQuality = 20;

int minisStd = 0;

extern SAMPLE_VENC_GETSTREAM_PARA_S substream_stPara[8];
extern HI_BOOL ChannelStreammark[];
extern unsigned char g_ttl;

struct option longopt[] =
{
	 {"c", required_argument, &livestreamid, 'c'},
     {"r", required_argument, &bitratestream, 'r'},
     {"f", required_argument, &framerate, 'f'},
     {"n", required_argument, &recordfrmcnt, 'n'},
     {"help", no_argument, &do_help, 'h'},
     {"version", no_argument, &do_version, 'v'},
     {"VOmode", required_argument,&vomode, 'm'},
     {"Workmode", required_argument,&workmode, 'w'},
     {"ttl", no_argument, NULL, 't'},
     {"picsize", required_argument, NULL, 'p'},
     {0, 0, 0, 0},
};

//streaming control
//normally we use CBR for streaming
//VRB for storage
videobuffer m_videobuffer;
pthread_mutex_t streammutex;


VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
HI_U32 gs_u32ViFrmRate = 0;
#define SAMPLE_YUV_D1_FILEPATH         "SAMPLE_420_D1.yuv"
char version[] = "venvo version 1.1.0.0";
//mount -t nfs 192.168.1.177:/home/share /lib -o nolock
//insmod extdrv/tw2865.ko
#define SAMPLE_AUDIO_AO_DEV 2
#define SAMPLE_AUDIO_AI_DEV 2

AO_CHN AoChn = 0;

HI_BOOL audiostreamingflag=HI_FALSE;

static PAYLOAD_TYPE_E gs_enPayloadType = PT_G711U;

static HI_BOOL gs_bAiAnr = HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;
#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

pthread_t ntid;
extern char szStation[42];
extern unsigned short nSpeed;

#define KEY  121
#define controlkey 150
#define PLAYSOUND_SHM_ID 200


struct videotype {
    long mtype;
    char videobit[8000];
};

struct audiotype {
    long mtype;
    char audiobit[164];
};



struct msgtype {
  long mtype;
  char mtext[164];
};
struct controltype {
    char videochannel;
    char controlcommand;
};
//enum box{pencil,pen};
enum streamCMDtype{
	Start,
	Stop,
	KeyframeRequest,
	GOPSet,
	BitrateSet,
	PicSizeChange,

};

typedef struct STREAMINGCONTROL
{
    HI_S32  bitrate;
    HI_S32  framerate;
	HI_U8   streamingid;
	HI_U8   videostate;
	HI_U8   controlcommand;
	HI_U8   reserved;
}STREAMINGCONTROL_S;



//command line module
void printhelp()
{
	printf("JSD Company limited!!!!!!!\n");
	printf("avtest %s********\n",version);
	printf("-c set streaming channel ID \n");
	printf("-r set streaming channel bitrate \n");
	printf("-n set record frame count \n");
	printf("-f Set steaming channel framerate \n");
	printf("-m Set VO out mode\n");
	printf("-w select app working mode!!!!!!\n");
	printf("-h that's all.Thanks for view!!!!!!\n");
}


void initaudio(unsigned char dec, unsigned char ch_num,
		unsigned char samplerate, unsigned char bits, unsigned char audiogain);

//zly
void writeI2C(int fd, int device_addr, int regaddr, int regvalue) {
	int value;
	int ret;
	value = ((device_addr & 0xff) << 24) | ((regaddr & 0xff) << 16)
			| (regvalue & 0xffff);
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);
}

int readI2C(int fd, int device_addr, int reg_addr) {
	int value;
	int ret;

	value = ((device_addr & 0xff) << 24) | ((reg_addr & 0xff) << 16);
	ret = ioctl(fd, GPIO_I2C_READ, &value);
	value = value & 0xff;
	return value;

}

#define LOCKFILE "/lib/avtest.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/* set advisory lock on file */
int lockfile(int fd)
{
        struct flock fl;

        fl.l_type = F_WRLCK;  /* write lock */
        fl.l_start = 0;
        fl.l_whence = SEEK_SET;
        fl.l_len = 0;  //lock the whole file

        return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
        int fd;
        char buf[16];

        fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
        if (fd < 0) {
                printf( "can't open %s: %m\n", filename);
                exit(1);
        }

        /* 先获取文件锁 */
        if (lockfile(fd) == -1) {
                if (errno == EACCES || errno == EAGAIN) {
                        printf( "file: %s already locked", filename);
                        close(fd);
                        return 1;
                }
                printf("can't lock %s: %m\n", filename);
                exit(1);
        }
        /* 写入运行实例的pid */
        ftruncate(fd, 0);
        sprintf(buf, "%ld", (long)getpid());
        write(fd, buf, strlen(buf) + 1);
        return 0;
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

    /* create file for save stream*/
    sprintf(aszFileName, "audio_chn%d.%s", AdChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "rb");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for adec ok\n", aszFileName);
    return pfd;
}
void sig_usr(int a )
{

  printf("interupt by user\n");

}

void *AUDIOPLAY(void *args) {

	int msgid;
	HI_S32 s32Ret;
	int count = 0;
	struct msgtype msg;
	AUDIO_STREAM_S stAudioStream;

	signal(SIGUSR1, sig_usr);

//	if ((msgid = msgget(KEY, IPC_CREAT | 0755)) < 0) {

//		perror("msgget");

//		exit(1);

//	}
//    printf("msgid is %d\n", msgid);

    //create a share memory for audioplay to break the playing sound
//    int shmid,ret;
//    void* mem;
//    long *playindex = 0;
//    shmid=shmget( PLAYSOUND_SHM_ID, 1024, 0666 | IPC_CREAT );
//    if(shmid < 0){
//        perror("create shmget failed!");
//        exit(1);
//    }
//    printf("shmid=%d\n", shmid);
//    mem=shmat( shmid,( const void* )0,0 );
//    if( ( int )mem != -1 )
//         {
//        playindex = (long *)mem;
//        *playindex = 0;
//        printf( "Shared memory was attached in our address space at %p/n",mem );
//        printf( "%llu/n", *playindex);
//    }

//    long lastmaxindex = 0;

    audiobitstream m_audiobitstream;
    audiobuffer m_audiobuffer;
    unsigned char* tmpmessage;
    init_audio_buffer(&m_audiobuffer, "/lib/tmaudiobufplay0", 10, 30);

    sleep(10);
    while (1) {
//        printf("\nbegin play audio.\n");
//		memset(&msg, 0, sizeof(struct msgtype));
//		count++;
//        if (msgrcv(msgid, &msg, sizeof(struct msgtype), 0, 0) < 0) {
//            perror("msgrcv");
//            msgctl(msgid, IPC_RMID,0);
//            msgid = msgget(KEY, IPC_CREAT | 0755);
//            continue;
//        }


//        printf("msgtype=%d, lastmaxindex=%d\n", msg.mtype, lastmaxindex);
//        if(msg.mtype > lastmaxindex){
//            lastmaxindex = msg.mtype;
//        }
//        if(msg.mtype < lastmaxindex){
//            continue;
//        }

//        stAudioStream.u32Len = sizeof(msg.mtext);
//        stAudioStream.pStream=(HI_U8*)msg.mtext;


        audiobuffer_read(&m_audiobuffer, &m_audiobitstream);
        if(m_audiobitstream.reserved == 1){

            stAudioStream.u32Len = m_audiobitstream.size;
            stAudioStream.pStream=(HI_U8*)m_audiobitstream.videodata;


            s32Ret = HI_MPI_ADEC_SendStream(0, &stAudioStream, HI_TRUE);
            if (s32Ret) {
                printf("%s: AUDIOPLAY(%d) failed with %#x!\n", __FUNCTION__, 0,
                       s32Ret);
                break;
            }

        }

        if ((count % 50) == 1) {
            printf("-message cnt is %d\n", count);
		}

	}
}


HI_S32 stopaiaenc()
{
    HI_S32 i, s32Ret;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AencChnCnt;
    AENC_CHN    AeChn;

	 for (i=0; i<s32AencChnCnt; i++)
	    {
	        AeChn = i;
	        AiChn = i;
	        {
	            SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, AiChn, AeChn);
	        }
	    }

	    SAMPLE_COMM_AUDIO_StopAenc(s32AencChnCnt);
	    SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);

	    return HI_SUCCESS;
}

//zly 2014-05-15
HI_S32 StopAI()
{
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn;
    HI_S32      s32AiChnCnt=1;
    HI_S32      s32AencChnCnt=2;
    AENC_CHN    AeChn;
    HI_S32 i;

	    for (i=0; i<s32AencChnCnt; i++)
	    {
	        AeChn = i;
	        AiChn = i;

	        if (HI_TRUE )
	        {
	            SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
	        }
	    }

		SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);

	    return HI_SUCCESS;
}

/******************************************************************************
* function : Ai -> Aenc -> file
*                                -> Adec -> Ao
******************************************************************************/
HI_S32 HIJSD_AUDIO_AiAenc(AIO_ATTR_S *pstAioAttr)
{
    HI_S32 i, s32Ret;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AencChnCnt;
    AENC_CHN    AeChn;
    HI_BOOL     bSendAdec = HI_TRUE;
    FILE        *pfd = NULL;

    /* config ai aenc dev attr */
    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    /********************************************
      step 2: start Ai
    ********************************************/
    s32AiChnCnt = pstAioAttr->u32ChnCnt;
    s32AencChnCnt = 1;//s32AiChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
      step 3: start Aenc
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_StartAenc(s32AencChnCnt, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

//set up ao adec,ao channel
HI_S32 initaudiosystem() {
	HI_S32 s32Ret;
	AUDIO_DEV AoDev = SAMPLE_AUDIO_AO_DEV;
	ADEC_CHN AdChn = 0;
	AIO_ATTR_S stAioAttr;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn=0;
    HI_S32      s32AiChnCnt;
    AO_CHN      AoChn = 0;
	FILE *pfd = NULL;
	stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000; //test 32k
	stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
	stAioAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
	stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
	stAioAttr.u32EXFlag = 1;
	stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = 480;
	stAioAttr.u32ChnCnt = 2;
	stAioAttr.u32ClkSel = 1;
	pthread_mutex_init(&streammutex, NULL);
	//init codec
	initaudio(0x60,16,1,0,0x10);//working chip second stage
		//initaudio(0x64,16,0,0);
	initaudio(0x62,16,1,0,0x10);//working chip audio connect to output speaker first stage
    /********************************************
      step 2: start Ai  0515
    ********************************************/
    s32AiChnCnt = 0;
    //s32Ret = HI_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr);
//    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr,HI_FALSE,NULL);
//    if (s32Ret != HI_SUCCESS)
//    {
//        SAMPLE_DBG(s32Ret);
//        return HI_FAILURE;
//    }
     s32Ret = HIJSD_AUDIO_AiAenc(&stAioAttr);
     if (s32Ret != HI_SUCCESS)
     {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, &stAioAttr, gs_pstAoReSmpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    pfd = SAMPLE_AUDIO_OpenAdecFile(AdChn, gs_enPayloadType);
    if (!pfd)
    {
        SAMPLE_DBG(HI_FAILURE);
        return HI_FAILURE;
    }
//    /*start ai capture thread*/
//    printf("start audio capture ###############\n");
//   // s32Ret = HI_AUDIO_CreatTrdAiAenc(AiDev, AiChn);
    printf("start dec thread ******************\n");
    pthread_create(&ntid, 0, AUDIOPLAY, NULL);



    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 stopaudiosystem() {
	HI_S32 s32Ret;
	pthread_join(ntid,0);//wait for audio play thread quit
	AUDIO_DEV AoDev = SAMPLE_AUDIO_AO_DEV;
	ADEC_CHN AdChn = 0;
	SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
	SAMPLE_COMM_AUDIO_StopAdec(AdChn);
	SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
}

void initaudio(unsigned char dec, unsigned char ch_num, unsigned char samplerate, unsigned char bits,unsigned char audiogain)
{
	int fd;
	int reg_valueret;
	fd = open("/dev/gpioi2c", 0);
	//setup video capture output timing of 0x60
	//0-8channel
	if(dec == 0x60)
	{
		writeI2C(fd, dec, 0xff, 0x00);

		writeI2C(fd, dec, 0x58, 0x50);
		writeI2C(fd, dec, 0x59, 0x50);
		writeI2C(fd, dec, 0x5A, 0x50);
		writeI2C(fd, dec, 0x5b, 0x50);

		writeI2C(fd, dec, 0xD8, 0x50);
		writeI2C(fd, dec, 0xD9, 0x50);
		writeI2C(fd, dec, 0xDA, 0x50);
		writeI2C(fd, dec, 0xDb, 0x50);

		writeI2C(fd, dec, 0x68, 0x00);
		writeI2C(fd, dec, 0x69, 0x00);
		writeI2C(fd, dec, 0x6a, 0x00);
		writeI2C(fd, dec, 0x6b, 0x00);
	}


	writeI2C(fd, dec, 0xff, 0x01);

    //gpio_i2c_write(dec, 0xff, 0x01);
    if( (dec == 0x60) || (dec == 0x64))
    {
        writeI2C(fd,dec, 0x00, 0x02);//normal operation 16K
        writeI2C(fd,dec, 0x00, 0x03);//32K  audio mode
        writeI2C(fd,dec, 0x06, 0x1a);

         reg_valueret=readI2C(fd,dec,  0x06);
        printf("I2C write 0x1a,I2C read back is %d line:%d\n",  reg_valueret,__LINE__);

        if(8 == ch_num)
        {
            writeI2C(fd,dec, 0x06, 0x1b);
            writeI2C(fd,dec, 0x08, 0x02);
            writeI2C(fd,dec, 0x0e, 0x54);
            writeI2C(fd,dec, 0x0f, 0x76);
        }
        else if(4 == ch_num)
        {
            writeI2C(fd,dec, 0x06, 0x1b);
            writeI2C(fd,dec, 0x08, 0x01);//
            writeI2C(fd,dec, 0x0e, 0x32);
        }
        writeI2C(fd,dec, 0x07, 0x80|(samplerate<<3)|(bits<<2));	//Rec I2C 16K 16bit : master
        writeI2C(fd,dec, 0x13, 0x80|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : master 320fps
        writeI2C(fd,dec, 0x23, 0x10);  // Audio playback out
        writeI2C(fd,dec, 0x14, 0x00);  //
            //zly audio mixing
        writeI2C(fd,dec, 0x16, 0x88);  //
        writeI2C(fd,dec, 0x17, 0x88);  //
        writeI2C(fd,dec, 0x18, 0x88);
        writeI2C(fd,dec, 0x19, 0x88);
        writeI2C(fd,dec, 0x1A, 0x88);
        writeI2C(fd,dec, 0x1B, 0x88);
        writeI2C(fd,dec, 0x1C, 0x88);
        writeI2C(fd,dec, 0x1D, 0x88);
        writeI2C(fd,dec, 0x1E, 0x88);
        writeI2C(fd,dec, 0x1D, 0x88);
        writeI2C(fd,dec, 0x20, 0x88);
        writeI2C(fd,dec, 0x21, 0x88);

        //
        writeI2C(fd,dec, 0x0a, 0x10);  //
        writeI2C(fd,dec, 0x0b, 0x32);  //
        writeI2C(fd,dec, 0x0c, 0x54);
        writeI2C(fd,dec, 0x0d, 0x76);
        writeI2C(fd,dec, 0x0e, 0x98);
        writeI2C(fd,dec, 0x0f, 0xBA);
        writeI2C(fd,dec, 0x10, 0xDC);
        writeI2C(fd,dec, 0x11, 0xFE);
        writeI2C(fd,dec, 0x12, 0xE4);
        writeI2C(fd,dec, 0x23, 0x10); //-----//first stage playback audio

    }
    else
    {
        writeI2C(fd,dec, 0x07, 0x80|(samplerate<<3)|(bits<<2));	//Rec I2C 16K 16bit : master
        writeI2C(fd,dec, 0x13, 0x80|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : master
        //zly audio mixing
        writeI2C(fd,dec, 0x06, 0x1a);
        writeI2C(fd,dec, 0x16, 0x88);  //
        writeI2C(fd,dec, 0x17, 0x88);  //
        writeI2C(fd,dec, 0x18, 0x88);
        writeI2C(fd,dec, 0x19, 0x88);
        writeI2C(fd,dec, 0x1A, 0x88);   writeI2C(fd,dec, 0x16, 0x88);  //
        writeI2C(fd,dec, 0x1B, 0x88);
        writeI2C(fd,dec, 0x1C, 0x88);
        writeI2C(fd,dec, 0x1D, 0x88);
        writeI2C(fd,dec, 0x1E, 0x88);
        writeI2C(fd,dec, 0x20, 0x88);
        writeI2C(fd,dec, 0x21, 0x88);
        writeI2C(fd,dec, 0x06, 0x5a);
        writeI2C(fd,dec, 0x23, 0x19); //-----//first stage playback audio

    }
    writeI2C(fd,dec, 0x01, 0x88);  // Audio input gain initpstAioAttr->u32ChnCnt = 2;
    writeI2C(fd,dec, 0x02, 0x88);
    writeI2C(fd,dec, 0x03, 0x88);
    writeI2C(fd,dec, 0x04, 0x88);
    writeI2C(fd,dec, 0x05, 0x88);

	writeI2C(fd,dec, 0x22, 0x80);  //aogain
	writeI2C(fd,dec, 0x22, (unsigned char)audiogain);//audiogain

	writeI2C(fd,dec, 0x24, 0x18); //first stage playback audio
	writeI2C(fd,dec, 0x25, 0x16); //first stage playback audio

	close(fd);

}

/******************************************************************************
 * function : show usage
 ******************************************************************************/
void SAMPLE_VENC_Usage(char *sPrgNm) {
	printf("Usage : %s <index>\n", sPrgNm);
	printf("index:\n");
	printf("\t 0) 4D1 H264 encode.\n");
	printf("\t 1) 1*720p H264 encode.\n");
	printf("\t 2) 1D1 MJPEG encode.\n");
	printf("\t 3) 4D1 JPEG snap.\n");
	printf("\t 4) 8*Cif JPEG snap.\n");
	printf("\t 5) 1D1 User send pictures for H264 encode.\n");
	printf("\t 6) 4D1 H264 encode with color2grey.\n");
	return;
}

/******************************************************************************
 * function : to process abnormal case
 ******************************************************************************/
void SAMPLE_VENC_HandleSig(HI_S32 signo) {
	if (SIGINT == signo || SIGTSTP == signo) {
		SAMPLE_COMM_SYS_Exit();
		printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
	}
    printf("SAMPLE_VENC_HandleSig\n");
	exit(-1);
}

void SAMPLE_VENC_HandleSigDoNone(HI_S32 signum/*,siginfo_t *info,void *myact*/)
{
    //do nothing
    printf("receive SIGPIPE signal\n");

}

/*****************************************************************************VoChn = 0;
 VpssGrp = 0;
 SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);*
 * function : to process abnormal case - the case of stream venc
 ******************************************************************************/
void SAMPLE_VENC_StreamHandleSig(HI_S32 signo) {

	if (SIGINT == signo || SIGTSTP == signo) {
		SAMPLE_COMM_SYS_Exit();
		printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
	}
    printf("SAMPLE_VENC_StreamHandleSig exit\n");
	exit(0);
}
HI_S32 SubStreamstop(HI_U8 channel)
{
    substream_stPara[channel].bThreadStart = HI_FALSE;
    VENC_GRP VencGrp=channel;//4
    VENC_CHN VencChn=channel;//4

    VPSS_GRP VpssGrp=channel;//0123

    SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);

	//stop substream encode
}
HI_S32 SNAPOneshot(HI_VOID *p) {
	HI_S32 s32Ret = HI_SUCCESS;
	/******************************************
	 step 5: snap process
	 ******************************************/
	VENC_GRP VencGrp = 8;
	VENC_CHN VencChn = 8;//channel 8 as SNAP encode channel
	HI_S32 i;
	SIZE_S stSize;
	HI_U32 u32ViChnCnt = 4;
	VPSS_GRP VpssGrp;
	HI_U32 Snapcnt=10;
	/*snap Cif pic*/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;

	}
	s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start snap failed!\n");
		goto END_VENC_SNAP_0;
	}

	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VpssGrp =i;
		s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp,
				VPSS_BYPASS_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("snap process failed!\n");
			goto END_VENC_SNAP_1;
		}
        printf("snap chn %d ok!\n", VpssGrp);

		sleep(1);
	}
	END_VENC_SNAP_1:
	    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
	END_VENC_SNAP_0:

	return 0;
}

HI_S32 SNAPshot(HI_VOID *p) {


	HI_S32 s32Ret = HI_SUCCESS;
	/******************************************
	 step 5: snap process
	 ******************************************/
	VENC_GRP VencGrp = 8;
	VENC_CHN VencChn = 8;//channel 8 as SNAP encode channel
	HI_S32 i,j;
	SIZE_S stSize;
	HI_U32 u32ViChnCnt = 4;
	VPSS_GRP VpssGrp;

	snap_pic_contol* snapcontrol=(snap_pic_contol *)p;
	HI_U32 Snapcnt=snapcontrol->piccount;


	/*snap Cif pic*/
    ////////////////////////////bu biao biao ji///////////////////////
    if(minisStd){
        s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_CIF, &stSize);   //部标用CIF格式
    }
    else{
        s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_CUSTOM, &stSize);
    }
    /////////////////////////////////end////////////////////////////////
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;

	}
	s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start snap failed!\n");
		goto END_VENC_SNAP_0;
	}
	switch (snapcontrol->snaptype) {
	case selected_oneshot:/* 4D1 H264 encode */
		u32ViChnCnt = 1;
		Snapcnt = 1;
		break;
	case all_oneshot:/* 4D1 H264 encode */
		u32ViChnCnt = 4;
		Snapcnt = 1;
		break;
	case selected_continuouslyshot:/* 4D1 H264 encode */
		u32ViChnCnt = 1;
		break;
	case all_continuouslyshot:/* 4D1 H264 encode */
		u32ViChnCnt = 4;
		break;

	default:
		break;
	}

	/////////////////////////////////////////////////////////////
	for(j=0;j<Snapcnt;j++)
	{
		for (i = 0; i < u32ViChnCnt; i++) {
			/*** main frame **/
			if((snapcontrol->snaptype==selected_oneshot)||(snapcontrol->snaptype==selected_continuouslyshot))
							VpssGrp =snapcontrol->videochannel;
			else
					VpssGrp =i;

            ////////////reduce jpg quality by wxb///////////
            VENC_PARAM_JPEG_S pstJpegParam;
            s32Ret = HI_MPI_VENC_GetJpegParam(VencChn, &pstJpegParam);
            if (HI_SUCCESS != s32Ret)
            {
            printf("HI_MPI_VENC_GetJpegParam err 0x%x\n",s32Ret);
            return HI_FAILURE;
            }

            pstJpegParam.u32Qfactor = snapQuality;
//            for(int i=0; i<64; i++){
//                pstJpegParam.u8YQt[i] = 20;
//                pstJpegParam.u8CrQt[i] = 20;
//                pstJpegParam.u8CbQt[i] = 20;
//            }
//            printf("jpg qt: %d, %d, %d %d\n", pstJpegParam.u8CbQt[0], pstJpegParam.u8CrQt[0], pstJpegParam.u8YQt[0],
//                    pstJpegParam.u32MCUPerECS);
            HI_MPI_VENC_SetJpegParam(VencChn, &pstJpegParam);
            /////////////////end///////////////////

			s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp,
					VPSS_BYPASS_CHN);
			if (HI_SUCCESS != s32Ret) {
				SAMPLE_PRT("snap process failed!\n");
				goto END_VENC_SNAP_1;
			}
            printf("snap chn %d ok!\n", VpssGrp);

			sleep(snapcontrol->snapinterval);
		}
	}
	END_VENC_SNAP_1:
	    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
	END_VENC_SNAP_0:

	return 0;
}

HI_S32 SNAPContinuousshot(HI_VOID *p)
{
	HI_S32 s32Ret = HI_SUCCESS;
	/******************************************
	 step 5: snap process
	 ******************************************/
	VENC_GRP VencGrp = 8;
	VENC_CHN VencChn = 8;//channel 8 as SNAP encode channel
	HI_S32 i;
	SIZE_S stSize;
	HI_U32 u32ViChnCnt = 4;
	VPSS_GRP VpssGrp;
	HI_U32 Snapcnt=10;
	/*snap Cif pic*/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;

	}
	s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start snap failed!\n");
		goto END_VENC_SNAP_0;
	}
	VpssGrp =1;
	for (i = 0; i < Snapcnt; i++) {
		/*** main frame **/

		s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp,
				VPSS_BYPASS_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("snap process failed!\n");
			goto END_VENC_SNAP_1;
		}
        printf("snap chn %d ok!\n", VpssGrp);

		sleep(1);
	}
	END_VENC_SNAP_1:
	    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
	END_VENC_SNAP_0:

	return 0;
}
HI_S32 SNAPSHOT_Start(HI_VOID *p)
{
	//snap init
	//start snap thread

	return 0;

}
HI_S32 SNAPSHOT_Stop(HI_VOID *p)
{
	//snap init
	//start snap thread
	return 0;
}

HI_S32 SubStreamEncode(HI_VOID *p)
{
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
    printf("------------------------------picsize=%d\n", picsize);
    PIC_SIZE_E enSize[2] = { PIC_D1, picsize };  //wxb 20140620
	HI_S32 s32Ret = HI_SUCCESS;
	SAMPLE_RC_E enRcMode;
	STREAMINGCONTROL_S* streamcontrol;
	streamcontrol=(STREAMINGCONTROL_S*)p;
	//sub stream video codec group and channel init
	VENC_GRP VencGrp=streamcontrol->streamingid;//4
	VENC_CHN VencChn=streamcontrol->streamingid;//4
	/*** Sub stream **/
	VPSS_GRP VpssGrp=streamcontrol->streamingid-4;//0123
	/******************************************
	 step 6: select rc mode
	 *****************************************/
	printf("start get sub stream \n");

	enRcMode = SAMPLE_RC_CBR;
//4,4,264,,cif cbr
	//create grp&VencChn
	//frame mode
	int i=0;
	for (i =4; i < 8; i++)
	{
		VencGrp=i;
		VencChn=i;
		VpssGrp=i-4;

		s32Ret = SUBSTREAM_VENC_Start(VencGrp, VencChn, enPayLoad[1], gs_enNorm,
				enSize[1], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			//goto END_VENC_8D1_2;
	}
		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
                    SAMPLE_PRT("Start sub Venc failed! Error=%d\n", s32Ret);
					//goto END_VENC_8D1_3;
		}
	}
	/******************************************
		 step 6: stream venc process -- get sub bitstream, then save it to file.
		 ******************************************/
		s32Ret = VENC_GetSubStream(streamcontrol->streamingid);


        printf("streamcontrol->streamingid=%d\n", streamcontrol->streamingid);
		if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("Start Venc failed! Error=%d\n", s32Ret);
			//goto END_VENC_8D1_3;
		}
        else{
            printf("set streamcontrol %d state = true\n", streamcontrol->streamingid);
            streamcontrol->videostate = true;
        }
		////getchar();

}

#define VideoCnt 4
HI_S32 VENC_4D1_H264(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;
	//enViMode =SAMPLE_VI_MODE_8_D1;

	HI_U32 u32ViChnCnt = VideoCnt;
	HI_U32 u32VoChnCnt = VideoCnt;
	HI_S32 s32VpssGrpCnt = VideoCnt;
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
    PIC_SIZE_E enSize[2] = { PIC_D1, PIC_CIF };
    if(minisStd){
        enSize[0] = PIC_CIF; //部标用CIF格式,部标用CIF格式,码率为256，可减小视频文件尺寸
    }
    else{
        char secstr[32] = {0};
        readString("basesetting", "cityConfig", "config0", "/lib/clientcity.ini", secstr, sizeof(secstr));
        //find config
        char tmpstr[32] = {0};
        readString(secstr, "videoFormat", "-1", "/lib/clientcommon.ini", tmpstr, sizeof(tmpstr));
        int tmprate = atoi(tmpstr);
        if(tmprate == 1){
            enSize[0] = PIC_CIF;
            printf("clientcommint config enSize=%d\n", enSize[0]);
        }

    }

	HI_U32 voindex[4] = { 0, 0, 0, 0 };

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;
	//vo zly
	VO_DEV VoDev;
	VO_CHN VoChn;
	VI_CHN ViChn;
	VO_PUB_ATTR_S stVoPubAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
	VPSS_CHN VpssChn_VoHD0 = VPSS_PRE1_CHN;//2014-03-12zly
	static SAMPLE_VENC_GETSTREAM_PARA_S stPara;
//zly

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30; //zly

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);

	}

    //audio init
    s32Ret =initaudiosystem() ;

#if 1
	  AUDIO_DEV  AiDev = SAMPLE_AUDIO_AI_DEV;
	  AENC_CHN   AeChn = 0;
	  AI_CHN     AiChn = 0;
#endif

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");

	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
	}
	Osd_Init(4);//init OSD and start OSD system
	/******************************************
	 step 5: start VO SD0 (bind * vi )
	 ******************************************/
	printf("start vo sd0 bind vi chn0 \n");

	VoDev = SAMPLE_VO_DEV_DHD0;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
	stVoPubAttr.enIntfType = VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	stVoPubAttr.bDoubleFrame = HI_FALSE; //In HD, this item should be set to HI_FALSE

	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, 25);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
		// goto END_720P_3;
	}
	Setup_FB();
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		//goto END_720P_4;
	}
	//VPSS_CHN VpssChn_VoHD0 = VPSS_PRE0_CHN;
	//net to bind four VPSS to VoChn
	for (i = 0; i < u32VoChnCnt; i++) {
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn_VoHD0);
	}
	if(vomode==4)
		SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);//show four channel
	else
	    SAMPLE_VO_CHN_show(VoDev, &stVoPubAttr, enVoMode, vomode);//show vo channel 0

	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_BindVi(vo:%d)-(vichn:%d) failed with %#x!\n",
				VoDev, VoChn, s32Ret);
		// goto END_720P_4;
	}
	/******************************************
	 step 6: select rc mode
	 *****************************************/

	enRcMode = SAMPLE_RC_CBR;
	/******************************************
	 step 5: start main stream venc (big)
	 ******************************************/
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main stream **/
		VencGrp = i;
		VencChn = i;
		VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],	gs_enNorm, enSize[0], enRcMode);
		if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("Start Venc failed! ErrorId=%d\n", s32Ret);
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
		if (HI_SUCCESS != s32Ret) {
            SAMPLE_PRT("Start Venc failed! ErrorId=%d\n", s32Ret);
		}
	}
#if 1
	stPara.AencChn=AeChn;
	stPara.AiChn=AiChn;
	stPara.AiDev=AiDev;
#endif

	stPara.recordpath=recordfilepath;
	stPara.s32Cnt=4;
	/******************************************
	 step 6: stream venc process -- get stream, then save it to file.
	 ******************************************/
	s32Ret = VENC_StartGetMainStream(&stPara);

	if (HI_SUCCESS != s32Ret) {
        SAMPLE_PRT("Start Venc failed! ErrorId=%d\n", s32Ret);
    }
	return s32Ret;
}

void END_VENC_8D1_3()
{
	HI_U32 u32ViChnCnt = 4;
	HI_U32 u32VoChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
	PIC_SIZE_E enSize[2] = { PIC_D1, PIC_CIF };
	HI_U32 voindex[4] = { 0, 0, 0, 0 };
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;
	HI_U32 i;

	for (i = 0; i < u32ViChnCnt ; i++) {
			VencGrp = i;
			VencChn = i;
			VpssGrp = i;
			VpssChn = VPSS_BSTR_CHN;
			SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
			SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
		}
		SAMPLE_COMM_VI_UnBindVpss(enViMode);
		END_VENC_8D1_2:	//vpss stop
		SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
		END_VENC_8D1_1:	//vi stop
		SAMPLE_COMM_VI_Stop(enViMode);
		END_VENC_8D1_0:	//system exit
		stopaudiosystem();
		SAMPLE_COMM_SYS_Exit();

}
void END_VENC_8D1_2()
{

	HI_S32 s32VpssGrpCnt = 4;
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;


		SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
		SAMPLE_COMM_VI_Stop(enViMode);
		stopaudiosystem();
		SAMPLE_COMM_SYS_Exit();
}

void END_VENC_8D1_1()
{

	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

	SAMPLE_COMM_VI_Stop(enViMode);
	stopaudiosystem();
	SAMPLE_COMM_SYS_Exit();
}
void END_VENC_8D1_0()
{
	stopaudiosystem();
	SAMPLE_COMM_SYS_Exit();
}

//2013-12-01
//
/******************************************************************************
 * function :  4D1 H264 encode
 ******************************************************************************/
//set four vo chn.
HI_S32 SAMPLE_VENC_4D1_H264(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

	HI_U32 u32ViChnCnt = 4;
	HI_U32 u32VoChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
	PIC_SIZE_E enSize[2] = { PIC_D1, PIC_CIF };
	HI_U32 voindex[4] = { 0, 0, 0, 0 };

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;
	//vo zly
	VO_DEV VoDev;
	VO_CHN VoChn;
	VI_CHN ViChn;
	VO_PUB_ATTR_S stVoPubAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
	VPSS_CHN VpssChn_VoHD0 = VPSS_PRE1_CHN;//2014-03-12zly
//zly

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30; //zly

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_8D1_0;
	}
	//audio init
	s32Ret =initaudiosystem() ;

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_8D1_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_8D1_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_8D1_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_8D1_2;
	}
	/******************************************
	 step 5: start VO SD0 (bind * vi )
	 ******************************************/
	printf("start vo sd0 bind vi chn0 \n");

	VoDev = SAMPLE_VO_DEV_DHD0;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
	stVoPubAttr.enIntfType = VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	stVoPubAttr.bDoubleFrame = HI_FALSE; //In HD, this item should be set to HI_FALSE

	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, 25);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
		// goto END_720P_3;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
		//goto END_720P_4;
	}
	//VPSS_CHN VpssChn_VoHD0 = VPSS_PRE0_CHN;
	//net to bind four VPSS to VoChn
	for (i = 0; i < u32VoChnCnt; i++) {
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn_VoHD0);
	}

	SAMPLE_VO_CHN_show(VoDev, &stVoPubAttr, enVoMode, 0);//show vo channel 0
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VO_BindVi(vo:%d)-(vichn:%d) failed with %#x!\n",
				VoDev, VoChn, s32Ret);
		// goto END_720P_4;
	}

	/******************************************
	 step 6: select rc mode
	 *****************************************

	while (0) {
		printf("please choose rc mode:\n");
		printf("\t0) CBR\n");
		printf("\t1) VBR\n");
		printf("\t2) FIXQP\n");
		//ch = ////getchar();
		if ('0' == ch) {
			enRcMode = SAMPLE_RC_CBR;
			break;
		} else if ('1' == ch) {
			enRcMode = SAMPLE_RC_VBR;
			break;
		} else if ('2' == ch) {
			enRcMode = SAMPLE_RC_FIXQP;
			break;
		} else {
			printf("rc mode invaild! please try again.\n");
			continue;
		}
	}
	*/
	enRcMode = SAMPLE_RC_CBR;
	/******************************************
	 step 5: start stream venc (big + little)
	 ******************************************/
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main stream **/
		VencGrp = i * 2;
		VencChn = i * 2;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],
				gs_enNorm, enSize[0], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_8D1_2;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_8D1_3;
		}

		/*** Sub stream **/

			VencGrp++;
			VencChn++;
			s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1],
					gs_enNorm, enSize[1], enRcMode);
			if (HI_SUCCESS != s32Ret) {
				SAMPLE_PRT("Start sub Venc failed!\n");
				goto END_VENC_8D1_3;
			}

			s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
			if (HI_SUCCESS != s32Ret) {
				SAMPLE_PRT("Start sub Venc failed!\n");
				goto END_VENC_8D1_3;
			}

	}

	/******************************************
	 step 6: stream venc process -- get stream, then save it to file.
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt * 2);

	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_8D1_3;
	}
	//SAMPLE_VO_CHN_show(VoDev, &stVoPubAttr, enVoMode, 0);

	//////getchar();
	if (1) {

		s32Ret = HI_MPI_VO_SetAttrBegin(VoDev);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start VO failed!\n");
			//goto END_16_2Cif_6;
		}

		s32Ret = HI_MPI_VO_SetAttrEnd(VoDev);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start VO SetAttrEnd!\n");
			//goto END_16_2Cif_6;
		}

	}

	//////getchar();

	/******************************************
	 step 7: exit process
	 ******************************************/
	SAMPLE_COMM_VENC_StopGetStream();

	END_VENC_8D1_3: for (i = 0; i < u32ViChnCnt * 2; i++) {
		VencGrp = i;
		VencChn = i;
		VpssGrp = i / 2;
		VpssChn = (VpssGrp % 2) ? VPSS_PRE0_CHN : VPSS_BSTR_CHN;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	}
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_8D1_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_8D1_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_8D1_0:	//system exit
	stopaudiosystem();
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
 * function :  1HD H264 encode
 ******************************************************************************/
HI_S32 SAMPLE_VENC_1HD_H264(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_720P;

	HI_U32 u32ViChnCnt = 1;
	HI_S32 s32VpssGrpCnt = 1;
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
	PIC_SIZE_E enSize[2] = { PIC_HD720, PIC_D1 };

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_HD720,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 4;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_1HD_0;
	}

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_1HD_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_HD720, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_1HD_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_1HD_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_1HD_2;
	}
	/******************************************
	 step 6: start stream venc (big + little)
	 ******************************************/
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VencGrp = i * 2;
		VencChn = i * 2;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],
				gs_enNorm, enSize[0], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_1HD_3;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_1HD_3;
		}

		/*** Sub frame **/
		VencGrp++;
		VencChn++;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1],
				gs_enNorm, enSize[1], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_1HD_3;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_1HD_3;
		}
	}

	/******************************************
	 step 7: stream venc process -- get stream, then save it to file.
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt * 2);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_1HD_3;
	}

	printf("please press twice ENTER to exit this sample\n");

	/******************************************
	 step 8: exit process
	 ******************************************/
	SAMPLE_COMM_VENC_StopGetStream();

	END_VENC_1HD_3: for (i = 0; i < u32ViChnCnt * 2; i++) {
		VencGrp = i;
		VencChn = i;
		VpssGrp = i / 2;
		VpssChn = (VpssGrp % 2) ? VPSS_PRE0_CHN : VPSS_BSTR_CHN;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	}
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_1HD_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_1HD_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_1HD_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
 * function :  1D1 MJPEG encode
 ******************************************************************************/
HI_S32 SAMPLE_VENC_1D1_MJPEG(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_D1;

	HI_U32 u32ViChnCnt = 1;
	HI_S32 s32VpssGrpCnt = 1;
	PAYLOAD_TYPE_E enPayLoad = PT_MJPEG;
	PIC_SIZE_E enSize = PIC_D1;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	/*video buffer*/
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_MJPEG_0;
	}

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_MJPEG_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_MJPEG_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_MJPEG_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_MJPEG_2;
	}
	enRcMode = SAMPLE_RC_CBR;
	/******************************************
	 step 5: select rc mode
	 ******************************************/

	/******************************************
	 step 5: start stream venc
	 ******************************************/
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VencGrp = i;
		VencChn = i;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad, gs_enNorm,
				enSize, enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_MJPEG_3;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_MJPEG_3;
		}
	}

	/******************************************
	 step 6: stream venc process -- get stream, then save it to file.
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_MJPEG_3;
	}

	printf("please press twice ENTER to exit this sample\n");
	////getchar();
	////getchar();

	/******************************************
	 step 8: exit process
	 ******************************************/
	SAMPLE_COMM_VENC_StopGetStream();

	END_VENC_MJPEG_3: for (i = 0; i < u32ViChnCnt; i++) {
		VencGrp = i;
		VencChn = i;
		VpssGrp = i;
		VpssChn = VPSS_BSTR_CHN;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	}
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_MJPEG_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_MJPEG_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_MJPEG_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
 * function :  4D1 SNAP
 ******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_Snap(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;
	PIC_SIZE_E enSize = PIC_D1;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enSize,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 15;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 15;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_SNAP_0;
	}

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_SNAP_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_SNAP_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_SNAP_2;
	}

	/******************************************
	 step 5: snap process
	 ******************************************/
	VencGrp = 0;
	VencChn = 0;
	s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start snap failed!\n");
		goto END_VENC_SNAP_3;
	}
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp,
				VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("snap process failed!\n");
			goto END_VENC_SNAP_4;
		}
        printf("snap chn %d ok!\n", VpssGrp);

		sleep(1);
	}

	/******************************************
	 step 8: exit process
	 ******************************************/
	printf("snap over!\n");

	END_VENC_SNAP_4: s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Stop snap failed!\n");
		goto END_VENC_SNAP_3;
	}
	END_VENC_SNAP_3: SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_SNAP_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_SNAP_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_SNAP_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
 * function :  16*Cif SNAP
 ******************************************************************************/
HI_S32 SAMPLE_VENC_8_Cif_Snap(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_2Cif;

	HI_U32 u32ViChnCnt = 8;
	HI_S32 s32VpssGrpCnt = 8;
	PIC_SIZE_E enSize = PIC_2CIF;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enSize,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/* hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_SNAP_0;
	}

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_SNAP_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_SNAP_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_SNAP_2;
	}

	/******************************************
	 step 5: snap process
	 ******************************************/
	VencGrp = 0;
	VencChn = 0;
	/*snap Cif pic*/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_CIF, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_SNAP_0;
	}
	s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start snap failed!\n");
		goto END_VENC_SNAP_3;
	}

	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp,
				VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("snap process failed!\n");
			goto END_VENC_SNAP_4;
		}
        printf("snap chn %d ok!\n", VpssGrp);

		sleep(1);
	}

	/******************************************
	 step 8: exit process
	 ******************************************/
	printf("snap over!\n");

	END_VENC_SNAP_4: s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Stop snap failed!\n");
		goto END_VENC_SNAP_3;
	}
	END_VENC_SNAP_3: SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_SNAP_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_SNAP_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_SNAP_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
 * function : 4D1 H264 encode with color2grey
 ******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_H264_COLOR2GREY(HI_VOID) {
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;
	PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
	PIC_SIZE_E enSize[2] = { PIC_D1, PIC_CIF };

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr;
	VENC_GRP VencGrp;
	VENC_CHN VencChn;
	SAMPLE_RC_E enRcMode;
	GROUP_COLOR2GREY_CONF_S stGrpColor2GreyConf;
	GROUP_COLOR2GREY_S stGrpColor2Grey;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_CHAR ch;
	SIZE_S stSize;

	/******************************************
	 step  1: init variable
	 ******************************************/
	memset(&stVbConf, 0, sizeof(VB_CONF_S));

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1,
			SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;

	/*ddr0 video buffer*/
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
	memset(stVbConf.astCommPool[0].acMmzName, 0,
			sizeof(stVbConf.astCommPool[0].acMmzName));

	/*ddr0 hist buf*/
	stVbConf.astCommPool[1].u32BlkSize = (196 * 4);
	stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 3;
	memset(stVbConf.astCommPool[1].acMmzName, 0,
			sizeof(stVbConf.astCommPool[1].acMmzName));

	/******************************************
	 step 2: mpp system init.
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_VENC_16D1_0;
	}

	/******************************************
	 step 3: start vi dev & chn to capture
	 ******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("start vi failed!\n");
		goto END_VENC_16D1_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_VENC_16D1_0;
	}

	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bDrEn = HI_FALSE;
	stGrpAttr.bDbEn = HI_FALSE;
	stGrpAttr.bIeEn = HI_TRUE;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.bHistEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,
			NULL);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_VENC_16D1_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_VENC_16D1_2;
	}

	/******************************************
	 step 5: Set color2grey conf
	 ******************************************/
	stGrpColor2GreyConf.bEnable = HI_TRUE;
	stGrpColor2GreyConf.u32MaxWidth = 720;
	stGrpColor2GreyConf.u32MaxHeight = 576;
	s32Ret = HI_MPI_VENC_SetColor2GreyConf(&stGrpColor2GreyConf);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SetColor2GreyConf failed!\n");
		goto END_VENC_16D1_2;
	}

	/******************************************
	 step 6: select rc mode
	 ******************************************/
		enRcMode = SAMPLE_RC_VBR;
	/******************************************
	 step 7: start stream venc (big + little)
	 ******************************************/
	for (i = 0; i < u32ViChnCnt; i++) {
		/*** main frame **/
		VencGrp = i * 2;
		VencChn = i * 2;
		VpssGrp = i;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],
				gs_enNorm, enSize[0], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_16D1_3;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_16D1_3;
		}

		/*** Enable a grp with color2grey **/
		stGrpColor2Grey.bColor2Grey = HI_TRUE;
		s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("SetGrpColor2Grey failed!\n");
			goto END_VENC_16D1_3;
		}

		/*** Sub frame **/
		VencGrp++;
		VencChn++;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1],
				gs_enNorm, enSize[1], enRcMode);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_16D1_3;
		}

		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_16D1_3;
		}

		/*** Enable a grp with color2grey **/
		stGrpColor2Grey.bColor2Grey = HI_TRUE;
		s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("SetGrpColor2Grey failed!\n");
			goto END_VENC_16D1_3;
		}
	}

	/******************************************
	 step 8: stream venc process -- get stream, then save it to file.
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt * 2);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("Start Venc failed!\n");
		goto END_VENC_16D1_3;
	}

	printf("please press twice ENTER to exit this sample\n");
	////getchar();
	////getchar();

	/******************************************
	 step 9: exit process
	 ******************************************/
	SAMPLE_COMM_VENC_StopGetStream();

	END_VENC_16D1_3: for (i = 0; i < u32ViChnCnt * 2; i++) {
		VencGrp = i;
		VencChn = i;
		VpssGrp = i / 2;
		VpssChn = (VpssGrp % 2) ? VPSS_PRE0_CHN : VPSS_BSTR_CHN;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	}
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
	END_VENC_16D1_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
	END_VENC_16D1_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
	END_VENC_16D1_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

void switchvideo()
{
    VO_DEV VoDev;
    VO_CHN VoChn;
    VPSS_GRP VpssGrp;
    HI_S32 i;
    HI_U32 u32VoChnCnt = VideoCnt;
    HI_U32 u32ViChnCnt = VideoCnt;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHN VpssChn_VoHD0 = VPSS_PRE1_CHN;//2014-03-12zly
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode;
    SAMPLE_RC_E enRcMode;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoad[2] = { PT_H264, PT_H264 };
    PIC_SIZE_E enSize[2] = { PIC_D1, PIC_CIF };
    static SAMPLE_VENC_GETSTREAM_PARA_S stPara;

    VoDev = SAMPLE_VO_DEV_DHD0;
    enVoMode = VO_MODE_4MUX;
    stVoPubAttr.enIntfSync = VO_OUTPUT_1024x768_60;
    stVoPubAttr.enIntfType = VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_FALSE; //In HD, this item should be set to HI_FALSE


    if(vomode==4){
        SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);//show four channel
        printf("SAMPLE_COMM_VO_StartChn 4 \n");
    }
    else
        SAMPLE_VO_CHN_show(VoDev, &stVoPubAttr, enVoMode, vomode);//show vo channel 0

}



/******************************************************************************
 * function    : main()
 * Description : video venc sample
 ******************************************************************************/
int main(int argc, char *argv[]) {
    if (already_running(LOCKFILE)){
        printf("avtest is already exsit!\n");
        return 0;
    }

    char tmpstr[32] = {0};

    char secstr[32] = {0};
    readString("basesetting", "cityConfig", "config0", "/lib/clientcity.ini", secstr, sizeof(secstr));

    //find config
    readString(secstr, "recordFrmCnt", "3500", "/lib/clientcommon.ini", tmpstr, sizeof(tmpstr));
    printf("secstr=%s, tmpstr=%s\n", secstr, tmpstr);
    int tmprate = atoi(tmpstr);
    if(tmprate > 0){
        recordfrmcnt = tmprate;//wxb
        printf("clientcommint config recordfrmcnt=%d\n", recordfrmcnt);
    }

    memset(tmpstr, 0, sizeof(tmpstr));
    readString(secstr, "snapQuality", "20", "/lib/clientcommon.ini", tmpstr, sizeof(tmpstr));
    snapQuality = atoi(tmpstr);
    printf("snapQuality=%d\n", snapQuality);


	HI_S32 s32Ret;
	int oc = -1;
    printf("avtest......................................\n");
	while ((oc = getopt_long(argc, argv, para, longopt, NULL)) != -1) {

		switch (oc) {
		case 'v':
			printf("venc %s\n", version);
			break;
		case 'c':
			livestreamid = atoi(optarg);
			printf("select live straming  %d\n", livestreamid);
			break;
		case 'h':
			printhelp();
			break;
		case 'w':
			workmode = atoi(optarg);
			printf("workmode  is %d\n", workmode);
			break;
		case 'm':
			vomode = atoi(optarg);
			printf("VO mod setting is %d \n", vomode);
			if (vomode > 5)
			{
				vomode = 4;	//debug VO use
				printf("VO mod setting is 0-4 \n");
			}
			break;

		case 'f':
			framerate = atoi(optarg);
			printf("streaming frame rate is %d f\n", framerate);
			break;
		case 'r':
            bitratestream = atoi(optarg);
            printf("streaming bitrate is %d f\n", bitratestream);
			break;
		case 'n':
			recordfrmcnt = atoi(optarg);
			if(recordfrmcnt<3500)
				recordfrmcnt=3500;
            printf("recordfrmcnt is %d f\n", recordfrmcnt);
			break;
        case 't':
            g_ttl = 1;
            printf("tts g_ttl is %d f\n", g_ttl);
            break;
        case 'p':
            picsize = (PIC_SIZE_E)atoi(optarg);
            printf("picsize is %d f\n", picsize);
            break;
		case 0:
			break;
		case ':':
			printf("option %c requires an argument\n", optopt);
			break;
		case '?':
		default:
			printf("option %c is invalid:ignored\n", optopt);
			break;
		}
	}


    readString("basesetting", "mdn", "000000000000", "/lib/mdn.ini", mdn, sizeof(mdn));
    printf("mdn=%s\n", mdn);

    readString("basesetting", "minisStd", "0", "/lib/clientcity.ini", tmpstr, sizeof(tmpstr));
    minisStd = atoi(tmpstr);
    printf("minisStd=%d\n", minisStd);

	signal(SIGINT, SAMPLE_VENC_HandleSig);
	signal(SIGTERM, SAMPLE_VENC_HandleSig);
//    signal(SIGPIPE, SAMPLE_VENC_HandleSigDoNone);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_handler = SAMPLE_VENC_HandleSigDoNone;
    sigaction(SIGPIPE, &act, NULL);

    signal(SIGPIPE, SAMPLE_VENC_HandleSigDoNone);

	//initaudiodevice
	STREAMINGCONTROL_S streamcontrol[8];

	workmode=8;
    struct controltype msg;
    int msgid;
    if((msgid = msgget(controlkey,IPC_CREAT | 0755)) < 0) {
        perror("2217 msgget");
        exit(1);
    }
    printf("control queque msgid is %d\n",msgid);
    streamcontrol[0].streamingid=4;
    streamcontrol[0].videostate=false;
    streamcontrol[1].streamingid=5;
    streamcontrol[1].videostate=false;
	streamcontrol[2].streamingid=6;
    streamcontrol[2].videostate=false;
    streamcontrol[3].streamingid=7;
    streamcontrol[3].videostate=false;

    streamcontrol[4].streamingid=4;
    streamcontrol[4].videostate=false;
    streamcontrol[5].streamingid=5;
    streamcontrol[5].videostate=false;
    streamcontrol[6].streamingid=6;
    streamcontrol[6].videostate=false;
    streamcontrol[7].streamingid=7;
    streamcontrol[7].videostate=false;

	switch (workmode) {
	case 0:/* 4D1 H264 encode */
		s32Ret = SAMPLE_VENC_4D1_H264();
		break;
	case 8:
		s32Ret = VENC_4D1_H264();
        s32Ret = SubStreamEncode(&streamcontrol[2]);
        //stopaudiosystem();//no used
		break;
	case 1:/* 1*720p H264 encode */
		s32Ret = SAMPLE_VENC_1HD_H264();
		break;
	case 2:/* 1D1 MJPEG encode */
		s32Ret = SAMPLE_VENC_1D1_MJPEG();
		break;
	case 3:/* 4D1 JPEG snap */
		s32Ret = SAMPLE_VENC_4D1_Snap();
		break;
	case 4:/* 8*Cif JPEG snap */
		s32Ret = SAMPLE_VENC_8_Cif_Snap();
		break;
	case 5:/* 1D1 User send pictures for H264 encode */
		//s32Ret = SAMPLE_VENC_1D1_USER_SEND_PICTURES();
		break;
	case 6:/* 4D1 H264 encode with color2grey */
		s32Ret = SAMPLE_VENC_4D1_H264_COLOR2GREY();
		break;
	default:
		printf("the index is invaild!\n");
		SAMPLE_VENC_Usage(argv[0]);
		return HI_FAILURE;
	}

    ///////////////////////snap test///////////////////////////
	//start snap thread
//	SNAPOneshot(NULL);//zly
#if 0
    snap_pic_contol tm_snap_pic_contol;
    printf("press any key to snap pic\n");
    getchar();
	printf("selected_oneshot***************************************************\n");
	tm_snap_pic_contol.snaptype=selected_oneshot;
    tm_snap_pic_contol.videochannel=2; //前门
    tm_snap_pic_contol.snapinterval=1;
	tm_snap_pic_contol.piccount=10;
	SNAPshot(&tm_snap_pic_contol);
#endif
//    printf("all_oneshot&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
//    tm_snap_pic_contol.snaptype=all_oneshot;
//    tm_snap_pic_contol.videochannel=0;
//    tm_snap_pic_contol.snapinterval=2;
//    tm_snap_pic_contol.piccount=10;
//    SNAPshot(&tm_snap_pic_contol);
//    printf("selected_continuouslyshot::::::::::::::::::::::::::::::\n");
//    tm_snap_pic_contol.snaptype=selected_continuouslyshot;
//    tm_snap_pic_contol.videochannel=0;
//    tm_snap_pic_contol.snapinterval=2;
//    tm_snap_pic_contol.piccount=10;
//    SNAPshot(&tm_snap_pic_contol);
//    snap_pic_contol tm_snap_pic_contol;
//    printf("all_continuouslyshot<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
//    tm_snap_pic_contol.snaptype=all_continuouslyshot;
//    tm_snap_pic_contol.videochannel=0;
//    tm_snap_pic_contol.snapinterval=2;
//    tm_snap_pic_contol.piccount=10;
//    SNAPshot(&tm_snap_pic_contol);
    //////////////////snap end///////////////////////////

	//wait for command msgqueque
	char channel;
	char controlcmd;
	controltype* controlcommand;
    msgtype queuemsg;

	for(;;)
	{
        memset(&msg, 0, sizeof(struct controltype));
        //	count++;

        if (msgrcv(msgid, &queuemsg, sizeof(queuemsg.mtext), 0, 0) < 0) {
            perror("msgrcv");
            msgctl(msgid, IPC_RMID,0);
            msgid = msgget(controlkey,IPC_CREAT | 0755);

            continue;
        }

        if(queuemsg.mtype == 1){
            memcpy(&msg, queuemsg.mtext, sizeof(msg));
            channel=msg.videochannel;
            controlcmd=msg.controlcommand;

            printf("command=%d, channel=%d\n", msg.controlcommand, msg.videochannel);

            // add by wxb
            // 100表示切换视频
            if(channel >= 8 && channel == 100){
                //1表示循环切
                if(controlcmd == 1){
                    vomode = (vomode + 1)%5;
                    switchvideo();

                    //start 切换成4路时，会出现只显示1路，其它3路是蓝色空画面的bug。所以这里直接切走
                    //而循环切摄像头时，必须是切第0路，切第1路，切第2路，切第3路，切4路。不能跳过4路，否则会闪。
                    if(vomode==4){
                        vomode = (vomode + 1)%5;
                        switchvideo();
                    }
                    //end
                    printf("swicth video by QT");
                }
                else if(controlcmd == 4){
                    if(vomode != 4){
                        vomode = 4;
                        switchvideo();
                    }
                }
                continue;
            }
            // add end

            if(channel<0 || channel>7){
                continue;
            }

            if(channel>=0 && channel <=3){
                channel += 4;
            }


            printf("streamcontrol[channel].videostate=%d\n", streamcontrol[channel].videostate);

            if(controlcmd==true){
                for(int i=4; i<=7; i++){
                    HI_MPI_VENC_RequestIDR(i);//4567
//                    ChannelStreammark[i-4] = HI_TRUE;
                }
                audiostreamingflag=HI_TRUE;//for audio streaming control
                printf("start video!\n");
            }
            else{
                for(int i=4; i<=7; i++){
//                    ChannelStreammark[i-4] = HI_FALSE;
                }
                audiostreamingflag=HI_FALSE;
                printf("stop video!\n");
            }
        }
        else if(queuemsg.mtype == 2){
            //更新当前站 STRING
            memset(szStation, 0, sizeof(szStation));
            int len = *(queuemsg.mtext) << 8 | *(queuemsg.mtext + 1);
//            strncpy(szStation, queuemsg.mtext + 2, len > sizeof(szStation)-1 ? sizeof(szStation)-1 : len);
            memcpy(szStation, queuemsg.mtext + 2, len);
//            printf("msg2:%s, len=%d\n", szStation, len);
        }
        else if(queuemsg.mtype == 3){
            //更新速度 UINT16
            nSpeed = *(queuemsg.mtext) << 8 | *(queuemsg.mtext + 1);
            if(nSpeed >= 10 && nSpeed < 20){
                nSpeed += 2;
            }
            else if(nSpeed >= 20 && nSpeed <30){
                nSpeed += 3;
            }
            else if(nSpeed >= 30){
                nSpeed += 4;
            }
        }
        else if(queuemsg.mtype == 4){
            //此功能是给麻城项目抓拍用的
            //snapshot
            snap_pic_contol tm_snap_pic_contol;
            printf("selected_oneshot***************************************************\n");
            tm_snap_pic_contol.snaptype=selected_oneshot;
            tm_snap_pic_contol.videochannel=2; //前门
            tm_snap_pic_contol.snapinterval=0;
            tm_snap_pic_contol.piccount=1;
            SNAPshot(&tm_snap_pic_contol);
        }
        else if(queuemsg.mtype == 5){
            //此功能用于部标
            if(minisStd){
                snap_pic_contol tm_snap_pic_contol;
                printf("selected_oneshot***************************************************\n");
                tm_snap_pic_contol.snaptype=selected_oneshot;
                tm_snap_pic_contol.videochannel=2; //前门
                tm_snap_pic_contol.snapinterval=0;
                tm_snap_pic_contol.piccount=1;
                SNAPshot(&tm_snap_pic_contol);

                usleep(200);
                tm_snap_pic_contol.videochannel=1;//车厢
                SNAPshot(&tm_snap_pic_contol);
            }
        }
    }

		printf("program exit abnormally!\n");
	exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
