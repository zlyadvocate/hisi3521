#include <streamer/core/MemoryPool.h>
#include <iterator>

MemoryPool::MemoryPool() {
	pthread_mutex_init(&video_mutex, NULL);
	pthread_mutex_init(&audio_mutex, NULL);
}

MemoryPool::~MemoryPool() {
	pthread_mutex_destroy(&video_mutex);
	pthread_mutex_destroy(&audio_mutex);
}

bool MemoryPool::allocateVideoFrames(size_t nframes, uint32_t nbytes) {

  lockVideo();

  try{
    for(size_t i = 0; i < nframes; ++i) {
      AVPacket* pkt = new AVPacket(this);
      pkt->allocate(nbytes);
      pkt->makeVideoPacket();
      pkt->refcount = 0; // make sure refcount starts at zero, so it's free
      video_packets.push_back(pkt);
    }
  }catch (std::string e){
	  unlockVideo();
      throw e;
      return false;
  }
  unlockVideo();
  return true;
}

bool MemoryPool::allocateAudioFrames(size_t nframes, uint32_t nbytes) {

  lockAudio();
  try{

    for(size_t i = 0; i < nframes; ++i) {
      AVPacket* pkt = new AVPacket(this);
      pkt->allocate(nbytes);
      pkt->makeAudioPacket();
      pkt->refcount = 0; // make sure refcount starts at zero, so it's free
      audio_packets.push_back(pkt);
    }
  }catch (std::string e){
	  unlockAudio();
      throw e;
      return false;
  }
  unlockAudio();

  return true;
}

