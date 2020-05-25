/******************************************************************************
 Some simple Hisilicon Hi3531 video encode functions.

 Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
 Modification:  2011-2 Created
 ******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>




#include "sample_comm.h"
#include "avstream.h"
#include "videobuf.h"
#include "hi_faac.h"


#include "HI264RTPPACK.h"

#include "zini.h"

#define BITDEBUG 0
#define STREAMDEBUG 0

#define VIDEOSHARED      1
#define AUDIOCOMBINED    1
#define AUDIOSTREAMING   1

#define MAX_BUFF_SIZE			524288
#define MAXSUB_BUFF_SIZE			32*1024


#if VIDEOSHARED
extern pthread_mutex_t streammutex;
HI_BOOL ChannelStreammark[]={HI_FALSE,HI_FALSE,HI_FALSE,HI_FALSE};
extern HI_BOOL audiostreamingflag;
//for audio recording

char savepath[] = "/mnt/hd1/chan/";
char name[128] = { 0 };
char timestr[4][128] = { 0 };
char oldtimestr[4][128] = { 0 };

extern char mdn[32];
extern videobuffer m_videobuffer;
extern int minisStd;
extern int recordfrmcnt;

#endif



const HI_U8 g_SOI[2] = { 0xFF, 0xD8 };
const HI_U8 g_EOI[2] = { 0xFF, 0xD9 };
static pthread_t gs_VencPid[8];
static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;
SAMPLE_VENC_GETSTREAM_PARA_S substream_stPara[8];
static HI_S32 gs_s32SnapCnt = 0;
FILE *pFile[VENC_MAX_CHN_NUM];
FILE *mp3File = NULL;
HI_S32 streamid = 255;
HI_U8 SPSbuff[100];

unsigned char g_ttl = 0;  // add by wxb 20140610
extern int bitratestream;         // add by wxb 20140618
extern int framerate;

typedef struct s_videopackinfo{
	int naltype;
	int length;
	int u32Seq;
	int u32PackCount;
	short crc;
	short resv;
	HI_U64   u64PTS;                /*PTS*/

}videopackinfo;
extern HI_BOOL audiostreamingflag;

/******************************************************************************
 * function : Set venc memory location
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_MemConfig(HI_VOID) {
	HI_S32 i = 0;
	HI_S32 s32Ret;

	HI_CHAR * pcMmzName;
	MPP_CHN_S stMppChnVENC;
	MPP_CHN_S stMppChnGRP;

	/* group, venc max chn is 64*/
	for (i = 0; i < 64; i++) {
		stMppChnGRP.enModId = HI_ID_GROUP;
		stMppChnGRP.s32DevId = i;
		stMppChnGRP.s32ChnId = 0;

		stMppChnVENC.enModId = HI_ID_VENC;
		stMppChnVENC.s32DevId = 0;
		stMppChnVENC.s32ChnId = i;

		if (0 == (i % 2)) {
			pcMmzName = NULL;
		} else {
			pcMmzName = "ddr1";
		}

		/*grp*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnGRP, pcMmzName);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_SYS_SetMemConf failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}

		/*venc*/
		s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVENC, pcMmzName);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_SYS_SetMemConf with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

