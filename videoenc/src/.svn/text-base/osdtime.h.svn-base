/*
 * osdtime.h
 *
 *  Created on: Mar 17, 2014
 *      Author: root
 */

#ifndef OSDTIME_H_
#define OSDTIME_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "../common/sample_comm.h"
//4 video input channel
#define VMAX_CHANNEL_NUM 4



#define GET_R_COMPONENT(RGBColor)  (HI_U8)((RGBColor>>16)&0xff)
#define GET_G_COMPONENT(RGBColor)  (HI_U8)((RGBColor>>8)&0xff)
#define GET_B_COMPONENT(RGBColor)  (HI_U8)(RGBColor&0xff)

#define GET_Y_COMPONENT(R,G,B)   (HI_U8)(((R*66+G*129+B*25)/256)+16)
#define GET_Cb_COMPONENT(R,G,B)  (HI_U8)((((R*112-G*94)-B*18)/256)+128)
#define GET_Cr_COMPONENT(R,G,B)  (HI_U8)((((B*112-R*38)-G*74)/256)+128)
/* ============= font setting ============= */
#define MAX_FONT_NAME   16

typedef struct tag_VIDEO_FONT {
	char FontName[MAX_FONT_NAME];
	HI_U8 ** FontEntry;
	HI_U32 FontWidth;
	HI_U32 FontHeight;
} VIDEO_FONT;

/* global variables */

/* font class list-chain */
typedef struct tag_VIDEO_FONT_LIST {
	VIDEO_FONT VideoFont;
	struct tag_VIDEO_FONT_LIST * pNext;
} VIDEO_FONT_LIST;

/* for extend text and image bg show method */
typedef enum tag_VIDEO_BG_MODE_T {
	VIDEO_BG_MODE_TRANSPARENT = 0, VIDEO_BG_MODE_OPAQUE, /* not transparent */

	VIDEO_BG_MODE_BUTT
} VIDEO_BG_MODE_T;

typedef struct tag_VIDEO_Text {
	HI_U32 width;
	HI_U32 height;
	HI_U8 * pRGBBuffer;
} VIDEO_TEXT_T;

static VIDEO_FONT_LIST * g_pVideoFontList = NULL;
static VIDEO_FONT g_CurrentVideoFont = { "\0", NULL, 0, 0 };

/* ============= text setting ============= */
static HI_U8 g_VideoTextColor[3] = { 0xff, 0xff };
/* [0...2] = [B, G, R] */

static VIDEO_BG_MODE_T g_VideoTextBgMode = VIDEO_BG_MODE_OPAQUE;

static HI_U8 g_VideoTextBgColor[3] = { 0xff, 0 };
/* [0...2] = [Cr, Cb, Y] */

int VIDEO_RegisterFont(const char * FontName, const HI_U8 ** FontEntry,
		HI_U32 FontWidth, HI_U32 FontHeight);
int VIDEO_SetFont(const char * FontName);
int VIDEO_FontInit(void);
VIDEO_FONT * VIDEO_GetFont(void);
int VIDEO_TextOut(VIDEO_TEXT_T * pVText, HI_U32 ulXStart, HI_U32 ulYStart,
		const char* pString);
int VIDEO_HZ16TextOut(VIDEO_TEXT_T * pVText, HI_U32 ulXStart, HI_U32 ulYStart,
        const char* pString);
int VIDEO_SetTextColor(HI_U32 ulTextColor);
int VIDEO_SetTextBgMode(VIDEO_BG_MODE_T BgMode);
int VIDEO_SetTextBgColor(HI_U32 ulBgColor);
HI_S32 Osd_Init(VI_CHN ViChn);
void *Osd_TimeOut(void *args);
HI_S32 Osd_Stop();








#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* OSDTIME_H_ */
