#if !defined(_WIN32)
#  include <unistd.h>
#endif

extern "C" {
#  include <uv.h>
}

#include <iostream>
#include <streamer/core/RTMPThread.h>
#include <streamer/core/RTMPWriter.h>
#include <streamer/core/Log.h>
#include <streamer/flv/FLVWriter.h>


// ---------------------------------------------------

void rtmp_thread_func(void* user) {

  RTMPThread* rtmp_ptr = static_cast<RTMPThread*>(user);
  RTMPThread& rtmp = *rtmp_ptr;
  RTMPWriter& rtmp_writer = rtmp.rtmp_writer;
  std::vector<AVPacket*> todo;

  uint64_t packet_time_max = 0;
  uint64_t bytes_written = 0; // just showing some debug info
  bool must_stop = false;
  pthread_detach(pthread_self());
  double bitrate_now = 0; /* just the current time */
  double bitrate_delay = 1000 * 1000 * 1000; /* used to give you the bitrate - delay in nanoseconds */
  double bitrate_timeout = uv_hrtime() + bitrate_delay;
  double bitrate_kbps = 0; /* bitrate of the last "bitrate_delay" */
  double bitrate_time_started = uv_hrtime();
  rtmp.state = RTMP_STATE_STARTED;

  while(!must_stop) {

	    // get work to process
	    pthread_mutex_lock(&rtmp.RTMP_mutex);
	    {
	      while(rtmp.work.size() == 0) {
	    	  	  pthread_cond_wait(&rtmp.RTMP_condv, &rtmp.RTMP_mutex);
	      }
	      std::copy(rtmp.work.begin(), rtmp.work.end(), std::back_inserter(todo));
	      rtmp.work.clear();
	    }
	    pthread_mutex_unlock(&rtmp.RTMP_mutex);



    std::vector<AVPacket*>::iterator it = todo.begin();

    while(it != todo.end()) {
    	AVPacket* pkt = *it;

      // make sure we stop when we get a stop packet
      if(pkt->nal_type == RTMP_DATA_TYPE_STOP) {
        must_stop = true;
        break;
      }

      if(!pkt->videosize) {
        STREAMER_ERROR("error: zero sized packed!\n");
        ::exit(EXIT_FAILURE);
      }

      packet_time_max = pkt->timestamp;

      bytes_written += pkt->videosize; // just some debug info
//      usleep(5000);
      rtmp_writer.write( reinterpret_cast<unsigned char*>(&pkt->data[0]), pkt->videosize);
      
      bitrate_now = uv_hrtime();
      if(bitrate_now > bitrate_timeout) {
        bitrate_kbps = ((bytes_written * 8) / 1000.0) / ((uv_hrtime() - bitrate_time_started) / 1000000000); //  / ((uv_hrtime() - bitrate_time_started)); // in millis
        printf("++ rtmp kbps: %02.2f\n", bitrate_kbps);
        bitrate_timeout = bitrate_now + bitrate_delay;
      }


      delete pkt;
      pkt = NULL;

      it = todo.erase(it);
    }
  }

  rtmp.state = RTMP_STATE_NONE;
}

// ---------------------------------------------------

RTMPThread::RTMPThread(FLVWriter& flv, RTMPWriter& rtmp) 
  :flv(flv)
  ,rtmp_writer(rtmp)
  ,state(RTMP_STATE_NONE)
{
  pthread_mutex_init(&RTMP_mutex, NULL);
  pthread_cond_init(&RTMP_condv, NULL);
  flv.addListener(this);
}

RTMPThread::~RTMPThread() {
  if(state == RTMP_STATE_STARTED) {
    stop();
  }

  pthread_mutex_destroy(&RTMP_mutex);
  pthread_cond_destroy(&RTMP_condv);

  state = RTMP_STATE_NONE;
}
void* RTMP_thread_func(void* user)
{
	  RTMPThread* rtmp_ptr = static_cast<RTMPThread*>(user);
	  RTMPThread& rtmp = *rtmp_ptr;
	  RTMPWriter& rtmp_writer = rtmp.rtmp_writer;
	  std::vector<AVPacket*> todo;

	  uint64_t packet_time_max = 0;
	  uint64_t bytes_written = 0; // just showing some debug info
	  bool must_stop = false;

//	  double bitrate_now = 0; /* just the current time */
//	  double bitrate_delay = 1000 * 1000 * 1000; /* used to give you the bitrate - delay in nanoseconds */
//	  double bitrate_timeout = uv_hrtime() + bitrate_delay;
//	  double bitrate_kbps = 0; /* bitrate of the last "bitrate_delay" */
//	  double bitrate_time_started = uv_hrtime();
	  rtmp.state = RTMP_STATE_STARTED;
	  printf("RTMP thread created\n");

	  while(!must_stop) {

		    // get work to process
		    pthread_mutex_lock(&rtmp.RTMP_mutex);
		    {
		      while(rtmp.work.size() == 0) {
		    	  	  pthread_cond_wait(&rtmp.RTMP_condv, &rtmp.RTMP_mutex);
		      }
		      std::copy(rtmp.work.begin(), rtmp.work.end(), std::back_inserter(todo));
		      rtmp.work.clear();
		    }
		    pthread_mutex_unlock(&rtmp.RTMP_mutex);

	    std::vector<AVPacket*>::iterator it = todo.begin();
	    while(it != todo.end()) {
	    	AVPacket* pkt = *it;

	      // make sure we stop when we get a stop packet
	      if(pkt->nal_type == RTMP_DATA_TYPE_STOP) {
	        must_stop = true;
	        break;
	      }


	#if defined(USE_GRAPH)
	      network_graph["rtmp"] += pkt->data.size();
	#endif

//	      if(!pkt->videosize) {
//	        STREAMER_ERROR("error: zero sized packed!\n");
//	        ::exit(EXIT_FAILURE);
//	      }

	      packet_time_max = pkt->timestamp;

	      bytes_written += pkt->videosize; // just some debug info

	      rtmp_writer.write( reinterpret_cast<unsigned char*>(&pkt->data[0]), pkt->videosize);
	    //  printf("pkt->release();size id %d\n",pkt->videosize);
	      pkt->release();
	      //
	      it = todo.erase(it);
	    }
	  }

	  rtmp.state = RTMP_STATE_NONE;
}

