/*
 * AudioReceiveThread.cpp
 *
 *  Created on: May 16, 2014
 *      Author: root
 */

extern "C" {
#  include <uv.h>
#include "streamer/core/videobuf.h"
}

#include <streamer/core/AudioReceiveThread.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/avstream.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t audiotid;
int result;

void* Audio_Receive_func(void* user)
{
	printf("audio receive thd start........................................!\n");
//	AudioReceiveThread* enc_ptr = static_cast<AudioReceiveThread*>(user);
//	AudioReceiveThread& enc = *enc_ptr;
//
//	VideoStreamer& videostreamer=enc.videostreamer;
//	MemoryPool &avpool=enc.avpool;
//
//	AVPacket* audiopkt;
//	uint64_t start_time = uv_hrtime() / 1000000;
//	uint32_t curr_time = 0;
//	pthread_detach(pthread_self());
//
//	audiobuffer m_audiobuffer;
//
//	unsigned char* tmpmessage;
//	audiobitstream m_audiobitstream;
//	avpool.allocateAudioFrames(10,480);
//	audiopkt=avpool.getFreeAudioPacket();
//    printf("audio receive thd created........................................!\n");
//    init_audio_buffer(&m_audiobuffer, "/lib/tmaudiobuf0", 2, 21);
//    int count = 0;
//	while(!enc.must_stop) {
//
//
//		if(audiopkt!=NULL)
//		{
//			tmpmessage = audiopkt->data;
//			audiobuffer_read(&m_audiobuffer, &m_audiobitstream);
//            if(count++%1000==0){
//                printf("receive audio frame size is %d_______\n",m_audiobitstream.size);
//            }
//
//			memcpy(tmpmessage,m_audiobitstream.videodata,m_audiobitstream.size);
//			curr_time = uint32_t((uv_hrtime() / 1000000) - start_time);
//
//			audiopkt->audiosize=m_audiobitstream.size;
//			audiopkt->makeAudioPacket();
//			audiopkt->setTimeStamp(curr_time);
//			videostreamer.addAudio(audiopkt);
//		}
//		usleep(20);
//	    audiopkt=avpool.getFreeAudioPacket();
//	}
//	audiobuffer_release(&m_audiobuffer);
}

AudioReceiveThread::AudioReceiveThread(VideoStreamer& videostreamer,
		MemoryPool& avpool):videostreamer(videostreamer),avpool(avpool)
{

}

AudioReceiveThread::~AudioReceiveThread() {
	// TODO Auto-generated destructor stub
	result = pthread_join(audiotid, NULL);

}

bool AudioReceiveThread::start() {


	  must_stop = false;
      int m_running;

		pthread_attr_t stAttr;
		struct sched_param stShedParam;
        int stacksize = 10240;
		pthread_attr_init(&stAttr);
        result = pthread_attr_setstacksize(&stAttr, stacksize);
		pthread_attr_getschedparam(&stAttr, &stShedParam);
//		stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-4;
//		pthread_attr_setschedparam(&stAttr, &stShedParam);
	    printf("create pthread\n");
	   // result = pthread_create(&audiotid, &stAttr, Audio_Receive_func, this);
	   result = pthread_create(&audiotid, &stAttr, Audio_Receive_func, this);
        if (result == 0) {
        	pthread_attr_destroy(&stAttr);
            m_running = 1;
        }
        else
        {
            fprintf(stderr, "pthread_create failed:%s\n", strerror(result));
        }

	  state = 1;

	  return true;

}

void AudioReceiveThread::stop() {
	  must_stop = true;
}
