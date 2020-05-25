/*
 * HI264RTPPACK.h
 *
 *  Created on: May 13, 2014
 *      Author: root
 */

#ifndef HI264RTPPACK_H_
#define HI264RTPPACK_H_
#include "hi_type.h"
#include <pthread.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */




//////////////////////////////////////////////////////////////////////////////////////////
#define RTP_VERSION 2
pthread_mutex_t RTP_mutex;

// class CH264_RTP_PACK start
typedef struct NAL_msg_s {
	HI_BOOL eoFrame;
	unsigned char type;		// NAL type
	unsigned char *start;	// pointer to first location in the send buffer
	unsigned char *end;	// pointer to last location in send buffer
	unsigned long size;
} NAL_MSG_t;
typedef struct {
	//LITTLE_ENDIAN
	unsigned short cc :4; /* CSRC count                 */
	unsigned short x :1; /* header extension flag      */
	unsigned short p :1; /* padding flag               */
	unsigned short v :2; /* packet type                */
	unsigned short pt :7; /* payload type               */
	unsigned short m :1; /* marker bit                 */

	unsigned short seq; /* sequence number            */
	unsigned long ts; /* timestamp                  */
	unsigned long ssrc; /* synchronization source     */
} rtp_hdr_t;
typedef struct tagRTP_INFO {
	NAL_MSG_t nal;	    // NAL information
	rtp_hdr_t rtp_hdr;    // RTP header is assembled here
	int hdr_len;			// length of RTP header

	unsigned char *pRTP;    // pointer to where RTP packet has beem assembled
	unsigned char *start;	// pointer to start of payload
	unsigned char *end;		// pointer to end of payload

	unsigned int s_bit;		// bit in the FU header
	unsigned int e_bit;		// bit in the FU header
	HI_BOOL FU_flag;		// fragmented NAL Unit flag
} RTP_INFO;

typedef struct HI264_RTP_PACK_s {
//	virtual ~HI264_RTP_PACK();
//	HI264_RTP_PACK(unsigned long H264SSRC,unsigned char H264PAYLOADTYPE,unsigned short MAXRTPPACKSIZE);
	unsigned long H264SSRC;
	unsigned char H264PAYLOADTYPE;
	unsigned short MAXRTPPACKSIZE;
	RTP_INFO m_RTP_Info;
	HI_BOOL m_bBeginNAL;
	unsigned short m_MAXRTPPACKSIZE;
} HI264_RTP_PACK;
//传入Set的数据必须是一个完整的NAL,起始码为0x00000001。
//起始码之前至少预留10个字节，以避免内存COPY操作。
//打包完成后，原缓冲区内的数据被破坏。

unsigned int StartCode(unsigned char *cp) ;
void RTP_init();
void RTP_lock();
void RTP_unlock();
HI_BOOL RTP_Set(HI264_RTP_PACK* RTP_PACK, unsigned char *NAL_Buf,
		unsigned long NAL_Size, unsigned long Time_Stamp,
		HI_BOOL End_Of_Frame);
unsigned char* RTP_Get(HI264_RTP_PACK* RTP_PACK, unsigned short *pPacketSize) ;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI264RTPPACK_H_ */
