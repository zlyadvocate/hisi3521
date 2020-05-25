
/*
 * osdtime.c
 *
 *  Created on: Mar 17, 2014
 *      Author: zhangly
 *      jinsidao Com.Limited
 *      All rights reserved!
 */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "osdtime.h"
#include "hi_ascii_8_12.h"
#include "hi_ascii_8_16.h"
#include "hi_ascii_20_24.h"
pthread_t osdtimethreadid;

HI_BOOL g_ShowTimeThreadFlag=HI_FALSE;
RGN_HANDLE OverlayExHandle;
MPP_CHN_S stOverlayExChn[VMAX_CHANNEL_NUM];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char szStation[42] = {0};
extern char mdn[32];
unsigned short nSpeed = 0;
#define OSDLENGTH 72



int VIDEO_RegisterFont(const char * FontName, const HI_U8 ** FontEntry,
		HI_U32 FontWidth, HI_U32 FontHeight) {
	VIDEO_FONT_LIST * pNewVideoFontNode;
	VIDEO_FONT_LIST * pLastideoFontNode;
	HI_U32 FontNameLen;

	if ((NULL == FontName) || (NULL == FontEntry) || (0 == FontWidth)
			|| (0 == FontHeight)) {
		return -1;
	}

	pNewVideoFontNode = (VIDEO_FONT_LIST *) malloc(sizeof(VIDEO_FONT_LIST));
	if (NULL == pNewVideoFontNode) {
		printf("not enough memory to malloc!\n");
		return -1;
	}

	FontNameLen = strlen(FontName);
	if (FontNameLen >= MAX_FONT_NAME) {
		strncpy(pNewVideoFontNode->VideoFont.FontName, FontName,
				MAX_FONT_NAME - 1);
		pNewVideoFontNode->VideoFont.FontName[MAX_FONT_NAME - 1] = '\0';
	} else {
		strcpy(pNewVideoFontNode->VideoFont.FontName, FontName);
	}

	pNewVideoFontNode->VideoFont.FontEntry = (HI_U8 **) FontEntry;
	pNewVideoFontNode->VideoFont.FontWidth = FontWidth;
	pNewVideoFontNode->VideoFont.FontHeight = FontHeight;
	pNewVideoFontNode->pNext = NULL;

	if (g_pVideoFontList == NULL) {
		g_pVideoFontList = pNewVideoFontNode;
		return 0;
	}

	pLastideoFontNode = g_pVideoFontList;
	for (;;) {
		if (pLastideoFontNode->pNext) {
			pLastideoFontNode = pLastideoFontNode->pNext;
		} else {
			pLastideoFontNode->pNext = pNewVideoFontNode;
			return 0;
		}
	}
	return 0;
}

int VIDEO_SetFont(const char * FontName) {
	VIDEO_FONT_LIST * pVideoFontNode;

	if (NULL == FontName)
		return -1;

	pVideoFontNode = g_pVideoFontList;
	while (pVideoFontNode) {
		if (strcmp(pVideoFontNode->VideoFont.FontName, FontName) == 0) {
			memcpy(&g_CurrentVideoFont, &(pVideoFontNode->VideoFont),
					sizeof(VIDEO_FONT));
			return 0;
		}

		pVideoFontNode = pVideoFontNode->pNext;
	}

	return -1;
}

int VIDEO_FontInit(void) {
	int ret = 0;

	ret |= VIDEO_RegisterFont("8_12", ascii_table_8_12, 8, 12);
	ret |= VIDEO_RegisterFont("8_16", ascii_table_8_16, 8, 16);

	ret |= VIDEO_RegisterFont("20_24", ascii_table_20_24, 20, 24);
	if (0 != ret) {
		return -1;
	}

	ret = VIDEO_SetFont("8_16");
	if (0 != ret) {
		return -1;
	}

	return 0;
}

