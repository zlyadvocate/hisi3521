#include <signal.h>
#include <iostream>
#include <string.h>

#include <streamer/core/RTMPWriter.h>
#include <streamer/core/Log.h>
static const AVal av_setDataFramezly = AVC("@setDataFrame");
// ---------------------------------------------------

void rtmp_sigpipe_handler(int signum) {
  STREAMER_ERROR("rtmpwriter: got sigpipe, the remote server probably shut down.\n");
}

// ---------------------------------------------------

RTMPData::RTMPData()
  :timestamp(0)
  ,type(RTMP_DATA_TYPE_NONE)
{
}

// ---------------------------------------------------

RTMPWriter::RTMPWriter() 
  :rtmp(NULL)
  ,state(RW_STATE_NONE)
  ,cb_disconnect(NULL)
  ,cb_user(NULL)
{

  //#if !defined(NDEBUG)

  RTMP_LogSetLevel(RTMP_LOGINFO);
  RTMP_LogSetOutput(stderr);
  //#endif

  signal(SIGPIPE, rtmp_sigpipe_handler);
}

RTMPWriter::~RTMPWriter() {

  cb_disconnect = NULL;
  cb_user = NULL;

  if(rtmp && state == RW_STATE_INITIALIZED) {
    RTMP_Close(rtmp);
  }

  if(rtmp) {
    RTMP_Free(rtmp);
    rtmp = NULL;
  }

  state = RW_STATE_NONE;
}