/******************************************************************************
 * function : venc bind vpss
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_GRP GrpChn, VPSS_GRP VpssGrp,
		VPSS_CHN VpssChn) {
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = HI_ID_GROUP;
	stDestChn.s32DevId = GrpChn;
	stDestChn.s32ChnId = 0;

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

/******************************************************************************
 * function : venc unbind vpss
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_GRP GrpChn, VPSS_GRP VpssGrp,
		VPSS_CHN VpssChn) {
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;

	stDestChn.enModId = HI_ID_GROUP;
	stDestChn.s32DevId = GrpChn;
	stDestChn.s32ChnId = 0;

	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS) {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

/******************************************************************************
 * funciton : get file postfix according palyload_type.
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload,
		char *szFilePostfix) {
	if (PT_H264 == enPayload) {
		strcpy(szFilePostfix, ".h264");
	} else if (PT_JPEG == enPayload) {
		strcpy(szFilePostfix, ".jpg");
	} else if (PT_MJPEG == enPayload) {
		strcpy(szFilePostfix, ".mjp");
	} else if (PT_MP4VIDEO == enPayload) {
		strcpy(szFilePostfix, ".mp4");
	} else {
		SAMPLE_PRT("payload type err!\n");
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : save mjpeg stream.
 * WARNING: in Hi3531, user needn't write SOI & EOI.
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveMJpeg(FILE* fpJpegFile, VENC_STREAM_S *pstStream) {
	VENC_PACK_S* pstData;
	HI_U32 i;

	//fwrite(g_SOI, 1, sizeof(g_SOI), fpJpegFile); //in Hi3531, user needn't write SOI!

	for (i = 0; i < pstStream->u32PackCount; i++) {
		pstData = &pstStream->pstPack[i];
		fwrite(pstData->pu8Addr[0], pstData->u32Len[0], 1, fpJpegFile);
		fwrite(pstData->pu8Addr[1], pstData->u32Len[1], 1, fpJpegFile);
	}

	//fwrite(g_EOI, 1, sizeof(g_EOI), fpJpegFile);//in Hi3531, user needn't write SOI!

	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : save H264 stream
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveH264(FILE* fpH264File, VENC_STREAM_S *pstStream) {
	HI_S32 i;
    int offset = 0;

	for (i = 0; i < pstStream->u32PackCount; i++) {
        offset += pstStream->pstPack[i].u32Len[0];
        if(offset > MAX_BUFF_SIZE){
            SAMPLE_PRT("save stream failed!\n");
            return HI_FAILURE;
        }

		fwrite(pstStream->pstPack[i].pu8Addr[0],
				pstStream->pstPack[i].u32Len[0], 1, fpH264File);





		fflush(fpH264File);

		if (pstStream->pstPack[i].u32Len[1] > 0) {
            offset += pstStream->pstPack[i].u32Len[1];
            if(offset > MAX_BUFF_SIZE){
                SAMPLE_PRT("save stream failed!\n");
                return HI_FAILURE;
            }

			fwrite(pstStream->pstPack[i].pu8Addr[1],
					pstStream->pstPack[i].u32Len[1], 1, fpH264File);

			fflush(fpH264File);
		}
	}

	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : save jpeg stream
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveJPEG(FILE *fpJpegFile, VENC_STREAM_S *pstStream) {
	VENC_PACK_S* pstData;
	HI_U32 i;

	for (i = 0; i < pstStream->u32PackCount; i++) {
		pstData = &pstStream->pstPack[i];
		fwrite(pstData->pu8Addr[0], pstData->u32Len[0], 1, fpJpegFile);
		fwrite(pstData->pu8Addr[1], pstData->u32Len[1], 1, fpJpegFile);
	}

	return HI_SUCCESS;
}
/******************************************************************************
 * funciton : save snap stream
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveSnap(VENC_STREAM_S *pstStream, VPSS_GRP VpssGrp) {
	char acFile[128] = { 0 };
	FILE *pFile;
	HI_S32 s32Ret;
    printf("save snap jpg\n");

    struct tm *pstTimeNow = NULL;
    time_t TimeNow;
    TimeNow = time(&TimeNow);
    pstTimeNow = localtime(&TimeNow);
    sprintf(acFile, "snapshot/snap%d_%s_%04d%02d%02d%02d%02d%02d.jpg",
            VpssGrp, mdn,
            pstTimeNow->tm_year + 1900,
            pstTimeNow->tm_mon + 1,
            pstTimeNow->tm_mday,
            pstTimeNow->tm_hour % 24,
            pstTimeNow->tm_min,
            pstTimeNow->tm_sec);
	pFile = fopen(acFile, "wb");
	if (pFile == NULL) {
		SAMPLE_PRT("open file err\n");
		return HI_FAILURE;
	}
	s32Ret = SAMPLE_COMM_VENC_SaveJPEG(pFile, pstStream);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("save snap picture failed!\n");
		return HI_FAILURE;
	}
	fclose(pFile);
	gs_s32SnapCnt++;
	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : save stream
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType, FILE *pFd,
		VENC_STREAM_S *pstStream) {
	HI_S32 s32Ret;

	if (PT_H264 == enType) {
		s32Ret = SAMPLE_COMM_VENC_SaveH264(pFd, pstStream);
	} else if (PT_MJPEG == enType) {
		s32Ret = SAMPLE_COMM_VENC_SaveMJpeg(pFd, pstStream);
	} else {
		return HI_FAILURE;
	}
	return s32Ret;
}
HI_S32 SUBSTREAM_VENC_Start(VENC_GRP VencGrp, VENC_CHN VencChn,
		PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize,
		SAMPLE_RC_E enRcMode)
{
	HI_S32 s32Ret;
		VENC_CHN_ATTR_S stVencChnAttr;
		VENC_ATTR_H264_S stH264Attr;
		VENC_ATTR_H264_CBR_S stH264Cbr;
		VENC_ATTR_H264_VBR_S stH264Vbr;
		VENC_ATTR_H264_FIXQP_S stH264FixQp;
		VENC_ATTR_MJPEG_S stMjpegAttr;
		VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
		VENC_ATTR_JPEG_S stJpegAttr;
		SIZE_S stPicSize;

		s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enSize, &stPicSize);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("Get picture size failed!\n");
			return HI_FAILURE;
		}
		/******************************************
		 step 1: Greate Venc Group
		 ******************************************/
		s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n", VencGrp,
					s32Ret);
			return HI_FAILURE;
		}

		/******************************************
		 step 2:  Create Venc Channel
		 ******************************************/
		stVencChnAttr.stVeAttr.enType = enType;
		switch (enType) {
		case PT_H264: {
			stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
			stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
			stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
			stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
			stH264Attr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/

            stH264Attr.u32Profile = 1;/*0: baseline; 1:MP; 2:HP   ? zly*/
			stH264Attr.bByFrame = HI_FALSE;/*get stream mode is slice mode or frame mode?*/
			//stH264Attr.bByFrame = HI_FALSE;/*get stream mode is slice mode or frame mode?*/
			stH264Attr.bField = HI_FALSE; /* surpport frame code only for hi3516, bfield = HI_FALSE */
			stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
			stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
			stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr,
					sizeof(VENC_ATTR_H264_S));

			if (SAMPLE_RC_CBR == enRcMode) {
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
				stH264Cbr.u32Gop = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
                stH264Cbr.u32Gop = 15;
                stH264Cbr.u32Gop = 15;//QVGA
				stH264Cbr.u32StatTime = 1; /* stream rate statics time(s) */
				stH264Cbr.u32ViFrmRate =
						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;/* input (vi) frame rate */
                stH264Cbr.fr32TargetFrmRate = framerate;
//						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;/* target frame rate zly*/
				switch (enSize) {
				case PIC_QCIF:
                    stH264Cbr.u32BitRate = bitratestream; /* average bit rate */   //wxb 20140620


					break;
				case PIC_QVGA: /* 320 * 240 */
				case PIC_CIF:
                    stH264Cbr.u32BitRate = bitratestream;//bitrate, add by wxb 20140618
                     if(VencChn == 5)
                         stH264Cbr.u32BitRate = bitratestream+20;
					break;

				case PIC_D1:
				case PIC_VGA: /* 640 * 480 */
					stH264Cbr.u32BitRate = 1024 * 2;
					stH264Cbr.u32BitRate = 1280;
					break;
				case PIC_HD720: /* 1280 * 720 */
					stH264Cbr.u32BitRate = 1024 * 3;
					break;
				case PIC_HD1080: /* 1920 * 1080 */
					stH264Cbr.u32BitRate = 1024 * 6;
					break;
				default:
					stH264Cbr.u32BitRate = 1024 * 4;
					break;
				}

				stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr,
						sizeof(VENC_ATTR_H264_CBR_S));
			} else if (SAMPLE_RC_FIXQP == enRcMode) {
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
				stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264FixQp.u32ViFrmRate =
						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264FixQp.fr32TargetFrmRate =
						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264FixQp.u32IQp = 20;
				stH264FixQp.u32PQp = 23;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,
						sizeof(VENC_ATTR_H264_FIXQP_S));
			} else if (SAMPLE_RC_VBR == enRcMode) {
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
				stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264Vbr.u32StatTime = 1;
				stH264Vbr.u32ViFrmRate =
						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264Vbr.fr32TargetFrmRate =
						(VIDEO_ENCODING_MODE_PAL == enNorm) ? 25 : 30;
				stH264Vbr.u32MinQp = 10;
				stH264Vbr.u32MaxQp = 40;
				switch (enSize) {
				case PIC_QCIF:
					stH264Vbr.u32MaxBitRate = 256 * 3; /* average bit rate */
					break;
				case PIC_QVGA: /* 320 * 240 */
				case PIC_CIF:
					stH264Vbr.u32MaxBitRate = 512 * 3;
					break;
				case PIC_D1:
				case PIC_VGA: /* 640 * 480 */
					stH264Vbr.u32MaxBitRate = 1024 * 2;
					break;
				case PIC_HD720: /* 1280 * 720 */
					stH264Vbr.u32MaxBitRate = 1024 * 3;
					break;
				case PIC_HD1080: /* 1920 * 1080 */
					stH264Vbr.u32MaxBitRate = 1024 * 6;
					break;
				default:
					stH264Vbr.u32MaxBitRate = 1024 * 4 * 3;
					break;
				}
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr,
						sizeof(VENC_ATTR_H264_VBR_S));
			} else {
				return HI_FAILURE;
			}
		}
			break;
		default:
			return HI_ERR_VENC_NOT_SUPPORT;
		}

		s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n", VencChn,
					s32Ret);
			return s32Ret;
		}

		/******************************************
		 step 3:  Regist Venc Channel to VencGrp
		 ******************************************/
		s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
			return HI_FAILURE;
		}

		/******************************************
		 step 4:  Start Recv Venc Pictures
		 ******************************************/
		s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
			return HI_FAILURE;
		}

		return HI_SUCCESS;
}
/******************************************************************************
 * funciton : Start venc stream mode (h264, mjpeg)
 * note      : rate control parameter need adjust, according your case.
 ******************************************************************************/

