extern "C" {
#  include <uv.h>
#include "streamer/core/videobuf.h"
}

#include <sstream>
#include <signal.h>
#include <stdio.h>
#include <fstream>
#include <streamer/amf/AMF0.h>
#include <streamer/core/BitStream.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/VideoEncoder.h>

#include <streamer/core/H264Parser.h>
#include <streamer/core/RTMPWriter.h>
#include <streamer/core/EncoderThread.h>
#include <streamer/core/RTMPThread.h>
#include <streamer/core/avstream.h>
#include <streamer/flv/FLVReader.h>
#include <streamer/flv/FLVWriter.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/VideoStreamerConfig.h>
//Audio multicast

#include <streamer/core/AudioReceiveThread.h>
#include <streamer/core/FlvWritter.h>//zly 2014-06-08

#include <sys/msg.h>
#include <getopt.h>
#define MEMORYDEBUG      0

#if MEMORYDEBUG
#include "MemoryTrace.hpp"
#endif



//#if defined(USE_GRAPH)
//# include <streamer/utils/Graph.h>
//#endif


#define TEST_STREAMER_XMLCONFIG 0 /* tests loading a xml file for The VideoStreamer instead of providing the objects manually */
#define TEST_DAEMON 0
#define TEST_VIDEOSTREAMER 1
#define TEST_RTMP_WRTIER 0
#define TEST_FLV_READER 0
#define TEST_FLV_WRITER 0
#define TEST_X264_WRITER 0
#define TEST_AMF0_METADATA 0
#define TEST_H264_PARSER 0
#define TEST_AUDIO_GENERATOR 0
#define TEST_MP3_ENCODER 0
#define USE_AUDIO 0
#define TEST_SUBFILE 0


bool must_run = false;
bool must_stop =true;
bool audio_stop=false;

char g_channel = 4;  //default
char g_channelport[8];
char g_picsize = 0;
char *para = "u:c:p:";

int g_socket = -1;
int g_VideoBufferlength = 20*1024;
bool audiostreamingflag = true;

#define  videochncnt     4


pthread_t tid;
std::string g_sendurl = "rtmp://192.168.1.177/live/test";
std::string vstreamversion="1.2.0.2";
std::string sendurl[4] = {"rtmp://192.168.1.177/live/test0",
                          "rtmp://192.168.1.177/live/test1",
                          "rtmp://192.168.1.177/live/test2",
                          "rtmp://192.168.1.177/live/test3"};
void signal_handler(int p);
void exitprocess(int signum,siginfo_t *info,void *myact);


#define TIMEBASE 39
char messgeheader[4];
typedef unsigned long long  HI_U64;

typedef struct s_videopackinfo{
	int naltype;
	int length;
	int u32Seq;
	int u32PackCount;
	short crc;
	short resv;
	HI_U64   u64PTS;                /*PTS*/

}videopackinfo;

struct msgtype {
  long mtype;
  char mtext[164];
};
struct controltype {
    char videochannel;
    char controlcommand;
};

struct option longopt[] =
{
     {"url", required_argument, 0, 'u'},
     {"channel", required_argument, 0, 'c'},
     {"picsize", required_argument, 0, 'p'},
     {0, 0, 0, 0},
};

#define controlkey 150

#define LOCKFILE "/lib/videostreamer.pid"
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

        /* 鍏堣幏鍙栨枃浠堕攣 */
        if (lockfile(fd) == -1) {
                if (errno == EACCES || errno == EAGAIN) {
                        printf( "file: %s already locked", filename);
                        close(fd);
                        return 1;
                }
                printf("can't lock %s: %m\n", filename);
                exit(1);
        }
        /* 鍐欏叆杩愯瀹炰緥鐨刾id */
        ftruncate(fd, 0);
        sprintf(buf, "%ld", (long)getpid());
        write(fd, buf, strlen(buf) + 1);
        return 0;
}


void commandavtest(bool open, int channel){
    int msgid = -1;
    if((msgid = msgget(controlkey, 0755)) < 0) {
        perror("msgget 150 failed!");
        exit(1);
    }
    printf("in commandavtest, command=%d, channel=%d\n", open, channel);

    msgtype msg;
    memset(&msg,0,sizeof(struct msgtype));

    msg.mtype = 1;
    controltype ct;
    ct.videochannel = channel;
    ct.controlcommand = open;
    memcpy(msg.mtext, &ct, sizeof(ct));


    if(msgsnd(msgid,&msg, sizeof(ct),IPC_NOWAIT) < 0)
    {
        printf ("Error encountered: %s.\n", strerror(errno));
        perror("msgsnd");
        msgctl(msgid, IPC_RMID,0);
        msgget(controlkey,IPC_CREAT | 0755);

        if((msgid = msgget(controlkey, 0755)) < 0) {
            perror("msgget 150 failed!");
            exit(1);
        }
        msgsnd(msgid,&msg, sizeof(ct),IPC_NOWAIT);
    }
}