VIDEO_FONT * VIDEO_GetFont(void) {
	return &g_CurrentVideoFont;
}
int VIDEO_HZ16TextOut(VIDEO_TEXT_T * pVText, HI_U32 ulXStart, HI_U32 ulYStart,
		const char* pString)
{
	HI_U8 lineData;
	HI_U32 lineDataNum;
	HI_U8 * pLineData;
	FILE *fp;
	fp=fopen("HZK16","rb++");
	if(fp==NULL){
		/*打开hzk16字库错误 */

		exit(0);
	}

	HI_U32 charIndex, charLen, charLine, charRow;

	HI_U32 charValidWidth;
	HI_U8 qh,wh;
	HI_U32 location;
//	HI_U8  hzbytearray[32]={0x00,0x10,0x1F,0xF8,0x10,0x10,0x11,0x10,0x11,0x10,0x11,0x10,0x11,0x10,0x11,0x10,
//			0x11,0x10,0x12,0x10,0x12,0x90,0x04,0x80,0x04,0x82,0x08,0x82,0x30,0x7e,0xc0,0x00};
	HI_U8  hzbytearray[32]={0x04,0x80,0x0E,0xA0,0x78,0x90,0x08,0x90,0x08,0x84,0xFF,0xFE,0x08,0x80,0x08,0x90
	,0x0A,0x90,0x0C,0x60,0x18,0x40,0x68 ,0xA0,0x09,0x20,0x0A,0x14,0x28,0x14,0x10,0x0C};
//	HI_U8  hzbytearray[32]={0x00,0x80,0x40,0x40,0x30,0x00,0x13,0xc0,0x80,0x44,0x60,0x4c,0x27,0xf0,0x08,0xe0,
//	0x11,0x50,0x21,0x50,0xe2,0x48,0x22,0x4e,0x24,0x44,0x28,0x40,0x21,0x40,0x20,0x80};

	if ((NULL == pVText) || (NULL == pString)) {
		return -1;
	}

	lineDataNum = (16 - 1) / 8 + 1;

	charLen = strlen(pString)/2;//hz use two bytes

	if (pVText->pRGBBuffer == NULL) {
		printf("not enough memory to malloc!\n");
		return -1;
	}
    //16*16 hzk
	//one hz need 32 bytes.
	for (charIndex = 0; charIndex < charLen; charIndex++) {

		qh = pString[charIndex*2] - 0xa0;   /* 计算区码     */
		wh = pString[charIndex*2+ 1]- 0xa0; /* 计算位码     */
		location = (94 * (qh - 1) + (wh - 1)) * 32L; /* 计算字模在文件中的位置  */
//		printf("qh = %d,qh= %d location = %d    0x%x \n",qh,wh,location,location);
		fseek(fp, location, SEEK_SET);
		fread(hzbytearray, 32, 1, fp);

//		int m;
//		for (m=0;m<32;m++)
//		{
//			printf("0x%x ",hzbytearray[m]);
//			if(m==15)
//				printf("\n");
//		}
//		printf("\n");

		/* Font Height */
		for (charLine = 0; charLine < 16; charLine++) {
			charValidWidth = 16;
			//deal with one line
			//1555 16 bit
			pLineData = pVText->pRGBBuffer + charLine * pVText->width * 2
                    + charIndex * 16 * 2 + ulXStart * 32;
			/* Font Width 16*/
            //deal with every point of the line
//			printf("%d ",charLine);
			for (charRow = 0; charRow < charValidWidth; charRow++) {
				//2 byte
				lineData = hzbytearray[(charLine * lineDataNum) + (charRow / 8)];
				if (lineData & (0x80 >> (charRow % 8))) {
					memcpy((void *) pLineData, (void *) (&g_VideoTextColor), 2); //2---rgb1555
				} else if (g_VideoTextBgMode) {
					memcpy((void *) pLineData, (void *) (&g_VideoTextBgColor),	2); //2---rgb1555
				}
			    pLineData += 2; //one point

			}
//		   printf("\n");

		}

	}
	fclose(fp);
	return 0;
}