/******************************************************************************
* funciton : Start venc stream mode (h264, mjpeg)
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Start(VENC_GRP VencGrp,VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
    VENC_ATTR_JPEG_S stJpegAttr;
    SIZE_S stPicSize;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enSize, &stPicSize);
     if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get picture size failed!\n");
        return HI_FAILURE;
    }
    /******************************************
     step 1: Greate Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",\
                 VencGrp, s32Ret);
        return HI_FAILURE;
    }
printf("mydebug:SAMPLE_COMM_VENC_Start->HI_MPI_VENC_CreateGroup %d\n",VencGrp);
    /******************************************
     step 2:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = enType;
    switch(enType)
    {
        case PT_H264:
        {
            stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
            stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
            stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
            stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
            stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile  = 1;/*0: baseline; 1:MP; 2:HP   ? */
            stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
            stH264Attr.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
            stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
            stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
            stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
                stH264Cbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* input (vi) frame rate */
                stH264Cbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stH264Cbr.u32BitRate = 256; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                       if(minisStd){
                           stH264Cbr.u32BitRate = 256;//wxb,部标码率为256，可减小视频文件尺寸
                       }
                       else{
                           stH264Cbr.u32BitRate = 512;//wxb

                           char secstr[32] = {0};
                           readString("basesetting", "cityConfig", "config0", "/lib/clientcity.ini", secstr, sizeof(secstr));
                           //find config
                           char tmpstr[32] = {0};
                           readString(secstr, "mainStreamBitRate", "0", "/lib/clientcommon.ini", tmpstr, sizeof(tmpstr));
                           int tmprate = atoi(tmpstr);
                           if(tmprate > 0){
                               stH264Cbr.u32BitRate = tmprate;//wxb
                               printf("clientcommint config u32BitRate=%d\n", tmprate);
                           }
                       }
                       break;

                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                       stH264Cbr.u32BitRate = 500;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stH264Cbr.u32BitRate = 1024*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stH264Cbr.u32BitRate = 1024*6;
                	   break;
                  default :
                       stH264Cbr.u32BitRate = 1024*4;
                       break;
                }

                stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32MinQp = 10;
                stH264Vbr.u32MaxQp = 40;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stH264Vbr.u32MaxBitRate= 256*3; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stH264Vbr.u32MaxBitRate = 512*3;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stH264Vbr.u32MaxBitRate = 1024*2;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stH264Vbr.u32MaxBitRate = 1024*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stH264Vbr.u32MaxBitRate = 1024*6;
                	   break;
                  default :
                       stH264Vbr.u32MaxBitRate = 1024*4*3;
                       break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;

        case PT_MJPEG:
        {
            stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
            stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
            stMjpegAttr.u32PicWidth = stPicSize.u32Width;
            stMjpegAttr.u32PicHeight = stPicSize.u32Height;
            stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
            stMjpegAttr.bMainStream = HI_TRUE;  /*main stream or minor stream types?*/
            stMjpegAttr.bVIField = HI_FALSE;  /*the sign of the VI picture is field or frame?*/
            stMjpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

            if(SAMPLE_RC_FIXQP == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 90;
                stMjpegeFixQp.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stMjpegeFixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
                       sizeof(VENC_ATTR_MJPEG_FIXQP_S));
            }
            else if (SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 384*3; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 768*3;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3*3;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*5*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
                	   break;
                  default :
                       stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*7*3;
                       break;
                }
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32TargetFrmRate = 5;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate= 256*3; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 512*3;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2*3;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*6*3;
                	   break;
                  default :
                       stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*4*3;
                       break;
                }
            }
            else
            {
                SAMPLE_PRT("cann't support other mode in this version!\n");

                return HI_FAILURE;
            }
        }
        break;

        case PT_JPEG:
            stJpegAttr.u32PicWidth  = stPicSize.u32Width;
            stJpegAttr.u32PicHeight = stPicSize.u32Height;
            stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
            stJpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));
            break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
    printf("mydebug:SAMPLE_COMM_VENC_Start->HI_MPI_VENC_CreateChn %d\n",VencChn);

    /******************************************
     step 3:  Regist Venc Channel to VencGrp
    ******************************************/
    s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    printf("mydebug:SAMPLE_COMM_VENC_Start->HI_MPI_VENC_RegisterChn grp=%d,chn=%d\n",VencGrp,VencChn);

    /******************************************
     step 4:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}
/******************************************************************************
 * funciton : Stop venc ( stream mode -- H264, MJPEG )
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_Stop(VENC_GRP VencGrp, VENC_CHN VencChn) {
	HI_S32 s32Ret;

	/******************************************
	 step 1:  Stop Recv Pictures
	 ******************************************/
	s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return HI_FAILURE;
	}

	/******************************************
	 step 2:  UnRegist Venc Channel
	 ******************************************/
	s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return HI_FAILURE;
	}

	/******************************************
	 step 3:  Distroy Venc Channel
	 ******************************************/
	s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return HI_FAILURE;
	}

	/******************************************
	 step 4:  Distroy Venc Group
	 ******************************************/
	s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",
				VencGrp, s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : Start snap
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapStart(VENC_GRP VencGrp, VENC_CHN VencChn,
		SIZE_S *pstSize) {
	HI_S32 s32Ret;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_ATTR_JPEG_S stJpegAttr;

	/******************************************
	 step 1: Greate Venc Group
	 ******************************************/
	s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n", VencGrp,
				s32Ret);
		return HI_FAILURE;
	}

	/******************************************
	 step 2:  Create Venc Channel
	 ******************************************/
	stVencChnAttr.stVeAttr.enType = PT_JPEG;

	stJpegAttr.u32MaxPicWidth = pstSize->u32Width;
	stJpegAttr.u32MaxPicHeight = pstSize->u32Height;
	stJpegAttr.u32PicWidth = pstSize->u32Width;
	stJpegAttr.u32PicHeight = pstSize->u32Height;
	stJpegAttr.u32BufSize = pstSize->u32Width * pstSize->u32Height * 2;
	stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
	stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
	stJpegAttr.u32Priority = 0;/*channels precedence level*/
	memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr,
			sizeof(VENC_ATTR_JPEG_S));

	s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n", VencChn,
				s32Ret);
		return s32Ret;
	}
	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : Stop snap
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapStop(VENC_GRP VencGrp, VENC_CHN VencChn) {
	HI_S32 s32Ret;

	s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",
				VencGrp, s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : snap process
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_SnapProcess(VENC_GRP VencGrp, VENC_CHN VencChn,
		VPSS_GRP VpssGrp, VPSS_CHN VpssChn) {
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 s32VencFd;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret;

	/******************************************
	 step 1:  Regist Venc Channel to VencGrp
	 ******************************************/
	s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	/******************************************
	 step 2:  Venc Chn bind to Vpss Chn
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_BindVpss failed!\n");
		return HI_FAILURE;
	}
	/******************************************
	 step 3:  Start Recv Venc Pictures
	 ******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
		return HI_FAILURE;
	}
	/******************************************
	 step 4:  recv picture
	 ******************************************/
	s32VencFd = HI_MPI_VENC_GetFd(VencChn);
	if (s32VencFd < 0) {
		SAMPLE_PRT("HI_MPI_VENC_GetFd faild with%#x!\n", s32VencFd);
		return HI_FAILURE;
	}
//	printf("press any key to snap one pic\n");
//	getchar();

	FD_ZERO(&read_fds);
	FD_SET(s32VencFd, &read_fds);

	TimeoutVal.tv_sec = 2;
	TimeoutVal.tv_usec = 0;
	s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
	if (s32Ret < 0) {
		SAMPLE_PRT("snap select failed!\n");
		return HI_FAILURE;
	} else if (0 == s32Ret) {
		SAMPLE_PRT("snap time out!\n");
		return HI_FAILURE;
	} else {
		if (FD_ISSET(s32VencFd, &read_fds)) {
			s32Ret = HI_MPI_VENC_Query(VencChn, &stStat);
			if (s32Ret != HI_SUCCESS) {
				SAMPLE_PRT("HI_MPI_VENC_Query failed with %#x!\n", s32Ret);
				return HI_FAILURE;
			}

			stStream.pstPack = (VENC_PACK_S*) malloc(
					sizeof(VENC_PACK_S) * stStat.u32CurPacks);
			if (NULL == stStream.pstPack) {
				SAMPLE_PRT("malloc memory failed!\n");
				return HI_FAILURE;
			}

			stStream.u32PackCount = stStat.u32CurPacks;
			s32Ret = HI_MPI_VENC_GetStream(VencChn, &stStream, HI_TRUE);
			if (HI_SUCCESS != s32Ret) {
				SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				return HI_FAILURE;
			}

            s32Ret = SAMPLE_COMM_VENC_SaveSnap(&stStream, VpssGrp);
			if (HI_SUCCESS != s32Ret) {
				SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				return HI_FAILURE;
			}

			s32Ret = HI_MPI_VENC_ReleaseStream(VencChn, &stStream);
			if (s32Ret) {
				SAMPLE_PRT("HI_MPI_VENC_ReleaseStream failed with %#x!\n",
						s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				return HI_FAILURE;
			}

			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}
	}
	/******************************************
	 step 5:  stop recv picture
	 ******************************************/
	s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
	if (s32Ret != HI_SUCCESS) {
		SAMPLE_PRT("HI_MPI_VENC_StopRecvPic failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	/******************************************
	 step 6:  unbind
	 ******************************************/
	s32Ret = SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret) {
		SAMPLE_PRT("SAMPLE_COMM_VENC_UnBindVpss failed!\n");
		return HI_FAILURE;
	}
	/******************************************
	 step 7:  UnRegister
	 ******************************************/
	s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
	if (s32Ret != HI_SUCCESS) {
		SAMPLE_PRT("HI_MPI_VENC_UnRegisterChn failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}
//h264 file save module
void get_time_str264(char * str)
{
struct tm *newtime;
time_t lt1;
time(&lt1);
newtime = localtime(&lt1);
strftime(str, 128, "%F-%H-%M-%S", newtime);
strcat(str, ".h264");
}
void setchnfilename(char * str){
	struct tm *newtime;
	time_t lt1;
	time(&lt1);
	newtime = localtime(&lt1);
	strftime(str, 128, "%F-%H-%M-%S", newtime);
}

void getchnfilename(char * str,unsigned char channel,char *ntimestr,char * fileextension)
{
	char tmpstr[128]={0};
	 strcpy (tmpstr,savepath);
	 sprintf(name, "chn%d_", channel);//chn0
	 strcat (tmpstr,name);
	 strcat(tmpstr,ntimestr);
	 strcat(tmpstr, fileextension);
	 strcpy(str,tmpstr);
}

HI_VOID*  MP3FileInit(HI_U8 channel ) {

	FILE *pFile;
	char str[80];
	struct stat st = {0};

	if (stat("/mnt/hd1/chan", &st) == -1) {
	    mkdir("/mnt/hd1/chan", 0700);
	}
	getchnfilename(str, channel,timestr[channel],".aac");
#if 0
	printf("mp3 file is %s  \n",str);
#endif
	pFile = fopen(str, "wb");
	if (!pFile) {
		SAMPLE_PRT("open file[%s] failed!\n", str);
		return NULL;
	}
	return pFile;
}

HI_VOID*  H264FileInit(HI_U8 channel ) {

	FILE *pFile;
	char str[80];
	struct stat st = {0};

	if (stat("/mnt/hd1/chan", &st) == -1) {
	    mkdir("/mnt/hd1/chan", 0700);
	}
    getchnfilename(str, channel,timestr[channel],".h264");
#if 0
	printf("264 file is %s  \n",str);
#endif
	pFile = fopen(str, "wb");
	if (!pFile) {
		SAMPLE_PRT("open file[%s] failed!\n", str);
		return NULL;
	}
	return pFile;
}


HI_VOID* GetMainVencStreamProc(HI_VOID *p) {
	HI_S32 i;
	HI_S32 s32ChnTotal;
	VENC_CHN_ATTR_S stVencChnAttr;
	SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
	HI_S32 maxfd = 0;
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 VencFd[VENC_MAX_CHN_NUM];
	HI_S32 Fileframecnt[VENC_MAX_CHN_NUM];

	char tmpcommand[256];

	char szFilePostfix[10];

	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
    //use char array to store sps and pps
	HI_S32 s32Ret;
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];

	HI_U8 *typeindex;
	//for audio below zly 2014-07-10
#if AUDIOCOMBINED
	HI_S32 AiFd;
	AUDIO_FRAME_S stFrame;
	AI_CHN_PARAM_S stAiChnPara;
#endif


	pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*) p;
	s32ChnTotal = pstPara->s32Cnt;
	s32ChnTotal=4;//zly

	/******************************************
	 step 1:  check & prepare save-file & venc-fd
	 ******************************************/
	if (s32ChnTotal >= VENC_MAX_CHN_NUM) {
        SAMPLE_PRT("input count invaild \n");
		return NULL;
	}
	SAMPLE_PRT("input count is %d **********\n", s32ChnTotal);

	for (i = 0; i < s32ChnTotal; i++) {
		/* decide the stream file name, and open file to save stream */
		VencChn = i;
		s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != HI_SUCCESS) {
			SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n",
					VencChn, s32Ret);
			return NULL;
		}
		enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

		s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i],
				szFilePostfix);
		if (s32Ret != HI_SUCCESS) {
			SAMPLE_PRT(
					"SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n",
					stVencChnAttr.stVeAttr.enType, s32Ret);
			return NULL;
		}
		setchnfilename(timestr[i]);
		pFile[i]=H264FileInit(i);

		Fileframecnt[i]=0;
		/* Set Venc Fd. */
		VencFd[i] = HI_MPI_VENC_GetFd(i);
		if (VencFd[i] < 0) {
			SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd[i]);
			return NULL;
		}
		if (maxfd <= VencFd[i]) {
			maxfd = VencFd[i];
		}
#if 0
		SAMPLE_PRT("HI_MPI_VENC_GetFd  %x **********\n", VencFd[i]);
#endif

	}
	pthread_mutex_lock(&streammutex);
	mp3File=MP3FileInit(0);
	pthread_mutex_unlock(&streammutex);