VideoStreamer* vistreamer[4];
MemoryPool avpool;
#define NOTHREAD 0

void* videoreceivethd(void* user) {
#if NOTHREAD
      pthread_detach(pthread_self());
#endif

#if TEST_VIDEOSTREAMER
	{

		VideoSettings vs;
//        vs.width = 176;
//        vs.height = 144;
        vs.width = 320;
        vs.height = 240;

		vs.fps = 25;
		vs.bitrate = 300;
#if TEST_FLV_WRITER
		Flv_Writter hi_flv_Writter;
#endif

		AudioSettings as;
        as.samplerate = AV_AUDIO_SAMPLERATE_16000;
//        as.samplerate = AV_AUDIO_SAMPLERATE_32000;
//        as.samplerate = AV_AUDIO_SAMPLERATE_11025;
        as.codec_id=AV_AUDIO_CODEC_AAC;
        /// as.samplerate = AV_AUDIO_SAMPLERATE_16000;
		//as.mode = AV_AUDIO_MODE_STEREO;
		as.mode = AV_AUDIO_MODE_MONO;
		as.in_bitsize=AV_AUDIO_BITSIZE_S16;
		as.bitsize = AV_AUDIO_BITSIZE_S16;
        as.bitrate = 16;
		as.quality = 6;

		ServerSettings ss;
        ss.url = g_sendurl;

        printf("videostreamer version=%s!!\n", vstreamversion.c_str());


//		AudioEncoderG711* ae;
//		ae = new AudioEncoderG711;
	    AudioEncoderFAAC* ae;
	    ae = new AudioEncoderFAAC;

        unsigned char m;
        for(m=0; m<videochncnt; m++)
        {
        	ServerSettings vss;
        	        vss.url = sendurl[m];
        	vistreamer[m]=new VideoStreamer(*ae);
        	vistreamer[m]->setVideoSettings(vs);
        	vistreamer[m]->setServerSettings(vss);
            if(m==0)
                vistreamer[m]->setAudioSettings(as);

			if (!vistreamer[m]->setup()) {
				return 0;
			}
			if (!vistreamer[m]->start()) {
				printf(
						"error: something went wrong when starting the streamer.\n");
				return 0;
			}
			usleep(10);
        }

        avpool.allocateVideoFrames(10, g_VideoBufferlength);
		AVPacket* vid_pkt = avpool.getFreeVideoPacket();
		avpool.allocateAudioFrames(10,480);
		AVPacket* audiopkt=avpool.getFreeAudioPacket();

		videobitstream m_videobitstream;
		unsigned char* tmpmessage;
        printf("connect server ok\n");

        //when connected,  make avtest to send the subvideostream
        commandavtest(true, g_channelport[g_channel]);

		uint64_t start_time = uv_hrtime() / 1000000;
		uint32_t curr_time = 0;

		videobuffer m_videobuffer;

	    init_video_buffer(&m_videobuffer, "/lib/tmvideobuf0", 5, 20);

	    must_stop=false;
#if   0
        if(audiostreamingflag)
            Hi_audioreceive.start();
#endif
		while (must_run) {
			videobuffer_read(&m_videobuffer, &m_videobitstream);
            static int reflushcount = 0;
            reflushcount++;

            if(reflushcount == 5){
                system("touch  /lib/tmvideostreamerok.txt");
            }

			   curr_time = uint32_t((uv_hrtime() / 1000000) - start_time);
			 //  printf("av thread\n");
			if(m_videobitstream.naltype==0x99){
				if(audiopkt!=NULL)
				{
					//printf("audio bit\n");
					tmpmessage = audiopkt->data;
					memcpy(tmpmessage,m_videobitstream.videodata,m_videobitstream.size);
					audiopkt->audiosize=m_videobitstream.size;
					audiopkt->makeAudioPacket();
					audiopkt->setTimeStamp(curr_time);
                    if(reflushcount > 200)
                        vistreamer[0]->addAudio(audiopkt);
                    else
                        audiopkt->release();
				}
				{
					usleep(100);
					audiopkt=avpool.getFreeAudioPacket();
				}
			}else
			{
				if (vid_pkt != NULL) {
					//printf("video bit idx is %d \n",m_videobitstream.videochanindex);
					tmpmessage = vid_pkt->data;
					vid_pkt->nal_type = m_videobitstream.naltype;
					vid_pkt->videosize = m_videobitstream.size;
					//printf("000  videosize id %d\n", m_videobitstream.size);
					memcpy(tmpmessage, m_videobitstream.videodata,
							m_videobitstream.size);
					vid_pkt->makeVideoPacket();
					vid_pkt->setTimeStamp(curr_time);
					vistreamer[m_videobitstream.videochanindex]->addVideo(vid_pkt);
				}
					{

					vid_pkt = avpool.getFreeVideoPacket();
					}
			}

		} // while(must_run)
#if   0
        if(audiostreamingflag)
            Hi_audioreceive.stop();
//        videobuffer_release(&m_videobuffer);
#endif
		 for(m=0; m<videochncnt; m++)
		 {
			if (!vistreamer[m]->stop()) {
				printf("error: something went wrong when stopping the streamer.\n");
			}
		 }
		 videobuffer_free(&m_videobuffer);

	}

#endif
}


