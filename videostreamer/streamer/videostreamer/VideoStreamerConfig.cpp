#include <streamer/videostreamer/VideoStreamerConfig.h>
//#include <xmlconfig/Config.h>
#include <iostream>

// ---------------------------------------
StreamerConfiguration::StreamerConfiguration() 
  :id(0)
{
}

bool StreamerConfiguration::validate() {
  // @todo - use the same logic as in DaemonConfig
  return video.validate() && audio.validate() && server.validate();
}

void StreamerConfiguration::print() {
  video.print();
  audio.print();
}

bool StreamerConfiguration::hasAudio() {
  return audio.samplerate || audio.bitsize || audio.quality;
}

bool StreamerConfiguration::hasVideo() {
  return video.width || video.height || video.fps;
}

bool StreamerConfiguration::hasServer() {
  return server.url.size();
}

// ---------------------------------------

VideoStreamerConfig::VideoStreamerConfig() 
  :default_stream_id(0)  
{
}

VideoStreamerConfig::~VideoStreamerConfig() {
  for(std::vector<StreamerConfiguration*>::iterator it = configs.begin(); it != configs.end(); ++it) {
    delete *it;
  }
  configs.clear();
}

bool VideoStreamerConfig::load(std::string filepath) {


}