#if AUDIOCOMBINED
	//add audioFD
//	VencFd[s32ChnTotal]=AiFd;


#endif
	//debug query venc channel stat

    static timeoutcnt = 0;
    static failedcnt = 0;
	/******************************************
	 step 2:  Start to get streams of each channel.
	 ******************************************/
	while (HI_TRUE == pstPara->bThreadStart) {
		FD_ZERO(&read_fds);
		//s32ChnTotal is audioFD
		for (i = 0; i < s32ChnTotal; i++) {
			FD_SET(VencFd[i], &read_fds);
		}

        TimeoutVal.tv_sec = 3;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
            failedcnt++;
            if(failedcnt%5==0){
                exit(-1);
            }
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("get venc stream time out, exit thread\n");
            timeoutcnt++;
            if(timeoutcnt%5==0){
                exit(-2);
            }
		} else {
			for (i = 0; i <= s32ChnTotal; i++) {
				if (FD_ISSET(VencFd[i], &read_fds)) {
					if (i < s32ChnTotal) {
						/*******************************************************
						 step 2.1 : query how many packs in one-frame stream.
						 *******************************************************/
						memset(&stStream, 0, sizeof(stStream));
						s32Ret = HI_MPI_VENC_Query(i, &stStat);
						//SAMPLE_PRT("HI_MPI_VENC_GetFd  with %#x!\n",  VencFd[i]);
						if (HI_SUCCESS != s32Ret) {
							SAMPLE_PRT(
									"HI_MPI_VENC_Query chn[%d] failed with %#x!\n",
									i, s32Ret);
							break;
						}


                        // Add the code by Jerry.Zhuang, 2009-03-30
                        if (sizeof(VENC_PACK_S)*stStat.u32CurPacks > MAX_BUFF_SIZE)
                        {
                            printf("HI_MPI_VENC_Query: %d > %d\n", sizeof(VENC_PACK_S)*stStat.u32CurPacks, MAX_BUFF_SIZE);

                            break;
                        }

						/*******************************************************
						 step 2.2 : malloc corresponding number of pack nodes.
						 *******************************************************/
						stStream.pstPack = (VENC_PACK_S*) malloc(
								sizeof(VENC_PACK_S) * stStat.u32CurPacks);
						if (NULL == stStream.pstPack) {
							SAMPLE_PRT("malloc stream pack failed!\n");
							break;
						}

						/*******************************************************
						 step 2.3 : call mpi to get one-frame stream
						 *******************************************************/
						stStream.u32PackCount = stStat.u32CurPacks;
						s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							SAMPLE_PRT(	"HI_MPI_VENC_GetStream failed with %#x!\n",	s32Ret);
							break;
						}
						typeindex =	(unsigned char *) stStream.pstPack->pu8Addr[0];

						/*******************************************************
						 step 2.4 : save mainframe to file
						 *******************************************************/
						char naltype = typeindex[4];
                        if ((Fileframecnt[i] > recordfrmcnt) && (naltype == 0x67)) {
							Fileframecnt[i] = 0;
							fflush(pFile[i]);
							fclose(pFile[i]);
							setchnfilename(timestr[i]);  //renew file time mark
							printf("i is %d\n",i);
							pFile[i] = H264FileInit(i);
#if 1
							if (i == 0) {
						      //  mp3flush(mp3File);
								pthread_mutex_lock(&streammutex);
                                if(mp3File != NULL){
                                    fclose(mp3File);
                                    mp3File = NULL;
                                }

                                mp3File = MP3FileInit(0);
                                pthread_mutex_unlock(&streammutex);
                                printf("audiostreamingflag=%d\n", audiostreamingflag);

//								tmpcommand[0] = '\0';
                                if(minisStd){
                                    sprintf(tmpcommand,"/lib/aactowav.sh  %s &",oldtimestr[0]/*,oldtimestr[1],oldtimestr[2],oldtimestr[3]*/);
                                }
//								puts(tmpcommand);
                                printf("aactowav.sh:%s\n", tmpcommand);
//                                system(tmpcommand);
							}

						}
						if((Fileframecnt[i] > 2000))
												strcpy(oldtimestr[i], timestr[i]);
#endif
						s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i],
								pFile[i], &stStream);
						Fileframecnt[i]++;

#if 0
						SAMPLE_PRT("packet type is  %d  ****** !\n",
								stStream.pstPack->DataType.enH264EType);
						SAMPLE_PRT("packet length is  %d  ****** !\n",
								stStream.pstPack->u32Len[0]);
						memcpy(streambuffer, stStream.pstPack->pu8Addr[0],
								stStream.pstPack->u32Len[0]);
						printf("nal type is 0x%x  ....\n", streambuffer[4]);
#endif
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							SAMPLE_PRT("save stream failed!\n");
							break;
						}
						/*******************************************************
						 step 2.5 : release stream
						 *******************************************************/
						s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							break;
						}
						/*******************************************************
						 step 2.6 : free pack nodes
						 *******************************************************/
						free(stStream.pstPack);
						stStream.pstPack = NULL;
					}
				}
			}
		}
	}

	/*******************************************************
	 * step 3 : close save-file
	 *******************************************************/
	for (i = 0; i < s32ChnTotal; i++) {
		fclose(pFile[i]);
	}

	return NULL;
}

