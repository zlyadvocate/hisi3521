/*
 * videobuf.h
 *
 *  Created on: Jun 27, 2014
 *      Author: root
 */

#ifndef VIDEOBUF_H_
#define VIDEOBUF_H_
#define VIDEOBUFLENGTH 20*1024
#define AUDIOBUFLENGTH 500
typedef struct video_bit {
	int videochanindex;
	int videoseq;
	int naltype;
	int size;
	int reserved;
	char videodata[VIDEOBUFLENGTH];
} videobitstream;

typedef struct audio_bit {
	int videochanindex;
	int videoseq;
	int naltype;
	int size;
	int reserved;
	char videodata[AUDIOBUFLENGTH];
} audiobitstream;

typedef struct audio_buffer {

	caddr_t shared_memory; /* shared memory base address */
	int fd;
	int in; /* pointer to logical 'in' address for writer */
	int out; /* pointer to logical 'out' address for reader */
	int size;
	int buffercnt;

	audiobitstream *buffer; /* logical base address for buffer */

	/* semaphore elements */
	int semid; /* identifier for a semaphore set */

} audiobuffer;

typedef struct video_buffer {

	caddr_t shared_memory; /* shared memory base address */
	int fd;
	int in; /* pointer to logical 'in' address for writer */
	int out; /* pointer to logical 'out' address for reader */
	int size;
	int buffercnt;

	videobitstream *buffer; /* logical base address for buffer */

	/* semaphore elements */
	int semid; /* identifier for a semaphore set */

} videobuffer;
//for audio
void init_audio_buffer(audiobuffer* abuf, char * filepath,int buffercnt,int index) ;
void audiobuffer_read(audiobuffer* abuf, audiobitstream* value);
//void audiobuffer_write(audiobuffer* abuf, audiobitstream value);
//void audiobuffer_release(audiobuffer* abuf);
//void audiobuffer_free(audiobuffer* abuf);

//for video
void init_video_buffer(videobuffer* vbuf, char * filepath,int buffercnt,int index) ;
void videobuffer_read(videobuffer* vbuf, videobitstream* value);
void videobuffer_write(videobuffer* vbuf, videobitstream value);
void videobuffer_release(videobuffer* vbuf);
void videobuffer_free(videobuffer* vbuf);
//void release_videoBuffer(int index);
//void release_allvideoBuffer( );
//void free_videoBuffer(int index);





#endif /* VIDEOBUF_H_ */