int VIDEO_TextOut(VIDEO_TEXT_T * pVText, HI_U32 ulXStart, HI_U32 ulYStart,
		const char* pString) {
	HI_U8 lineData;
	HI_U32 lineDataNum;
	HI_U8 * pLineData;

	HI_U32 charIndex, charLen, charLine, charRow;

	HI_U32 charValidWidth;

	//HI_U32 charMemOffset;

	if ((NULL == pVText) || (NULL == pString)) {
		return -1;
	}

	lineDataNum = (g_CurrentVideoFont.FontWidth - 1) / 8 + 1;

	charLen = strlen(pString);
#if 0
	pVText->height = g_CurrentVideoFont.FontHeight;
	pVText->width = g_CurrentVideoFont.FontWidth * charLen;
	pVText->pRGBBuffer = (HI_U8 *)malloc(pVText->width * pVText->height);
#endif
	if (pVText->pRGBBuffer == NULL) {
		printf("not enough memory to malloc!\n");
		return -1;
	}

	//charMemOffset = 0;
	for (charIndex = 0; charIndex < charLen; charIndex++) {
		charValidWidth = g_CurrentVideoFont.FontWidth;
		/* Font Height */
		for (charLine = 0; charLine < g_CurrentVideoFont.FontHeight;
				charLine++) {
			//pLineData = pVText->pRGBBuffer + charLine*pVText->width + charMemOffset;
			pLineData = pVText->pRGBBuffer + charLine * pVText->width * 2
					+ charIndex * g_CurrentVideoFont.FontWidth * 2;
			/* Font Width */
			for (charRow = 0; charRow < charValidWidth; charRow++) {
				lineData =
						g_CurrentVideoFont.FontEntry[(HI_U8) pString[charIndex]][charLine
								* lineDataNum + charRow / 8];
				if (lineData & (0x80 >> (charRow % 8))) {
					memcpy((void *) pLineData, (void *) (&g_VideoTextColor), 2); //2---rgb1555
				} else if (g_VideoTextBgMode) {
					memcpy((void *) pLineData, (void *) (&g_VideoTextBgColor),
							2); //2---rgb1555
				}
				pLineData += 2; //2---rgb1555
			}
		}
		//charMemOffset += g_CurrentVideoFont.FontWidth*3;
	}
	return 0;
}

int VIDEO_SetTextColor(HI_U32 ulTextColor) {
	unsigned short A1555 = 0x8000;
	unsigned short REDColor = GET_R_COMPONENT(ulTextColor);
	unsigned short GREENColor = GET_G_COMPONENT(ulTextColor);
	unsigned short BLUEColor = GET_B_COMPONENT(ulTextColor);

	unsigned short data_OUT = (A1555) | (((REDColor >> 3) & 0x1f) << 10)
			| (((GREENColor >> 3) & 0x1f) << 5) | ((BLUEColor >> 3) & 0x1f);
	g_VideoTextColor[1] = data_OUT >> 8;
	g_VideoTextColor[0] = data_OUT & 0xFF;
	return 0;
}

int VIDEO_SetTextBgMode(VIDEO_BG_MODE_T BgMode) {
	if (VIDEO_BG_MODE_BUTT <= BgMode) {
		return -1;
	}

	g_VideoTextBgMode = BgMode;

	return 0;
}

int VIDEO_SetTextBgColor(HI_U32 ulBgColor) {
	unsigned short A1555 = 0x8000;
	unsigned short REDColor = GET_R_COMPONENT(ulBgColor);
	unsigned short GREENColor = GET_G_COMPONENT(ulBgColor);
	unsigned short BLUEColor = GET_B_COMPONENT(ulBgColor);

	unsigned short data_OUT = (A1555) | (((REDColor >> 3) & 0x1f) << 10)
			| (((GREENColor >> 3) & 0x1f) << 5) | ((BLUEColor >> 3) & 0x1f);
	g_VideoTextBgColor[1] = data_OUT >> 8;
	g_VideoTextBgColor[0] = data_OUT & 0xFF;

	return 0;
}

/**********************************************************
Function     : Osd_Text2Bitmap
Description  : 把文本格式转换成bmp格式
Input        : pVText, pString
Output       : 无
Return Value : 成功 SUCCESS,
               失败 FAILURE
Date         : 2014/03/18
Author       : zhangly
**********************************************************/
int Osd_Text2Bitmap(VIDEO_TEXT_T *pVText, const char *pString)
{
    HI_U8 LineData;
    HI_U32 LineDataNum;
    HI_U8 *pLineData;
    HI_U32 CharIndex;
    HI_U32 StringLen;
    HI_U32 CharLine;
    HI_U32 CharRow;
    HI_U32 CharValidWidth;

    /*输入参数合法性检查*/
    if((NULL == pVText) || (NULL == pString) || (NULL == pVText->pRGBBuffer))
    {
        return HI_FALSE;
    }

    LineDataNum = ((g_CurrentVideoFont.FontWidth - 1) / 8) + 1;
    StringLen = strlen(pString);

    for(CharIndex = 0; CharIndex < StringLen; CharIndex++)
    {
        CharValidWidth = g_CurrentVideoFont.FontWidth;

        for(CharLine = 0; CharLine < g_CurrentVideoFont.FontHeight; CharLine++)
        {
            pLineData = pVText->pRGBBuffer
                        + (CharLine * (pVText->width) * 2)
                        + (CharIndex * g_CurrentVideoFont.FontWidth * 2);

            for(CharRow = 0; CharRow < CharValidWidth; CharRow++)
            {
                LineData = g_CurrentVideoFont.FontEntry[(HI_U8)pString[CharIndex]][(CharLine * LineDataNum) + (CharRow / 8)];
                if(LineData & (0x80 >> (CharRow % 8)))
                {
                    memcpy((void*)pLineData, (void*)(&g_VideoTextColor), 2);
                }
                else if(g_VideoTextBgMode)
                {
                    memcpy((void*)pLineData, (void*)(&g_VideoTextBgColor), 2);
                }
                pLineData += 2;
            }
        }
    }

    return HI_SUCCESS;
}



