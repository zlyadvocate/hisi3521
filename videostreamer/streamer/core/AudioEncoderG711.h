/*
 * AudioEncoderG711.h
 *
 *  Created on: May 20, 2014
 *      Author: root
 */

#ifndef AUDIOENCODERG711_H_
#define AUDIOENCODERG711_H_

#include <streamer/core/AudioEncoder.h>
#define G711BUFFSIZE 1024
class AudioEncoderG711: public AudioEncoder {
public:
	AudioEncoderG711();
	~AudioEncoderG711();
	bool setup(AudioSettings s);
	bool initialize();
	bool encodePacket(AVPacket* p, FLVTag& tag);
	bool shutdown();
private:
	int nchannels;
	AudioSettings settings;
	int samplerate;

	uint8_t G711_buffer[G711BUFFSIZE];

	/* used to monitor bitrate */
	uint64_t bitrate_time_started; /* when we started with encoding, in nanosec. we've put this in setup() because we don't want to add a check in encodePacket(), this result in a bit less accurate value for the first run. */
	uint64_t bitrate_timeout; /* when we should calculate the current bitrate again */
	uint64_t bitrate_delay; /* the time between bitrate measurements, in nanosec */
	double bitrate_in_kbps; /* the current bitrate we measured last delay */
	double bitrate_nbytes; /* the total amount of transferred bytes */
};

#endif /* AUDIOENCODERG711_H_ */