bool RTMPThread::start() {

	 int stacksize=20480;
  if(state == RTMP_STATE_STARTED) {
    STREAMER_ERROR("error: canot start the rtmp thread because we're already running.\n");
    return false;
  }
  //allocate RTMP data memory pool

  int result ;
	pthread_attr_t stAttr;
	struct sched_param stShedParam;
	pthread_attr_init(&stAttr);
	pthread_attr_getschedparam(&stAttr, &stShedParam);
	stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-2;
	pthread_attr_setschedparam(&stAttr, &stShedParam);
 // uv_thread_create(&thread, rtmp_thread_func, this);
	result = pthread_attr_setstacksize(&stAttr, stacksize);
	result= pthread_create(&m_tid,  &stAttr, RTMP_thread_func, this);
  if (result == 0) {
         m_running = 1;
     }
  rtmpdatapool.allocateVideoFrames(50, 16*1024);
     return result;
}

bool RTMPThread::stop() {

  if(state != RTMP_STATE_STARTED) {
    STREAMER_ERROR("error: cannot stop the rtmp state because we're not running.\n");
    return false;
  }

  // trigger the thread loop/condvar
	AVPacket* rtmpdata = rtmpdatapool.getFreeVideoPacket();

	rtmpdata->type = RTMP_DATA_TYPE_STOP;
  	addPacket(rtmpdata);

  int result;
  if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 0;
        }
    }
    return result;

}

// we take ownership of the packet and we will delete delete it!
void RTMPThread::addPacket(AVPacket* pkt) {
  assert(pkt);
  
#if !defined(NDEBUG)
  if(pkt->type == RTMP_DATA_TYPE_NONE) {
    STREAMER_ERROR("error: the RTMPData packet has an invalid type (RTMP_DATA_TYPE_NONE) (RTMPThread)");
    ::exit(EXIT_FAILURE);
  }
#endif
  
  /* @todo - we need to handle each packet else the header won't be sent
  if(state == RTMP_STATE_NONE) {
    printf("error: we're not handling a packet because we're not started.\n");
    return;
  }
  */
//  printf("RTMPThread::addPacket\n");
  pthread_mutex_lock(&RTMP_mutex);
  {
    work.push_back(pkt);
//    printf("RTMPThread::addpack pushback\n");
  }
  pthread_cond_signal(&RTMP_condv);
  pthread_mutex_unlock(&RTMP_mutex);
}


void RTMPThread::onSignature(BitStream& bs) {

}

void RTMPThread::onTag(BitStream& bs, FLVTag& tag) {

  // @todo - we need to handle each packet else the header won't be sent
  /*
  if(state == RTMP_STATE_NONE) {
    printf("error: we're not handling a tag because we're not started.\n");
  }
  */
	if(state == RTMP_STATE_STARTED)
	{
	    AVPacket* rtmpdata = rtmpdatapool.getFreeVideoPacket();
		unsigned char* tmpmessage;
        usleep(2000);
		if (rtmpdata == NULL) {
			usleep(50000);
			printf("Usllep 50000\n");
			rtmpdata = rtmpdatapool.getFreeVideoPacket();
		}
		if (rtmpdata != NULL) {
					tmpmessage = reinterpret_cast<unsigned char*>(&rtmpdata->data[0]);
//					printf("rtmp thread ontag\n");
					rtmpdata->nal_type = RTMP_DATA_TYPE_AV;
					rtmpdata->timestamp = tag.timestamp;
					rtmpdata->videosize = bs.size();
//					std::copy(bs.getPtr(),bs.getPtr()+bs.size(),rtmpdata->data.begin());
					memcpy(tmpmessage,(unsigned char*)bs.getPtr(),bs.size());
					addPacket(rtmpdata);
		}
		else
		{
			printf("rtmp data is null\n");
		}
	}


}
