/*
 * AudioReceiveThread.h
 *
 *  Created on: May 16, 2014
 *      Author: root
 */

#ifndef AUDIORECEIVETHREAD_H_
#define AUDIORECEIVETHREAD_H_


#include <streamer/core/EncoderTypes.h>
#include <pthread.h>

//extern "C" {
//#  include <uv.h>
//}
class MemoryPool;
class AVPacket;
class VideoStreamer;

class AudioReceiveThread {
public:

	AudioReceiveThread(VideoStreamer& videostreamer, MemoryPool &avpool);
	~AudioReceiveThread();
	VideoStreamer& videostreamer;
	MemoryPool &avpool;

	volatile bool must_stop;
	pthread_mutex_t mutex;
	int state;


	bool start();
	void stop();

};

#endif /* AUDIORECEIVETHREAD_H_ */