//char spsppsarray[100];


//get sub stream encode data
HI_VOID* GetSubVencStreamProc(HI_VOID *p) {

	HI_S32 streamingID;
	VENC_CHN_ATTR_S stVencChnAttr;
	SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
	struct timeval TimeoutVal;
	fd_set read_fds;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret;
	VENC_CHN VencChn;


	HI_S32 s32ChnTostart = 4;
	HI_S32 s32ChnTotal=8;//zly
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
	int i=0;
	HI_S32 VencFd[VENC_MAX_CHN_NUM];
	char szFilePostfix[10];
	HI_S32 maxfd = 0;
	pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*) p;

#if VIDEOSHARED

    init_video_buffer(&m_videobuffer, "/lib/tmvideobuf0", 5, 20);

	videobitstream m_videobitstream;
	int hi_spssize=0;
	int hi_ppssize=0;
    int hi_seisize=0;
    int videolength=0;
#endif

#if AUDIOCOMBINED
	HI_S32 AiFd;
	AUDIO_FRAME_S stFrame;
	AI_CHN_PARAM_S stAiChnPara;
	HI_S32 AiDev=2,	AiChn=0;
	s32Ret = HI_MPI_AI_GetChnParam(AiDev, AiChn,
			&stAiChnPara);
	if (HI_SUCCESS != s32Ret) {
		printf("%s: Get ai chn param failed\n", __FUNCTION__);
		return NULL;
	}

	stAiChnPara.u32UsrFrmDepth = 30;

	s32Ret = HI_MPI_AI_SetChnParam(AiDev, AiChn,
			&stAiChnPara);
	if (HI_SUCCESS != s32Ret) {
		printf("%s: set ai chn param failed\n", __FUNCTION__);
		return NULL;
	}

	FD_ZERO(&read_fds);
	AiFd = HI_MPI_AI_GetFd(AiDev, AiChn);
	printf("AiFd is %d\n", AiFd);
	//audio init
