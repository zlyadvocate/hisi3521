
extern "C" {
#  include <uv.h>
}

#include <assert.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/Debug.h>
#include <streamer/core/Log.h>


#define FF_I_TYPE  1 ///< Intra
#define FF_P_TYPE  2 ///< Predicted
#define FF_B_TYPE  3 ///< Bi-dir predicted
#define FF_S_TYPE  4 ///< S(GMC)-VOP MPEG4
#define FF_SI_TYPE 5 ///< Switching Intra
#define FF_SP_TYPE 6 ///< Switching Predicted
#define FF_BI_TYPE 7



//cif sps pps defination


extern char g_picsize;

// --------------------------------------------------

// --------------------------------------------------


VideoEncoder::VideoEncoder() 
  :vflip(true)
  ,frame_num(0)
  ,stream_id(-1)
#if VIDEO_ENCODER_MEASURE_BITRATE
  ,kbps_timeout(0)
  ,kbps_delay(1000 * 1000 * 1000)
  ,kbps_nbytes(0)
  ,kbps_time_started(0)
  ,kbps(0.0)
#endif
{
	   nals_count = 0;
}

VideoEncoder::~VideoEncoder() {

  shutdown();

}

// @todo check if width / height are valid (multiples of 16 I believe)
bool VideoEncoder::setup(VideoSettings s) {
  assert(s.width > 0);
  assert(s.height > 0);
  assert(s.fps > 0);

  settings = s;

  return true;
}

bool VideoEncoder::initialize() {
  assert(settings.width);
  assert(settings.height);

  frame_num = 0;

//  if(!initializeX264()) {
//    return false;
//  }
//
//  if(!initializePic()) {
//    return false;
//  }

  return true;
}

//// See https://gist.github.com/roxlu/0f61a499df75e64b764d for an older version of this, with some rate control tests
//// @todo - we should check if the supplied settings are valid for the current profile.. e.g. bframes are not supported by the baseline profile
//bool VideoEncoder::initializeX264() {
//  assert(settings.width > 0);
//  assert(settings.height > 0);
//  assert(settings.fps > 0);
//
//  int r = 0;
//  x264_param_default(&params);//zly
//  x264_param_t* p = &params;
//
//  std::string preset = (settings.preset.size()) ? settings.preset : "superfast";
//  std::string tune = (settings.tune.size()) ? settings.tune : "zerolatency";
//  STREAMER_STATUS("x264 using preset: %s and tune: %s\n", preset.c_str(), tune.c_str());
//
//  r = x264_param_default_preset(p, preset.c_str(), tune.c_str());
//  if(r != 0) {
//    STREAMER_ERROR("error: cannot set the default preset on x264.\n");
//    return false;
//  }
//
//
//  p->i_threads = settings.threads;
//  p->i_width = settings.width;
//  p->i_height = settings.height;
//  p->i_fps_num = settings.fps;
//  p->i_fps_den = 1;
//  p->b_annexb = 0; // flv == no annexb, but strangely, when I disable it the generated flv cannot be played back, for raw h264 you'll need to set annexb to 1 when you want to play it in vlc (vlc isn't properly playing back flv)
//
//  p->rc.i_rc_method = X264_RC_ABR;  // when you're limited to bandwidth you set the vbv_buffer_size and vbv_max_bitrate using the X264_RC_ABR rate control method. The vbv_buffer_size is a decoder option and tells the decoder how much data must be buffered before playback can start. When vbv_max_bitrate == vbv_buffer_size, then it will take one second before the playback might start. when vbv_buffer_size == vbv_max_bitrate * 0.5, it might start in 0.5 sec.
//  p->rc.i_bitrate = settings.bitrate;
//  p->rc.i_vbv_buffer_size = (settings.vbv_buffer_size < 0) ? p->rc.i_bitrate : settings.vbv_buffer_size;
//  p->rc.i_vbv_max_bitrate = (settings.vbv_max_bitrate < 0) ? p->rc.i_bitrate : settings.vbv_max_bitrate;;
//
//  if(settings.keyint_max > 0) {
//    p->i_keyint_max = settings.keyint_max;
//  }
//
//  if(settings.bframe > 0) {
//    p->i_bframe = settings.bframe;
//  }
//
//  if(settings.level_idc > 0) {
//    p->i_level_idc = settings.level_idc;
//  }
//
//#if !defined(NDEBUG)
////  p->i_log_level = X264_LOG_DEBUG;
////  p->pf_log = videoencoder_x264_log;
//#else
//  p->i_log_level = X264_LOG_DEBUG;
//#endif
//
//
////  print_x264_params(p);
//
//  return true;
//}
//
//bool VideoEncoder::initializePic() {
//
////  unsigned int csp = (vflip) ? X264_CSP_I420 | X264_CSP_VFLIP : X264_CSP_I420;
//////  x264_picture_init(&pic_in);
////  pic_in.img.i_csp = csp;
////  pic_in.img.i_plane = 3;
//// // x264_picture_init(&pic_in);//初始化图片信息zly
//////  x264_picture_init(&pic_in);
////  x264_picture_alloc(&pic_in,X264_CSP_I420,settings.width,settings.height);
//
//  return true;
//}

