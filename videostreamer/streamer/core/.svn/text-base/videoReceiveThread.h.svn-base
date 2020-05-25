/*
 * videoReceiveThread.h
 *
 *  Created on: May 28, 2014
 *      Author: root
 */


#ifndef VIDEORECEIVETHREAD_H_
#define VIDEORECEIVETHREAD_H_

#include <streamer/core/EncoderTypes.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

class MemoryPool;
class AVPacket;
class VideoStreamer;

class videoReceiveThread {
public:
	videoReceiveThread(VideoStreamer& videostreamer, MemoryPool &avpool);
	~videoReceiveThread();
	VideoStreamer& videostreamer;
	MemoryPool &avpool;

	socklen_t fromlen;
	struct sockaddr_in addr;
	int addrlen, sock, cnt;
	struct ip_mreq mreq;
	volatile bool must_stop;

	int state;
	pthread_t m_tid;
	int m_running;
	int m_detached;
	struct sched_param schedParam;
	pthread_attr_t attr;


	bool start();
	void stop();
};

#endif /* VIDEORECEIVETHREAD_H_ */