//	    initshine();//mp3 codec init
	Init_Voaac();								//aac codec init

#endif
	for (i = s32ChnTostart; i < s32ChnTotal; i++) {
		/* decide the stream file name, and open file to save stream */
		VencChn = i;
		s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != HI_SUCCESS) {
			SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n",
					VencChn, s32Ret);
			return NULL;
		}
		enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

		/* Set Venc Fd. */
		VencFd[i] = HI_MPI_VENC_GetFd(i);
#if 1
		SAMPLE_PRT("HI_MPI_VENC_GetFd  %x **********\n", VencFd[i]);
#endif

	}

	//init videobuf
	VencFd[s32ChnTotal]=AiFd;
    for(i=0;i<=s32ChnTotal;i++)  //wxb
	{
		if (VencFd[i] < 0) {
			SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd[i]);
			return NULL;
		}
		if (maxfd <= VencFd[i]) {
			maxfd = VencFd[i];
		}
	}

	/******************************************
	 step 2:  Start to get streams of each channel.
	 ******************************************/
	while (HI_TRUE == pstPara->bThreadStart) {

		FD_ZERO(&read_fds);
        for (i = s32ChnTostart; i <= s32ChnTotal; i++) {
			FD_SET(VencFd[i], &read_fds);
		}

		TimeoutVal.tv_sec = 1;
		TimeoutVal.tv_usec = 0;

        s32Ret = select(maxfd + 1 , &read_fds, NULL, NULL, &TimeoutVal);

		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("get venc stream time out, exit thread\n");
			continue;
		} else {
			for (i = s32ChnTostart; i <= s32ChnTotal; i++) {
				if (FD_ISSET(VencFd[i], &read_fds)) {
                    if (i < s32ChnTotal) {
                    #if 1
						/*******************************************************
						 step 2.1 : query how many packs in one-frame stream.
						 *******************************************************/
						memset(&stStream, 0, sizeof(stStream));
						s32Ret = HI_MPI_VENC_Query(i, &stStat);

						//SAMPLE_PRT("HI_MPI_VENC_GetFd  with %#x!\n",  VencFd[i]);
						if (HI_SUCCESS != s32Ret) {
							SAMPLE_PRT(
									"HI_MPI_VENC_Query chn[%d] failed with %#x!\n",
									i, s32Ret);
							break;
						}


                        // Add the code by Jerry.Zhuang, 2009-03-30
                        if (sizeof(VENC_PACK_S)*stStat.u32CurPacks > MAXSUB_BUFF_SIZE)
                        {
                            printf("HI_MPI_VENC_Query: %d > %d\n", sizeof(VENC_PACK_S)*stStat.u32CurPacks, MAX_BUFF_SIZE);

                            break;
                        }



						/*******************************************************
						 step 2.2 : malloc corresponding number of pack nodes.
						 *******************************************************/
						stStream.pstPack = (VENC_PACK_S*) malloc(
								sizeof(VENC_PACK_S) * stStat.u32CurPacks);
						if (NULL == stStream.pstPack) {
							SAMPLE_PRT("malloc stream pack failed!\n");
							break;
						}


						/*******************************************************
						 step 2.3 : call mpi to get one-frame stream
						 *******************************************************/
						stStream.u32PackCount = stStat.u32CurPacks;
						s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							SAMPLE_PRT(
									"HI_MPI_VENC_GetStream failed with %#x!\n",
									s32Ret);
							break;
						}

#if VIDEOSHARED
						int hi_naltype = stStream.pstPack->DataType.enH264EType;
//                        printf("channel ,naltype is %d %d \n",i,hi_naltype);
						m_videobitstream.naltype =
								stStream.pstPack->DataType.enH264EType;
						m_videobitstream.videochanindex = i - 4;
						videolength = stStream.pstPack->u32Len[0];
						char *buffhead = stStream.pstPack->pu8Addr[0];

						if (m_videobitstream.naltype == 0x07) {
							hi_spssize = videolength;

							memcpy(m_videobitstream.videodata, buffhead,
									hi_spssize);
						}

						if (m_videobitstream.naltype == 0x08) {
							hi_ppssize = videolength;

							m_videobitstream.videoseq = stStream.u32Seq;
							m_videobitstream.size = hi_spssize + hi_ppssize;

							memcpy(m_videobitstream.videodata + hi_spssize,
									buffhead, hi_ppssize);
							//printf("write to shared memory %d",i - 4);
#if 1
												//if (ChannelStreammark[i-4] == HI_TRUE)
							{
								//	printf("write to shared memory %d \n",i - 4);
								if(audiostreamingflag==HI_TRUE)
								videobuffer_write(&m_videobuffer,
										m_videobitstream);
							}

#endif

						}

						if (m_videobitstream.naltype == 0x06) {
							hi_seisize = videolength;

							m_videobitstream.videoseq = stStream.u32Seq;
							;
							m_videobitstream.size = hi_seisize;

							memcpy(m_videobitstream.videodata, buffhead,
									hi_seisize);
#if 1
							//streamingcontrol_lock();
							//	if (ChannelStreammark[i-4] == HI_TRUE)
							{
								if(audiostreamingflag==HI_TRUE)
								videobuffer_write(&m_videobuffer,
										m_videobitstream);
							}
							//streamingcontrol_unlock();
#endif
						}



                        if ((m_videobitstream.naltype == 0x05)
                                || (m_videobitstream.naltype == 0x01))
                        {
                            #if 1
                            videolength = 0;

                            int t = 0;
                            char * bitstreambuffer = 0;
                            int cpLen = 0;
                            m_videobitstream.size = 0;
                            bitstreambuffer = m_videobitstream.videodata;

                            for (t = 0; t < stStream.u32PackCount; t++) {
                                cpLen = stStream.pstPack[t].u32Len[0];


//                                printf("aa11 t=%d cnt=%d naltype=%x \n", t, stStream.u32PackCount, m_videobitstream.naltype);
//                                printf("videolength=%d, u32Len[0]=%d u32Len[1]=%d\n", videolength, stStream.pstPack[t].u32Len[0], stStream.pstPack->u32Len[1]);


                                if(cpLen > sizeof(m_videobitstream.videodata) - videolength){
                                    videolength = 0;
                                    m_videobitstream.size = 0;
                                    printf("gt1 videodata buf\n");
                                    break;
                                }
                                memcpy(bitstreambuffer + videolength, stStream.pstPack[t].pu8Addr[0], cpLen);
//                                printf("aa11 t=%d  \n", t);


                                videolength = videolength
                                        + stStream.pstPack[t].u32Len[0];
//                                printf("aa11 t2=%d  \n", t);

                                if (stStream.pstPack->u32Len[1] > 0) {

                                    cpLen = stStream.pstPack[t].u32Len[1];
                                    if(cpLen > sizeof(m_videobitstream.videodata) - videolength){
                                        videolength = 0;
                                        m_videobitstream.size = 0;
                                        printf("gt2 videodata buf\n");
                                        break;
                                    }
                                    memcpy(bitstreambuffer + videolength, stStream.pstPack[t].pu8Addr[1], cpLen);
                                    videolength = videolength
                                            + stStream.pstPack[t].u32Len[1];
//                                    printf("aa11 t3=%d  \n", t);
                                }
//                                printf("aa11 t4=%d  \n", t);

                            }
                            m_videobitstream.videoseq = stStream.u32Seq;
                            m_videobitstream.size = videolength;
#if 1
                            //streamingcontrol_lock();
//                            if ( ChannelStreammark[i-4] == HI_TRUE)
                            if(m_videobitstream.size > 0)
                            {
                                if(audiostreamingflag==HI_TRUE)
                                videobuffer_write(&m_videobuffer,
                                        m_videobitstream);
                            }
                            //streamingcontrol_unlock();
#endif
//                            printf("aa55 \n");
                        #endif
                        }
#endif

						/*******************************************************
						 show debu info of bitstream
						 *******************************************************/
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							SAMPLE_PRT("save stream failed!\n");
							break;
						}
						/*******************************************************
						 step 2.5 : release stream
						 *******************************************************/
						s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
						if (HI_SUCCESS != s32Ret) {
							free(stStream.pstPack);
							stStream.pstPack = NULL;
							break;
						}
						/*******************************************************
						 step 2.6 : free pack nodes
						 *******************************************************/
						free(stStream.pstPack);
						stStream.pstPack = NULL;
                    #endif
                    }
                    else {
                    #if 1
                        //get audio
                        /* get frame from ai chn */
//                        printf("get audio frame\n");
                        s32Ret = HI_MPI_AI_GetFrame(AiDev, AiChn, &stFrame,
                        NULL, HI_FALSE);
                        if (HI_SUCCESS != s32Ret) {
                            printf(
                                    "%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!\n",
                                    __FUNCTION__, AiDev, AiChn, s32Ret);

                            return NULL;
                        } else {
#if 0
                            printf("AI GetFrame Len::is:: %d\n",stFrame.u32Len);
#endif
//                            pthread_mutex_lock(&streammutex);

                            FillAACPacket((short *) stFrame.pVirAddr[0], mp3File, audiostreamingflag);			//
//                            pthread_mutex_unlock(&streammutex);
                        }

                        /* finally you must release the stream */
                        HI_MPI_AI_ReleaseFrame(AiDev, AiChn, &stFrame, NULL);

                    #endif
                    }


				}
			}

		}

	}

	printf("thread exit\n*************");
	return NULL;
}