bool VideoEncoder::encodePacket(AVPacket* p, FLVTag& tag) {
  assert(p);
//  assert(encoder);
//  pic_in.i_pts = frame_num;
  frame_num++;

//#if defined(USE_GRAPH)
//  uint64_t enc_start = uv_hrtime() / 1000000;
//  printf("frame_num is %d\n",frame_num);
//#endif

  int frame_size;
  if(p!=NULL)
  {
	  //frame_size = x264_encoder_encode(encoder, &nal, &nals_count, &pic_in, &pic_out);
	  frame_size=p->videosize;
  }else
	  return true;
#if 0
  printf("frame_size is **9**%d\n",frame_size);
#endif


  if(frame_size < 0) {
    STREAMER_ERROR("error: x264_encoder_encode failed.\n");
    return false;
  }

#define ENCODE_PACKET_USE_PTR 1
#define ENCODE_PACKET_USE_COPY 0
#define ENCODE_PACKE_USE_COPY_ALL 0

#if ENCODE_PACKE_USE_COPY_ALL 
  tag.bs.clear();
  for(int i = 0; i < nals_count; ++i) {
    tag.bs.putBytes(nal[i].p_payload, nal[i].i_payload);
  }
  tag.setData(tag.bs.getPtr(), tag.bs.size());
#endif

#if ENCODE_PACKET_USE_COPY
  tag.bs.clear();
//  tag.bs.putBytes(nal[0].p_payload, frame_size);
  tag.bs.putBytes(reinterpret_cast<unsigned char*> (&p->data[0]), frame_size);//
  tag.setData(tag.bs.getPtr(), tag.bs.size());

#endif

#if ENCODE_PACKET_USE_PTR
  tag.setData(reinterpret_cast<unsigned char*> (&p->data[4]), frame_size-4);
#endif

#if 0
  printf("frame naltype,size is %d %d \n",p->nal_type,frame_size);
#endif
//  (dwTemp >> 8) & 0xFF
  tag.setTimeStamp(p->timestamp,(unsigned char)(p->timestamp>>24)&0x7F);
  //tag.setCompositionTime(offset);
  tag.setAVCPacketType(FLV_AVC_NALU);//zly for 06 sequence header
  //set for SPS PPS

  if((p->nal_type==0x05))
  {
	  tag.setFrameType(FLV_VIDEOFRAME_KEY_FRAME);
  }
  else
  {
	  tag.setFrameType(FLV_VIDEOFRAME_INTER_FRAME);
  }

  // debuging buffer issue, when a player has issues with loading/filling a buffer this 
  // might be caused because of the keyframe interval is to high. This piece of code
  // is used to determine how much time between each keyframe exists.

#if VIDEO_ENCODER_MEASURE_BITRATE
  uint64_t now = uv_hrtime();
  if(!kbps_time_started) {
    kbps_time_started = now;
  }

  kbps_nbytes += frame_size;

  if(now >= kbps_timeout) {
    double timediff = double(now - kbps_time_started) / (1000 * 1000 * 1000); // in sec
    kbps = ((kbps_nbytes*8) / timediff) / 1000.0;
    STREAMER_STATUS("-- x264 kbps: %02.2f\n", kbps);
    kbps_timeout = now + kbps_delay;
  }
#endif

  return true;
}