bool RTMPWriter::initialize() {

  if(!settings.url.size()) {
    STREAMER_ERROR("error: cannot initialize the RTMP Writer, no url set, call setURL() first.\n");
    return false;
  }

  if(state == RW_STATE_INITIALIZED) {
    STREAMER_ERROR("error: already initialized.\n");
    return false;
  }

  if(rtmp) {
    STREAMER_ERROR("error: already initialized a rtmp context, not creating another one!\n");
    ::exit(EXIT_FAILURE);
  }

  rtmp = RTMP_Alloc();
  if(!rtmp) {
    STREAMER_ERROR("error: cannot allocate the rtmp context.\n");
    ::exit(EXIT_FAILURE);
  }

  RTMP_Init(rtmp);

  if(!RTMP_SetupURL(rtmp, (char*)settings.url.c_str())) {
    STREAMER_ERROR("error: cannot setup the url for the RTMP Writer.\n");
    RTMP_Free(rtmp);
    rtmp = NULL;
    return false;
  }

  if(settings.username.size()) {
    rtmp->Link.pubUser.av_val = (char*)settings.username.c_str();
    rtmp->Link.pubUser.av_len = settings.username.size();
  }

  if(settings.password.size()) {
    rtmp->Link.pubPasswd.av_val = (char*)settings.password.c_str();
    rtmp->Link.pubPasswd.av_len = settings.password.size();
  }

  rtmp->Link.flashVer.av_val = (char*)"FMLE/3.0 (compatible; FMSc/1.0)"; // when streaming to a FMS you need this!
  rtmp->Link.flashVer.av_len = (int)strlen(rtmp->Link.flashVer.av_val);

  RTMP_EnableWrite(rtmp);

  if(!RTMP_Connect(rtmp, NULL)) {
    STREAMER_ERROR("error: cannot connect to the rtmp server: %s\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;
    return false;
  }

  if(!RTMP_ConnectStream(rtmp, 0)) {
    STREAMER_ERROR("error: cannot connect to the rtmp stream on %s.\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;

    if(state == RW_STATE_RECONNECTING) {
      state = RW_STATE_NONE;
    }

    return false;
  }

  state = RW_STATE_INITIALIZED;
  packet=new RTMPPacket();//创建包
  RTMPPacket_Alloc(packet,1024*64);//给packet分配数据空间
  RTMPPacket_Reset(packet);//重置packet状态

  return true;
}

void RTMPWriter::write(uint8_t* data, size_t nbytes) {
#if 0
printf("RTMP state is %d\n",state);
#endif

  if(state == RW_STATE_NONE) {
    STREAMER_ERROR("error: cannot write to rtmp server because we haven't been initialized. did you call initialize()?\n");
    return;
  }
  else if(state == RW_STATE_RECONNECTING) {
    //printf("reconnecting... ignoring data...\n");
    return;
  }
  else if(state == RW_STATE_DISCONNECTED) {
    // the caller needs to call reconnect() 
    return;
  }
//	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
//	packet->m_nTimeStamp = time;
//	packet->m_packetType=type;
//	packet->m_nBodySize=nbytes;
  //printf("rtmp: %ld\n", nbytes);
  //if (!RTMP_SendPacket(rtmp,packet,0))

 // int r = RTMP_Write(rtmp, (const char*)data, (int)nbytes);
//  usleep(10000);


  //modify by wxb -> every 40ms send once.
  uint32_t start_time = 0, stop_time = 0;
  start_time = uint32_t(uv_hrtime() / 1000);
  int ret=RTMPwritepack(rtmp, (const char*)data, (int)nbytes);
  stop_time = uint32_t(uv_hrtime() / 1000);

//  uint32_t pass_time = stop_time - start_time;
//  if(pass_time < 20 * 1000){
//      usleep(20*1000 - pass_time);
//  }



#if 0
  printf("%d,%d,%d\n",data[0],data[1],data[2]);

  printf("RTMP_Write %d bytes\n",ret);
#endif

  if(ret < 0) {

    // @todo - we should close and cleanup here!!!!
      if(rtmp){
          RTMP_Close(rtmp);
          RTMP_Free(rtmp);
      }
    rtmp = NULL;

    if(cb_disconnect) {
      cb_disconnect(this, cb_user);
    }

    STREAMER_ERROR("error: something went wrong while trying to write data to the rtmp server.\n");
    if(state == RW_STATE_DISCONNECTED) {
      return;
    }
    // when initialized and we arrive here, it means we're disconnected
    else if(state == RW_STATE_INITIALIZED) {

      state = RW_STATE_DISCONNECTED;
      if(cb_disconnect) {
        cb_disconnect(this, cb_user);
      }

    }
  }
}
int RTMPWriter::RTMPwritepack(RTMP *r, const char *buf, int size) {
	RTMPPacket *pkt = &r->m_write;
	char *pend, *enc;
	int s2 = size, ret, num;

	pkt->m_nChannel = 0x04; /* source channel */
	pkt->m_nInfoField2 = r->m_stream_id;

	while (s2) {
		if (!pkt->m_nBytesRead) {
			if (size < 11) {
				/* FLV pkt too small */
				return 0;
			}

			if (buf[0] == 'F' && buf[1] == 'L' && buf[2] == 'V') {
				buf += 13;
				s2 -= 13;
			}

			pkt->m_packetType = *buf++;
//			printf("pkt->m_packetType is %d**\n",pkt->m_packetType);
			pkt->m_nBodySize = AMF_DecodeInt24(buf);
			buf += 3;
			pkt->m_nTimeStamp = AMF_DecodeInt24(buf);
			buf += 3;
			pkt->m_nTimeStamp |= *buf++ << 24;
			buf += 3;
			s2 -= 11;

			if (((pkt->m_packetType == 0x08 || pkt->m_packetType == 0x09)
					&& !pkt->m_nTimeStamp) || pkt->m_packetType == 0x12) {

				pkt->m_headerType = RTMP_PACKET_SIZE_MEDIUM;//zly  2014-06-05
				pkt->m_headerType = RTMP_PACKET_SIZE_LARGE;//zly
				if (pkt->m_packetType == 0x12)
					pkt->m_nBodySize += 16;
			} else {
				pkt->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
				pkt->m_headerType = RTMP_PACKET_SIZE_LARGE;//zly
			}
//			printf("pkt->m_nBodySize is %d**\n",pkt->m_nBodySize);
			if (!RTMPPacket_Alloc(pkt, pkt->m_nBodySize)) {
				RTMP_Log(RTMP_LOGDEBUG, "%s, failed to allocate packet",
						__FUNCTION__);
				return FALSE;
			}
			enc = pkt->m_body;
			pend = enc + pkt->m_nBodySize;
			if (pkt->m_packetType == 0x12) {
				enc = AMF_EncodeString(enc, pend, &av_setDataFramezly);
				pkt->m_nBytesRead = enc - pkt->m_body;
			}
		} else {
			enc = pkt->m_body + pkt->m_nBytesRead;
		}
#if 0
		printf("pkt->m_nBodySize is %d**\n",pkt->m_nBodySize);
#endif
		num = pkt->m_nBodySize - pkt->m_nBytesRead;
		if (num > s2)
			num = s2;
		memcpy(enc, buf, num);
		pkt->m_nBytesRead += num;
		s2 -= num;
		buf += num;
		if (pkt->m_nBytesRead == pkt->m_nBodySize) {
#if 0
			printf("RTMP_SendPacket****III**** \n");
#endif
			ret = RTMP_SendPacket(r, pkt, FALSE);
			RTMPPacket_Free(pkt);
			pkt->m_nBytesRead = 0;
			if (!ret)
				return -1;
			buf += 4;
			s2 -= 4;
			if (s2 < 0)
				break;
		}
	}
	return size + s2;
}



void RTMPWriter::reconnect() {

  if(state == RW_STATE_RECONNECTING) {
    STREAMER_WARNING("warning: already reconnecting ....");
    return;
  }

  close();

  state = RW_STATE_RECONNECTING;

  initialize();
}

void RTMPWriter::close() {

  if(rtmp) {
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    rtmp = NULL;
  }

}

// @todo - read() is blocking, we would need to handle handle the socket ourself
void RTMPWriter::read() {

  if(state != RW_STATE_INITIALIZED) {
    STREAMER_ERROR("error: cannot read because we're not initialized.\n");
    return;
  }

  char buf[512];
  int r = RTMP_Read(rtmp, buf, sizeof(buf));
  STREAMER_VERBOSE("read: >> %d << \n", r);

}
