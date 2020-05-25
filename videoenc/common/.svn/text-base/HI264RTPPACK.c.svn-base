/*
 * HI264RTPPACK.c
 *
 *  Created on: May 13, 2014
 *      Author: root
 */
#include "HI264RTPPACK.h"


unsigned int StartCode(unsigned char *cp) {
	unsigned int d32;
	d32 = cp[3];
	d32 <<= 8;
	d32 |= cp[2];
	d32 <<= 8;
	d32 |= cp[1];
	d32 <<= 8;
	d32 |= cp[0];
	return d32;
}

void RTP_init() {
	pthread_mutex_init(&RTP_mutex, NULL);
}
void RTP_lock() {
	pthread_mutex_lock(&RTP_mutex);
}
void RTP_unlock() {
	pthread_mutex_unlock(&RTP_mutex);
}

HI_BOOL RTP_Set(HI264_RTP_PACK* RTP_PACK, unsigned char *NAL_Buf,
		unsigned long NAL_Size, unsigned long Time_Stamp,
		HI_BOOL End_Of_Frame) {
//RTP_lock();
	unsigned long startcode = StartCode(NAL_Buf);

	if (startcode != 0x01000000) {
		return HI_FALSE;
	}

	int type = NAL_Buf[4] & 0x1f;
	if (type < 1 || type > 12) {
		return HI_FALSE;
	}

	RTP_PACK->m_RTP_Info.nal.start = NAL_Buf;
	RTP_PACK->m_RTP_Info.nal.size = NAL_Size;
	RTP_PACK->m_RTP_Info.nal.eoFrame = End_Of_Frame;
	RTP_PACK->m_RTP_Info.nal.type = RTP_PACK->m_RTP_Info.nal.start[4];
	RTP_PACK->m_RTP_Info.nal.end = RTP_PACK->m_RTP_Info.nal.start
			+ RTP_PACK->m_RTP_Info.nal.size;

	RTP_PACK->m_RTP_Info.rtp_hdr.ts = Time_Stamp;

	RTP_PACK->m_RTP_Info.nal.start += 4;	// skip the syncword

	if ((RTP_PACK->m_RTP_Info.nal.size + 7) > RTP_PACK->m_MAXRTPPACKSIZE) {
		RTP_PACK->m_RTP_Info.FU_flag = HI_TRUE;
		RTP_PACK->m_RTP_Info.s_bit = 1;
		RTP_PACK->m_RTP_Info.e_bit = 0;

		RTP_PACK->m_RTP_Info.nal.start += 1;	// skip NAL header
	} else {
		RTP_PACK->m_RTP_Info.FU_flag = HI_FALSE;
		RTP_PACK->m_RTP_Info.s_bit = RTP_PACK->m_RTP_Info.e_bit = 0;
	}

	RTP_PACK->m_RTP_Info.start = RTP_PACK->m_RTP_Info.end =
			RTP_PACK->m_RTP_Info.nal.start;
	RTP_PACK->m_bBeginNAL = HI_TRUE;
//RTP_unlock();
	return HI_TRUE;
}
//循环调用Get获取RTP包，直到返回值为NULL
unsigned char* RTP_Get(HI264_RTP_PACK* RTP_PACK, unsigned short *pPacketSize) {
//RTP_lock();
	if (RTP_PACK->m_RTP_Info.end == RTP_PACK->m_RTP_Info.nal.end) {
		*pPacketSize = 0;
		return NULL;
	}

	if (RTP_PACK->m_bBeginNAL) {
		RTP_PACK->m_bBeginNAL = HI_FALSE;
	} else {
		RTP_PACK->m_RTP_Info.start = RTP_PACK->m_RTP_Info.end;// continue with the next RTP-FU packet
	}

	int bytesLeft = RTP_PACK->m_RTP_Info.nal.end - RTP_PACK->m_RTP_Info.start;
	int maxSize = RTP_PACK->m_MAXRTPPACKSIZE - 12;// sizeof(basic rtp header) == 12 bytes
	if (RTP_PACK->m_RTP_Info.FU_flag)
		maxSize -= 2;

	if (bytesLeft > maxSize) {
		RTP_PACK->m_RTP_Info.end = RTP_PACK->m_RTP_Info.start + maxSize;// limit RTP packetsize to 1472 bytes
	} else {
		RTP_PACK->m_RTP_Info.end = RTP_PACK->m_RTP_Info.start + bytesLeft;
	}

	if (RTP_PACK->m_RTP_Info.FU_flag) {	// multiple packet NAL slice
		if (RTP_PACK->m_RTP_Info.end == RTP_PACK->m_RTP_Info.nal.end) {
			RTP_PACK->m_RTP_Info.e_bit = 1;
		}
	}

	RTP_PACK->m_RTP_Info.rtp_hdr.m = RTP_PACK->m_RTP_Info.nal.eoFrame ? 1 : 0; // should be set at EofFrame
	if (RTP_PACK->m_RTP_Info.FU_flag && !RTP_PACK->m_RTP_Info.e_bit) {
		RTP_PACK->m_RTP_Info.rtp_hdr.m = 0;
	}

	RTP_PACK->m_RTP_Info.rtp_hdr.seq++;

	unsigned char *cp = RTP_PACK->m_RTP_Info.start;
	cp -= (RTP_PACK->m_RTP_Info.FU_flag ? 14 : 12);
	RTP_PACK->m_RTP_Info.pRTP = cp;

	unsigned char *cp2 = (unsigned char *) &(RTP_PACK->m_RTP_Info.rtp_hdr);
	cp[0] = cp2[0];
	cp[1] = cp2[1];

	cp[2] = (RTP_PACK->m_RTP_Info.rtp_hdr.seq >> 8) & 0xff;
	cp[3] = RTP_PACK->m_RTP_Info.rtp_hdr.seq & 0xff;

	cp[4] = (RTP_PACK->m_RTP_Info.rtp_hdr.ts >> 24) & 0xff;
	cp[5] = (RTP_PACK->m_RTP_Info.rtp_hdr.ts >> 16) & 0xff;
	cp[6] = (RTP_PACK->m_RTP_Info.rtp_hdr.ts >> 8) & 0xff;
	cp[7] = RTP_PACK->m_RTP_Info.rtp_hdr.ts & 0xff;

	cp[8] = (RTP_PACK->m_RTP_Info.rtp_hdr.ssrc >> 24) & 0xff;
	cp[9] = (RTP_PACK->m_RTP_Info.rtp_hdr.ssrc >> 16) & 0xff;
	cp[10] = (RTP_PACK->m_RTP_Info.rtp_hdr.ssrc >> 8) & 0xff;
	cp[11] = RTP_PACK->m_RTP_Info.rtp_hdr.ssrc & 0xff;
	RTP_PACK->m_RTP_Info.hdr_len = 12;
	/*!
	 * /n The FU indicator octet has the following format:
	 * /n
	 * /n      +---------------+
	 * /n MSB  |0|1|2|3|4|5|6|7|  LSB
	 * /n      +-+-+-+-+-+-+-+-+
	 * /n      |F|NRI|  Type   |
	 * /n      +---------------+
	 * /n
	 * /n The FU header has the following format:
	 * /n
	 * /n      +---------------+
	 * /n      |0|1|2|3|4|5|6|7|
	 * /n      +-+-+-+-+-+-+-+-+
	 * /n      |S|E|R|  Type   |
	 * /n      +---------------+
	 */
	if (RTP_PACK->m_RTP_Info.FU_flag) {
		// FU indicator  F|NRI|Type
		cp[12] = (RTP_PACK->m_RTP_Info.nal.type & 0xe0) | 28;//Type is 28 for FU_A
		//FU header		S|E|R|Type
		cp[13] = (RTP_PACK->m_RTP_Info.s_bit << 7)
				| (RTP_PACK->m_RTP_Info.e_bit << 6)
				| (RTP_PACK->m_RTP_Info.nal.type & 0x1f); //R = 0, must be ignored by receiver

		RTP_PACK->m_RTP_Info.s_bit = RTP_PACK->m_RTP_Info.e_bit = 0;
		RTP_PACK->m_RTP_Info.hdr_len = 14;
	}
	RTP_PACK->m_RTP_Info.start = &cp[RTP_PACK->m_RTP_Info.hdr_len];	// new start of payload

	*pPacketSize = RTP_PACK->m_RTP_Info.hdr_len
			+ (RTP_PACK->m_RTP_Info.end - RTP_PACK->m_RTP_Info.start);
//RTP_unlock();
	return RTP_PACK->m_RTP_Info.pRTP;

}