/**********************************************************
Function     : Osd_TimeOut
Description  : 显示输出时间OSD
Input        : VOID *args
Output       : 无
Return Value : 成功 SUCCESS,
               失败 FAILURE
Date         : 2007/10/27
Author       : cuixianwang
**********************************************************/
void *Osd_TimeOut(void *args)
{
    char szString[OSDLENGTH] = {0};
    BITMAP_S stBitmap;
    VIDEO_TEXT_T stText;
    VIDEO_FONT *pstFont = NULL;
    int RetValue;
    struct tm *pstTimeNow = NULL;
    time_t TimeNow;

    /*输入参数合法性判断
    if(NULL == args)
    {
        return;
    }*/

    /*设置字体大小颜色和背景颜色*/
    VIDEO_SetTextColor(0x00ff00);
    VIDEO_SetTextBgColor(0x000000);

    pstFont = VIDEO_GetFont();
    stText.height = pstFont->FontHeight;
    stText.width = pstFont->FontWidth * OSDLENGTH;
    stBitmap.u32Width = stText.width;
    stBitmap.u32Height = stText.height;
    stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;

    /*分配内存空间*/
    stText.pRGBBuffer = (HI_U8*)malloc(stText.width * stText.height * 2);
    if(NULL == stText.pRGBBuffer)
    {

    }
    memset(stText.pRGBBuffer, 0, stText.width * stText.height * 2);
    printf("start osd thread \n");

    char tmpmdn[13] = {0};
    //mdn只取4位
    if(strlen(mdn) > 4){
        int i = 0, index = 0;
        for(i=strlen(mdn) - 4; i<strlen(mdn); i++)
        {
            tmpmdn[index++] = mdn[i];
        }
        tmpmdn[index] = 0;
    }

    /*获取系统时间*/
    while(1)
    {
        TimeNow = time(&TimeNow);
        pstTimeNow = localtime(&TimeNow);
        sprintf(szString,
                "%34.34s %3dkm/h %.4s %04d-%02d-%02d %02d:%02d:%02d", "", nSpeed, tmpmdn,
//                "%3dkm/h %.4s %04d-%02d-%02d %02d:%02d:%02d", nSpeed, tmpmdn,
                pstTimeNow->tm_year + 1900,
                pstTimeNow->tm_mon + 1,
                pstTimeNow->tm_mday,
                (pstTimeNow->tm_hour) % 24,
                pstTimeNow->tm_min,
                pstTimeNow->tm_sec);
#if 0
        printf("time is %s, len=%d\n!!!!!!!!!!",szString, strlen(szString));
#endif

        /*将时间字符串转换为bmp图片*/
//        char testhz[]={0xd6, 0xd0, 0xb9, 0xfa, 0xbd, 0xf1, 0xca, 0xd3, 0xb5, 0xc0, 0xb5, 0xe7,0xd7, 0xd3, 0xbf, 0xc6, 0xbc, 0xbc};//今视道

       RetValue = Osd_Text2Bitmap(&stText, szString);

       VIDEO_HZ16TextOut(&stText,8,0,&szStation);


//       int i;
//       for(i=0; i<sizeof(szStation); i++)
//       {
//           printf("%d ", szStation[i]);
//       }
//       printf("\n");

//        VIDEO_HZ16TextOut

        if(HI_SUCCESS != RetValue)
        {
            continue;
        }
        stBitmap.pData = (HI_U8*)stText.pRGBBuffer;


        /*添加时间bmp到all channel OSD区域*/
        //for(Counter = 0; Counter < VMAX_CHANNEL_NUM; Counter++)
        {
           RetValue = HI_MPI_RGN_SetBitMap(OverlayExHandle,&stBitmap);
           if(RetValue != HI_SUCCESS)
           {
        	   printf("error !!!!!!!!!!");
           }

        }

        /*若没有要显示的time OSD则退出线程*/
        if(0 == g_ShowTimeThreadFlag)
        {
        	printf("break\n!!!!!!!!!");
            break;
        }
        sleep(1);
        //sleep(1);
    }
    free(stText.pRGBBuffer);
    stText.pRGBBuffer = NULL;
    printf("error !!!!!!!!!!");
}