/******************************************************************************
 * funciton : get stream from each channels and save them
 ******************************************************************************/
HI_VOID* SAMPLE_COMM_VENC_GetVencStreamProc(HI_VOID *p) {
	HI_S32 i;
	HI_S32 s32ChnTotal;
	VENC_CHN_ATTR_S stVencChnAttr;
	SAMPLE_VENC_GETSTREAM_PARA_S *pstPara;
	HI_S32 maxfd = 0;
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 VencFd[VENC_MAX_CHN_NUM];
	HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
	FILE *pFile[VENC_MAX_CHN_NUM];
	char szFilePostfix[10];
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret;
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
	unsigned char streambuffer[100 * 1024];

	pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*) p;
	s32ChnTotal = pstPara->s32Cnt;

	/******************************************
	 step 1:  check & prepare save-file & venc-fd
	 ******************************************/
	if (s32ChnTotal >= VENC_MAX_CHN_NUM) {
		SAMPLE_PRT("input count invaild\n");
		return NULL;
	}
	SAMPLE_PRT("input count is %d **********\n", s32ChnTotal);
	for (i = 0; i < s32ChnTotal; i++) {
		/* decide the stream file name, and open file to save stream */
		VencChn = i;
		s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != HI_SUCCESS) {
			SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed xxxxwith %#x!\n",
					VencChn, s32Ret);
			return NULL;
		}
		enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

		s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i],
				szFilePostfix);
		if (s32Ret != HI_SUCCESS) {
			SAMPLE_PRT(
					"SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n",
					stVencChnAttr.stVeAttr.enType, s32Ret);
			return NULL;
		}
		sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);
		//create main stream file
		if (i % 2 == 0) {
			pFile[i] = fopen(aszFileName[i], "wb");
			if (!pFile[i]) {
				SAMPLE_PRT("open file[%s] failed!\n", aszFileName[i]);
				return NULL;
			}
		}

		/* Set Venc Fd. */
		VencFd[i] = HI_MPI_VENC_GetFd(i);
#if 1
		SAMPLE_PRT("HI_MPI_VENC_GetFd  %x **********\n", VencFd[i]);
#endif
		if (VencFd[i] < 0) {
			SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd[i]);
			return NULL;
		}
		if (maxfd <= VencFd[i]) {
			maxfd = VencFd[i];
		}
	}
	//debug query venc channel stat
#if 1
	for (i = 0; i < s32ChnTotal; i++) {
		memset(&stStream, 0, sizeof(stStream));
		s32Ret = HI_MPI_VENC_Query(i, &stStat);
		//SAMPLE_PRT("HI_MPI_VENC_GetFd  with %#x!\n",  VencFd[i]);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i,
					s32Ret);
			break;
		}
	}
