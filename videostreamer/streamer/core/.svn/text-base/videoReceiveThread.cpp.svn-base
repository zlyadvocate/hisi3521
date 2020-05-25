/*
 * videoReceiveThread.cpp
 *
 *  Created on: May 28, 2014
 *      Author: root
 */

#include <streamer/core/videoReceiveThread.h>

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

char videostartpack[20] = { 0xd7, 0xee, 0x40, 0xf2, 0xa2, 0xc6, 0xd4, 0x66, 0x2b,
			0x68, 0x07, 0x02, 0x68, 0x7f, 0xa4, 0x2e,0x00,0x00,0x00,0x00 };

void* video_Receive_func(void* user) {

}



videoReceiveThread::videoReceiveThread(VideoStreamer& videostreamer, MemoryPool &avpool):videostreamer(videostreamer),avpool(avpool)
{
	// TODO Auto-generated constructor stub

}

videoReceiveThread::~videoReceiveThread() {
	// TODO Auto-generated destructor stub
}

bool videoReceiveThread::start() {
	int addrlen ;

	/* set up socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(VideoBasePort);
	addrlen = sizeof(addr);
	/* receive */
	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}
	mreq.imr_multiaddr.s_addr = inet_addr(GROUPIP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))
			< 0) {
		perror("setsockopt mreq");
		exit(1);
	}

	 int reuse = 1;

	 setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int *) &reuse, sizeof(reuse));

//	  must_stop = false;
//	  uv_thread_create(&thread, video_Receive_func, this);
	  must_stop = false;
		/* Set the thread priority */
		schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO) - 2;
		if (pthread_attr_setschedparam(&attr, &schedParam))
		{
			printf("Failed to set scheduler parameters\n");
			return(-1);
		}

	 // pthread_create(&m_tid, NULL, runThread, NULL);
	//  uv_thread_create(&thread, encode_thread_func, this);
	  // uv_thread_create(&thread, rtmp_thread_func, this);
		int result = pthread_create(&m_tid, &attr, video_Receive_func, this);
		if (result == 0) {
			m_running = 1;
		}
		  state = 1;
		return result;
}

void videoReceiveThread::stop() {
	  must_stop = true;
	  //  uv_thread_join(&thread);
	    int result;
	    if (m_running == 1) {
	          result = pthread_join(m_tid, NULL);
	          if (result == 0) {
	              m_detached = 0;
	          }
	      }
//	      return result;

}
