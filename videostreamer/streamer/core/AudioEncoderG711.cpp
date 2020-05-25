/*
 * AudioEncoderG711.cpp
 *
 *  Created on: May 20, 2014
 *      Author: root
 */

#include <streamer/core/AudioEncoderG711.h>
#define AUDIO_USE_DATA_PTR 0
#define AUDIO_USE_COPY_DATA 1
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635



AudioEncoderG711::AudioEncoderG711()
:samplerate(0)
,nchannels(0)
,bitrate_timeout(0)
,bitrate_delay(1000 * 1000 * 1000)
,bitrate_in_kbps(0.0)
,bitrate_nbytes(0.0)
,bitrate_time_started(0)
{
	// TODO Auto-generated constructor stub

}

AudioEncoderG711::~AudioEncoderG711() {
	// TODO Auto-generated destructor stub
	shutdown();

	  nchannels = 0;
	  bitrate_timeout = 0;
	  bitrate_delay = 0;
	  bitrate_in_kbps = 0;
	  bitrate_nbytes = 0;
}

bool AudioEncoderG711::setup(AudioSettings s) {
	  settings = s;
	  return true;
}

bool AudioEncoderG711::initialize() {
//	  if(settings.bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
//	    STREAMER_ERROR("error: cannot initialize AudioEncoder because the set bitsize is invalid.\n");
//	    return false;
//	  }
//
//	  if(settings.in_bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
//	    settings.in_bitsize = settings.bitsize;
//	  }
//
//	  if(settings.mode == AV_AUDIO_MODE_UNKNOWN) {
//	    STREAMER_ERROR("error: cannot initialize AudioEncoder because the mode (mono/stereo) is invalid.\n");
//	    return false;
//	  }
//
//	  if(settings.samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
//	    STREAMER_ERROR("error: cannot intialize the AudioEncoder because the samplerate is invalid.\n");
//	    return false;
//	  }
//
//	  if(!settings.bitrate) {
//	    STREAMER_ERROR("error: cannot initialize the AudioEncoder because the bitrate was not set.\n");
//	    return false;
//	  }

//	  if(settings.samplerate == AV_AUDIO_SAMPLERATE_44100) {
//	    samplerate = 44100;
//	  }
//	  else if(settings.samplerate == AV_AUDIO_SAMPLERATE_22050) {
//	    samplerate = 22050;
//	  }
//	  else {
//	    STREAMER_ERROR("error: invalid samplerate given for the AudioEncoder.\n");
//	    return false;
//	  }
//
//	  if(settings.mode == AV_AUDIO_MODE_STEREO) {
//	   // mode = STEREO;
//	    nchannels = 2;
//	  }
//	  else if(settings.mode == AV_AUDIO_MODE_MONO) {
//	   // mode = MONO;
//	    nchannels = 1;
//	    STREAMER_ERROR("error: for now we only implement stereo audio.\n");
//	    return false;
//	  }
//	  else {
//	    STREAMER_ERROR("error: invalid mode given for the AudioEncoder.\n");
//	    return false;
//	  }
	  bitrate_time_started = uv_hrtime();
	  bitrate_timeout = bitrate_time_started + bitrate_delay;

	  return true;

}

//
bool AudioEncoderG711::encodePacket(AVPacket* p, FLVTag& tag) {
	  int nsamples = 0;

	  int written=p->audiosize;
	  printf("audiosize if %d\n",written);
	  if(written > 0) {
	    bitrate_nbytes += written;
	  }
	  uint64_t time_now = uv_hrtime();
	  if(time_now >= bitrate_timeout) {
	    bitrate_timeout = time_now + bitrate_delay;
	    double duration = (time_now - bitrate_time_started) / 1000000000.0; // in s.
	    bitrate_in_kbps = ((bitrate_nbytes * 8) / 1000) / duration;
	    STREAMER_STATUS("audio bitrate: %0.2f kbps\n", bitrate_in_kbps);
	  }

#if AUDIO_USE_DATA_PTR
  if(written) {
    tag.setData(mp3_buffer, written);
  }
#elif AUDIO_USE_COPY_DATA
  tag.bs.clear();
  if(written) {
//	  tag.bs.putBytes((uint8_t*)G711_buffer, written);
	  tag.bs.putBytes(reinterpret_cast<unsigned char*> (&p->data[0]), written);
	  tag.setData(tag.bs.getPtr(), tag.bs.size());
  }
#endif

  tag.makeAudioTag();
  tag.setTimeStamp(p->timestamp);

  return written > 0;


}

bool AudioEncoderG711::shutdown() {
	  memset(G711_buffer, 0x00, G711BUFFSIZE);

	  return false;

}