#endif

	/******************************************
	 step 2:  Start to get streams of each channel.
	 ******************************************/
	while (HI_TRUE == pstPara->bThreadStart) {
		FD_ZERO(&read_fds);
		for (i = 0; i < s32ChnTotal; i++) {
			FD_SET(VencFd[i], &read_fds);
		}

		TimeoutVal.tv_sec = 2;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) {
			SAMPLE_PRT("select failed!\n");
			break;
		} else if (s32Ret == 0) {
			SAMPLE_PRT("get venc stream time out, exit thread\n");
			continue;
		} else {
			for (i = 0; i < s32ChnTotal; i++) {
				if (FD_ISSET(VencFd[i], &read_fds)) {
					/*******************************************************
					 step 2.1 : query how many packs in one-frame stream.
					 *******************************************************/
					memset(&stStream, 0, sizeof(stStream));
					s32Ret = HI_MPI_VENC_Query(i, &stStat);
					//SAMPLE_PRT("HI_MPI_VENC_GetFd  with %#x!\n",  VencFd[i]);
					if (HI_SUCCESS != s32Ret) {
						SAMPLE_PRT(
								"HI_MPI_VENC_Query chn[%d] failed with %#x!\n",
								i, s32Ret);
						break;
					}

					/*******************************************************
					 step 2.2 : malloc corresponding number of pack nodes.
					 *******************************************************/
					stStream.pstPack = (VENC_PACK_S*) malloc(
							sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					if (NULL == stStream.pstPack) {
						SAMPLE_PRT("malloc stream pack failed!\n");
						break;
					}

					/*******************************************************
					 step 2.3 : call mpi to get one-frame stream
					 *******************************************************/
					stStream.u32PackCount = stStat.u32CurPacks;
					s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
					if (HI_SUCCESS != s32Ret) {
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n",
								s32Ret);
						break;
					}

					/*******************************************************
					 step 2.4 : save mainframe to file
					 *******************************************************/
					if (i % 2 == 0) {
						s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i],
								pFile[i], &stStream);
					}

#if 1
					SAMPLE_PRT("packet type is  %d  ****** !\n",
							stStream.pstPack->DataType.enH264EType);
					SAMPLE_PRT("packet length is  %d  ****** !\n",
							stStream.pstPack->u32Len[0]);
					memcpy(streambuffer, stStream.pstPack->pu8Addr[0],
							stStream.pstPack->u32Len[0]);
					printf("nal type is 0x%x  ....\n", streambuffer[4]);
#endif
					if (HI_SUCCESS != s32Ret) {
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						SAMPLE_PRT("save stream failed!\n");
						break;
					}
					/*******************************************************
					 step 2.5 : release stream
					 *******************************************************/
					s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
					if (HI_SUCCESS != s32Ret) {
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						break;
					}
					/*******************************************************
					 step 2.6 : free pack nodes
					 *******************************************************/
					free(stStream.pstPack);
					stStream.pstPack = NULL;
				}
			}
		}
	}

	/*******************************************************
	 * step 3 : close save-file
	 *******************************************************/
	for (i = 0; i < s32ChnTotal; i++) {
		fclose(pFile[i]);
	}

	return NULL;
}
/******************************************************************************
 * funciton : start get venc stream process thread
 ******************************************************************************/
HI_S32 VENC_StartGetMainStream(SAMPLE_VENC_GETSTREAM_PARA_S *spara) {
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = spara->s32Cnt;
	gs_stPara.recordpath = spara->recordpath;
#if 1
	gs_stPara.AencChn=spara->AencChn;
	gs_stPara.AiChn=spara->AiChn;
	gs_stPara.AiDev=spara->AiDev;
#endif

    pthread_attr_t stAttr;
    struct sched_param stShedParam;
    pthread_attr_init(&stAttr);
    pthread_attr_getschedparam(&stAttr, &stShedParam);
    stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-2;
    pthread_attr_setschedparam(&stAttr, &stShedParam);

    return pthread_create(&gs_VencPid, &stAttr, GetMainVencStreamProc,
			(HI_VOID*) &gs_stPara);
}

HI_S32 VENC_GetSubStream(HI_S32 s32Cnt) {
	substream_stPara[s32Cnt].bThreadStart = HI_TRUE;
	substream_stPara[s32Cnt].s32Cnt = s32Cnt; //channel ID
	pthread_attr_t stAttr;
	struct sched_param stShedParam;
	pthread_attr_init(&stAttr);
	pthread_attr_getschedparam(&stAttr, &stShedParam);
    stShedParam.sched_priority = sched_get_priority_max(SCHED_RR)-3;
	pthread_attr_setschedparam(&stAttr, &stShedParam);

	return pthread_create(&gs_VencPid[s32Cnt], 0, GetSubVencStreamProc,
			(HI_VOID*) &substream_stPara[s32Cnt]);
}
/******************************************************************************
 * funciton : start get venc stream process thread
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_StartGetStream(HI_S32 s32Cnt) {
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = s32Cnt;

	return pthread_create(&gs_VencPid, 0, SAMPLE_COMM_VENC_GetVencStreamProc,
			(HI_VOID*) &gs_stPara);
}

/******************************************************************************
 * funciton : stop get venc stream process.
 ******************************************************************************/
HI_S32 SAMPLE_COMM_VENC_StopGetStream() {
	if (HI_TRUE == gs_stPara.bThreadStart) {
		gs_stPara.bThreadStart = HI_FALSE;
		pthread_join(gs_VencPid, 0);
	}
	return HI_SUCCESS;
}

HI_VOID SAMPLE_COMM_VENC_ReadOneFrame(FILE * fp, HI_U8 * pY, HI_U8 * pU,
		HI_U8 * pV, HI_U32 width, HI_U32 height, HI_U32 stride, HI_U32 stride2) {
	HI_U8 * pDst;

	HI_U32 u32Row;

	pDst = pY;
	for (u32Row = 0; u32Row < height; u32Row++) {
		fread(pDst, width, 1, fp);
		pDst += stride;
	}

	pDst = pU;
	for (u32Row = 0; u32Row < height / 2; u32Row++) {
		fread(pDst, width / 2, 1, fp);
		pDst += stride2;
	}

	pDst = pV;
	for (u32Row = 0; u32Row < height / 2; u32Row++) {
		fread(pDst, width / 2, 1, fp);
		pDst += stride2;
	}

}

HI_S32 SAMPLE_COMM_VENC_PlanToSemi(HI_U8 *pY, HI_S32 yStride, HI_U8 *pU,
		HI_S32 uStride, HI_U8 *pV, HI_S32 vStride, HI_S32 picWidth,
		HI_S32 picHeight) {
	HI_S32 i;
	HI_U8* pTmpU, *ptu;
	HI_U8* pTmpV, *ptv;

	HI_S32 s32HafW = uStride >> 1;
	HI_S32 s32HafH = picHeight >> 1;
	HI_S32 s32Size = s32HafW * s32HafH;

	pTmpU = malloc(s32Size);
	ptu = pTmpU;
	pTmpV = malloc(s32Size);
	ptv = pTmpV;

	memcpy(pTmpU, pU, s32Size);
	memcpy(pTmpV, pV, s32Size);

	for (i = 0; i < s32Size >> 1; i++) {
		*pU++ = *pTmpV++;
		*pU++ = *pTmpU++;

	}
	for (i = 0; i < s32Size >> 1; i++) {
		*pV++ = *pTmpV++;
		*pV++ = *pTmpU++;
	}

	free(ptu);
	free(ptv);

	return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