int main(int argc, char *argv[]) {

    if (already_running(LOCKFILE)){
        printf("videostreamer is already exsit!\n");
        return 0;
    }

    system("rm -f /lib/tmvideostreamerok.txt");

    int oc = -1;
    while((oc = getopt_long(argc, argv, para, longopt, NULL)) != -1)
    {
         switch(oc)
         {
        case 'u':
            g_sendurl = std::string(optarg);
            printf("url is %s\n",g_sendurl.c_str());
                 break;
        case 'c':
            g_channel=atoi(optarg);
            printf("channel setting is %d \n",g_channel);
            if(g_channel<0 || g_channel>7){
                printf("invalid channel id.\n");
                exit(1);
            }
            break;
         case 'p':
             g_picsize = atoi(optarg);
             printf("g_picsize is %d \n",g_picsize);
             if(g_picsize == 0){
                 g_VideoBufferlength = 32*1024;
             }
             else{
                 g_VideoBufferlength = 64*1024;
             }
             break;

         case '?':
         default:
            printf("option %c is invalid:ignored\n",optopt);
            exit(1);
            break;
         }
     }

    for(int i=0; i<4; i++){
        char tmpurl[128] = {0};
        memset(tmpurl, 0, sizeof(tmpurl));
        sprintf(tmpurl, "%s%d", g_sendurl.c_str(), i);
        sendurl[i] = std::string(tmpurl);
        printf("address=%s\n", sendurl[i].c_str());
    }



    //对应avtest中的通道号。只有4~7有效。0对应4，1对应5，2对应6，3对应7
    g_channelport[0] = 4;
    g_channelport[1] = 5;
    g_channelport[2] = 6;
    g_channelport[3] = 7;

    g_channelport[4] = 4;
    g_channelport[5] = 5;
    g_channelport[6] = 6;
    g_channelport[7] = 7;


	// generic signal handler
	signal(SIGINT, signal_handler);
    struct sigaction act;
    int sig = 38;
    int result ;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = exitprocess;
    sigaction(sig, &act, NULL);
    must_run = true;
#if NOTHREAD
	must_run = true;
	pthread_attr_t stAttr;
	struct sched_param stShedParam;
	pthread_attr_init(&stAttr);
	result = pthread_attr_setstacksize(&stAttr, stacksize);
	pthread_attr_getschedparam(&stAttr, &stShedParam);
	stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-4;
	pthread_attr_setschedparam(&stAttr, &stShedParam);
    printf("create pthread\n");

	//int result = pthread_create(&tid, &stAttr, videoreceivethd, NULL);
	result = pthread_create(&tid, NULL, videoreceivethd, NULL);
	if (result == 0) {
		pthread_attr_destroy(&stAttr);
	}
#endif
	videoreceivethd(NULL);

	result = pthread_join(tid, NULL);

	return 1;
}


void signal_handler(int p) {
  printf("verbose: receive signal, stop rtmp writer.\n");
  if(g_channel>=0 && g_channel<=7){
      commandavtest(false, g_channelport[g_channel]);
  }
  must_run = false;
  audio_stop=false;

//  struct ip_mreq mreq;
//  mreq.imr_multiaddr.s_addr = inet_addr(GROUPIP);
//  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
//  setsockopt(g_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

  system("rm -f /lib/tmvideostreamerok.txt");

  //kill this process to exit
  pid_t pid = getpid();
  int ret = kill(pid, SIGKILL);


  printf("kill ret=%d\n", ret);
}

void exitprocess(int signum,siginfo_t *info,void *myact)
{
    signal_handler(0);
}

