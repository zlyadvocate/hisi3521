/*
 * FlvWritter.h
 *
 *  Created on: Jun 8, 2014
 *      Author: root
 */

#ifndef FLVWRITTER_H_
#define FLVWRITTER_H_
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


inline char* UI08ToBytes(char* buf, unsigned char val)
{
    buf[0] = (char)(val) & 0xff;
    return buf + 1;
}

inline char* UI16ToBytes(char* buf, unsigned short val)
{
    buf[0] = (char)(val >> 8) & 0xff;
    buf[1] = (char)(val) & 0xff;
    return buf + 2;
}

inline char* UI24ToBytes(char* buf, unsigned int val)
{
    buf[0] = (char)(val >> 16) & 0xff;
    buf[1] = (char)(val >> 8) & 0xff;
    buf[2] = (char)(val) & 0xff;
    return buf + 3;
}

inline char* UI32ToBytes(char* buf, unsigned int val)
{
    buf[0] = (char)(val >> 24) & 0xff;
    buf[1] = (char)(val >> 16) & 0xff;
    buf[2] = (char)(val >> 8) & 0xff;
    buf[3] = (char)(val) & 0xff;
    return buf + 4;
}

//inline char* UI64ToBytes(char* buf, unsigned int64 val)
//{
//    buf[0] = (char)(val >> 56) & 0xff;
//    buf[1] = (char)(val >> 48) & 0xff;
//    buf[2] = (char)(val >> 40) & 0xff;
//    buf[3] = (char)(val >> 32) & 0xff;
//    buf[4] = (char)(val >> 24) & 0xff;
//    buf[5] = (char)(val >> 16) & 0xff;
//    buf[6] = (char)(val >> 8) & 0xff;
//    buf[7] = (char)(val) & 0xff;
//    return buf + 8;
//}

inline char* DoubleToBytes(char* buf, double val)
{
    union {
        unsigned char dc[8];
        double dd;
    } d;
    unsigned char b[8];

    d.dd = val;

    b[0] = d.dc[7];
    b[1] = d.dc[6];
    b[2] = d.dc[5];
    b[3] = d.dc[4];
    b[4] = d.dc[3];
    b[5] = d.dc[2];
    b[6] = d.dc[1];
    b[7] = d.dc[0];
    memcpy(buf, b, 8);
    return buf + 8;
}

inline unsigned int BytesToUI32(const char* buf)
{
    return ( (((unsigned int)buf[0]) << 24)	& 0xff000000 )
        | ( (((unsigned int)buf[1]) << 16)	& 0xff0000 )
        | ( (((unsigned int)buf[2]) << 8)	& 0xff00 )
        | ( (((unsigned int)buf[3]))		& 0xff );
}



typedef struct PutBitContext
{
    unsigned int bit_buf;
    int bit_left;
    char *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;

static inline void init_put_bits(PutBitContext *s, char *buffer, int buffer_size)
{
    if(buffer_size < 0) {
        buffer_size = 0;
        buffer = 0;
    }

    s->size_in_bits= 8*buffer_size;
    s->buf = buffer;
    s->buf_end = s->buf + buffer_size;
    s->buf_ptr = s->buf;
    s->bit_left=32;
    s->bit_buf=0;
}

static inline void flush_put_bits(PutBitContext *s)
{
    s->bit_buf<<= s->bit_left;
    while (s->bit_left < 32) {
        *s->buf_ptr++=s->bit_buf >> 24;
        s->bit_buf<<=8;
        s->bit_left+=8;
    }
    s->bit_left=32;
    s->bit_buf=0;
}


static inline void put_bits(PutBitContext *s, int n, unsigned int value)
{
    unsigned int bit_buf;
    int bit_left;

    bit_buf = s->bit_buf;
    bit_left = s->bit_left;

    if (n < bit_left) {
        bit_buf = (bit_buf<<n) | value;
        bit_left-=n;
    } else {
        bit_buf<<=bit_left;
        bit_buf |= value >> (n - bit_left);
        UI32ToBytes(s->buf_ptr, bit_buf);
        //AV_WB32(s->buf_ptr, bit_buf);
        //printf("bitbuf = %08x\n", bit_buf);
        s->buf_ptr+=4;
        bit_left+=32 - n;
        bit_buf = value;
    }

    s->bit_buf = bit_buf;
    s->bit_left = bit_left;
}


class Flv_Writter {
public:
	Flv_Writter();
	~Flv_Writter();
	 void Open(const char* filename);

	    void Close();

	    void WriteAACSequenceHeaderTag(int sampleRate, int channel);

	    void WriteAVCSequenceHeaderTag(
	        const char* spsBuf, int spsSize,
	        const char* ppsBuf, int ppsSize);

	    void WriteAACDataTag(const char* dataBuf,
	        int dataBufLen, int timestamp);

	    void WriteAVCDataTag(const char* dataBuf,
	        int dataBufLen, int timestamp, int isKeyframe);

	    void WriteAudioTag(char* buf,
	        int bufLen, int timestamp);

	    void WriteVideoTag(char* buf,
	        int bufLen, int timestamp);

	private:
	    FILE* file_handle_;
	    int time_begin_;

};

#endif /* FLVWRITTER_H_ */