bool VideoEncoder::createDecoderConfigurationRecord(AVCDecoderConfigurationRecord& rec) {
//  assert(encoder);

   int sps_size, pps_size, sei_size;
   char HI3521SPS[]={0x67,0x42,0x00,0x14,0x95,0xA8,0x58,0x25,0x90};
   char HI3521PPS[]={0x68,0xce,0x3c,0x80};
   char HI3521SEI[]={0x06,0xe5,0x01,0xbb,0x80};

   //qcif
   char HI3521SPS_QCIF[]={0x67,0x42,0x00,0x0A,0x95,0xA8,0xB1,0x39};
   char HI3521PPS_QCIF[]={0x68,0xCE,0x3C,0x80};
   char HI3521SEI_QCIF[]={0x06,0xe5,0x01,0xbb,0x80};

   //qcif MP
   char HI3521SPS_MPQCIF[]={0x67,0x4D,0x00,0x0A,0x95,0xA8,0xB1,0x39};
   char HI3521PPS_MPQCIF[]={0x68,0xEE,0x3C,0x80};
   char HI3521SEI_MPQCIF[]={0x06,0xe5,0x01,0x20,0x80};
   //320*240 MP
   char HI3521SPS_MPQVGA[]={0x67,0x4D,0x00,0x14,0x95,0xA8,0x50,0x7E,0x40};
   char HI3521PPS_MPQVGA[]={0x68,0xEE,0x3C,0x80};
   char HI3521SEI_MPQVGA[]={0x06,0xe5,0x01,0x35,0x80};
    uint8_t* sps;
    uint8_t* pps;
    uint8_t* sei;

    if(g_picsize == 0) //PIC_QCIF
    {
//        sps_size = sizeof(HI3521SPS_QCIF);
//        pps_size = sizeof(HI3521PPS_QCIF);
//        sei_size = sizeof(HI3521SEI_QCIF);
//        sps = (uint8_t*)(HI3521SPS_QCIF);
//        pps = (uint8_t*)(HI3521PPS_QCIF );
//        sei =(uint8_t*) (HI3521SEI_QCIF );
//         sps_size = sizeof(HI3521SPS_MPQCIF);
//         pps_size = sizeof(HI3521PPS_MPQCIF);
//         sei_size = sizeof(HI3521SEI_MPQCIF);
//         sps = (uint8_t*)(HI3521SPS_MPQCIF);
//         pps = (uint8_t*)(HI3521PPS_MPQCIF );
//         sei =(uint8_t*) (HI3521SEI_MPQCIF );
            sps_size = sizeof(HI3521SPS_MPQVGA);
            pps_size = sizeof(HI3521PPS_MPQVGA);
            sei_size = sizeof(HI3521SEI_MPQVGA);
            sps = (uint8_t*)(HI3521SPS_MPQVGA);
            pps = (uint8_t*)(HI3521PPS_MPQVGA );
            sei =(uint8_t*) (HI3521SEI_MPQVGA );
    }
    else    // as PIC_CIF
    {
        sps_size = sizeof(HI3521SPS);
        pps_size = sizeof(HI3521PPS);
        sei_size = sizeof(HI3521SEI);
        sps = (uint8_t*)(HI3521SPS );
        pps = (uint8_t*)(HI3521PPS );
        sei =(uint8_t*) (HI3521SEI );
    }


  rec.configuration_version = 1;
  rec.avc_profile_indication = sps[1];
  rec.profile_compatibility = sps[2];
  rec.avc_level_indication = sps[3];

  std::copy(sps, sps+(sps_size), std::back_inserter(rec.sps));
  std::copy(pps, pps+(pps_size), std::back_inserter(rec.pps));
  std::copy(sei, sei+(sei_size), std::back_inserter(rec.sei));

  return true;
} 

bool VideoEncoder::openFile(std::string filepath) {

  if(ofs.is_open()) {
    STREAMER_ERROR("error: cannot open the video encoder output file becuase it's already open.\n");
    return false;
  }
  
  ofs.open(filepath.c_str(), std::ios::binary | std::ios::out);
  if(!ofs.is_open()) {
    STREAMER_ERROR("error: cannot open the video encoder output file: %s\n", filepath.c_str());
    return false;
  }
  
  return true;
}

bool VideoEncoder::writeTagToFile(FLVTag& tag) {

  if(!ofs.is_open()) {
    STREAMER_ERROR("error: cannot write the video tag to the encoder output file because the file hasn't been opened yet.\n");
    return false;
  }
  
  ofs.write((char*)tag.data, tag.size);

  return true;
}

bool VideoEncoder::closeFile() {

  if(!ofs.is_open()) {
    STREAMER_ERROR("error: cannot close the encoder file because it hasn't been opened yet.\n");
    return false;
  }

  ofs.close();

  return true;
}

bool VideoEncoder::shutdown() {

  if(ofs.is_open()) {
    closeFile();
  }

  STREAMER_VERBOSE("Shutting down the video encoder.\n");

  frame_num = 0;

  return true;
}

