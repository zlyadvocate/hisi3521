// @todo - cleanup the encoder thread
#include <iostream>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <streamer/flv/FLVWriter.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/Log.h>
#include <streamer/core/EncoderThread.h>


#define ENCODEDEBUG

//void*  encode_thread_func(void* user)//UV version
void*  encode_thread_func(void* user)
{
	  EncoderThread* enc_ptr = static_cast<EncoderThread*>(user);
	  EncoderThread& enc = *enc_ptr;
	  VideoEncoder& vid_enc = enc.video_encoder;
	  AudioEncoder& audio_enc = enc.audio_encoder;
	  FLVWriter& flv = enc.flv;
	  FLVTag flv_tag;
      int count = 0;
	  pthread_detach(pthread_self());

	  uint64_t last_timestamp = 0;
	  uint64_t nframes = 0;
	  std::vector<AVPacket*> todo;
	  printf("encode thread created####\n");
	  enc.state = ENCT_STATE_STARTED;

	  while(!enc.must_stop) {

		  // get work to process
			  pthread_mutex_lock(&enc.mutex);
		    {

		      while(enc.work.size() == 0) {
		    	  pthread_cond_wait(&enc.cv, &enc.mutex);
		        //
		      }

		      std::copy(enc.work.begin(), enc.work.end(), std::back_inserter(todo));
		      enc.work.clear();
		    }
		    pthread_mutex_unlock(&enc.mutex);

	    // process the new work
	    for(std::vector<AVPacket*>::iterator it = todo.begin(); it != todo.end(); ++it) {
	      AVPacket& pkt = **it;
#if 0
	      printf("AV_TYPE is %d\n",pkt.type);
#endif

	      if(pkt.type == AV_TYPE_VIDEO) {
	    	  switch (pkt.nal_type) {
				case 0x06:
				//	printf("set SEI data \n");
					flv.setSEIData(reinterpret_cast<unsigned char*>(&pkt.data[4]),pkt.videosize-4);//0x05
					break;
				case 0x08:
                  if(count++%10==0)
                      printf("writeDecoderConfiguration \n");
					 flv.writeDecoderConfiguration(pkt.timestamp);
									break;
				case 0x05:
				case 0x01:
		    		  if(vid_enc.encodePacket(&pkt, flv_tag)){
	//	    			  printf("AV_TYPE\n");

						  nframes++;
	//					  printf(" EncoderThread handle a AVPacket with type: %d\n", pkt.type);
						  flv.writeVideoTag(flv_tag);
						}
						break;
				default:
					break;
			}

	      }
	      else if(pkt.type == AV_TYPE_AUDIO) {
	    	  if(audio_enc.encodePacket(&pkt, flv_tag)){
	            STREAMER_ERROR(" EncoderThread handle a AVPacket with type: %d\n", pkt.type);
	          flv.writeAudioTag(flv_tag);

	        }
	      }
	      else {
	        printf("unknown type\n");
	      }

	      last_timestamp = pkt.timestamp;
#if 0
	      printf(" pkt.release %d\n", pkt.type);
#endif
	      pkt.release();


	    }

	    todo.clear();
	  }

	  // reset state when thread stops
	  uv_mutex_lock(&enc.mutex);
	    enc.work.clear();
	    enc.state = ENCT_STATE_NONE;
	    enc.flv.close();
	  uv_mutex_unlock(&enc.mutex);

	  enc.audio_encoder.shutdown();
	  enc.video_encoder.shutdown();
}
// -------------------------------------------------

EncoderThread::EncoderThread(FLVWriter& flv, VideoEncoder& venc, AudioEncoder& aenc) 
  :flv(flv)
  ,audio_encoder(aenc)
  ,video_encoder(venc)
  ,must_stop(true)
  ,state(ENCT_STATE_NONE)
{
	  pthread_mutex_init(&mutex, NULL);
	  pthread_cond_init(&cv, NULL);
}

EncoderThread::~EncoderThread() {

  if(!must_stop) {
    stop();
  }
 
//  // cleanup
  // cleanup
  int result;
  if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 0;
        }
    }

//  uv_thread_join(&thread);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cv);

  must_stop = true;      

  state = ENCT_STATE_NONE;
}

bool EncoderThread::start() {
	int result;
  
  if(state == ENCT_STATE_STARTED) {
    STREAMER_ERROR("error: already started! first stop().\n");
    return false;
  }

  if(!must_stop) {
    STREAMER_ERROR("error: seems like the encoder thread is already/still running.\n");
    return false;
  }

  must_stop = false;
  int stacksize=20480;
	pthread_attr_t stAttr;
	struct sched_param stShedParam;
	pthread_attr_init(&stAttr);
	pthread_attr_getschedparam(&stAttr, &stShedParam);
	stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-3;
	pthread_attr_setschedparam(&stAttr, &stShedParam);
//
//     uv_thread_create(&thread, encode_thread_func, this);
//  // uv_thread_create(&thread, rtmp_thread_func, this);
	result = pthread_attr_setstacksize(&stAttr, stacksize);

	result = pthread_create(&m_tid, &stAttr, encode_thread_func, this);


	if (result == 0) {
		m_running = 1;
		pthread_attr_destroy(&stAttr);
	}
	else
	{
		fprintf(stderr, "pthread_create failed:%s\n", strerror(result));
		sleep(10);
		result = pthread_create(&m_tid, NULL, encode_thread_func, this);
				return 5;

	}
	return 0;

}

bool EncoderThread::stop() {

  if(must_stop) {
    STREAMER_ERROR("error: seems that we've already stoppped the encoder thread.\n");
    return false;
  }

  must_stop = true;

  // we must trigger the thread conditional loop 
  AVPacket* stop_pkt = new AVPacket(NULL);
  addPacket(stop_pkt);
//
//  uv_thread_join(&thread);
  int result;
  if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 0;
        }
    }
    return true;

}

// we take ownership of the packet and we will delete delete it!
void EncoderThread::addPacket(AVPacket* pkt) {

  if(state == ENCT_STATE_NONE) {
	  pkt->release();
	  return;
  }

  pthread_mutex_lock(&mutex);
  {
    work.push_back(pkt);
  }
  pthread_cond_signal(&cv);

  pthread_mutex_unlock(&mutex);
}