HI_S32 Osd_Init(VI_CHN ViChn)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_U8 i;
    RGN_ATTR_S stOverlayExAttr;
	RGN_CHN_ATTR_S stOverlayExChnAttr[VMAX_CHANNEL_NUM];
	VI_DEV ViDev = 0;
	VIDEO_FONT *pstFont = NULL;
	g_ShowTimeThreadFlag=1;
	VIDEO_FontInit();
	pstFont = VIDEO_GetFont();
	stOverlayExAttr.enType = OVERLAYEX_RGN;
	stOverlayExAttr.unAttr.stOverlayEx.enPixelFmt =
			PIXEL_FORMAT_RGB_1555;
	stOverlayExAttr.unAttr.stOverlayEx.u32BgColor = 0x00ffffff;
	stOverlayExAttr.unAttr.stOverlayEx.stSize.u32Height =
			pstFont->FontHeight;
	stOverlayExAttr.unAttr.stOverlayEx.stSize.u32Width =
			pstFont->FontWidth * OSDLENGTH;
	//Create Rigion
	s32Ret = HI_MPI_RGN_Create(OverlayExHandle, &stOverlayExAttr); //region
	if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_RGN_Create failed with %#x!\n", s32Ret);
			return HI_FAILURE;
	}

	for (i = 0; i < VMAX_CHANNEL_NUM; i++) {

		stOverlayExChnAttr[i].enType = OVERLAYEX_RGN;
		stOverlayExChnAttr[i].bShow = HI_TRUE;
        stOverlayExChnAttr[i].unChnAttr.stOverlayExChn.stPoint.s32X = 160;//200;
		stOverlayExChnAttr[i].unChnAttr.stOverlayExChn.stPoint.s32Y = 10;
		stOverlayExChnAttr[i].unChnAttr.stOverlayExChn.u32BgAlpha = 50;
		stOverlayExChnAttr[i].unChnAttr.stOverlayExChn.u32FgAlpha = 130;
		stOverlayExChnAttr[i].unChnAttr.stOverlayExChn.u32Layer = 1;

		stOverlayExChn[i].enModId = HI_ID_VIU;
		stOverlayExChn[i].s32DevId = ViDev;
		stOverlayExChn[i].s32ChnId = i;

		s32Ret = HI_MPI_RGN_AttachToChn(OverlayExHandle, &stOverlayExChn[i],
				&stOverlayExChnAttr[i]); //channel
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_RGN_AttachToChn failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}


    //start thread

	 pthread_create(&osdtimethreadid, 0, Osd_TimeOut, NULL);
	return HI_SUCCESS;
}

HI_S32 Osd_Stop()
{
	HI_S32 s32Ret = HI_FAILURE;
    HI_U8 i;
    g_ShowTimeThreadFlag=0;
    pthread_join(osdtimethreadid,0);
	for (i = 0; i < VMAX_CHANNEL_NUM; i++) {
		s32Ret = HI_MPI_RGN_DetachFrmChn(OverlayExHandle, &stOverlayExChn[i]);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_RGN_DetachFrmChn (%d)) failed with %#x!\n",
					OverlayExHandle, s32Ret);
			return HI_FAILURE;
		}
		s32Ret = HI_MPI_RGN_Destroy(OverlayExHandle);
		if (HI_SUCCESS != s32Ret) {
			SAMPLE_PRT("HI_MPI_RGN_Destroy (%d)) failed with %#x!\n",
					OverlayExHandle, s32Ret);
			return HI_FAILURE;
		}
	}
	return HI_SUCCESS;

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


