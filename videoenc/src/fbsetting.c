/*
 * fbsetting.c
 *
 *  Created on: Mar 18, 2014
 *      Author: root
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "fbsetting.h"
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>   //mmap
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include "hi_common.h"
#include "hi_type.h"
#include "hi_comm_vb.h"
#include "hi_comm_sys.h"
#include "hi_comm_venc.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
//#include "hi_comm_group.h"

#include "hi_comm_region.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_venc.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_region.h"
#include "hifb.h"

//HI_BOOL Setup_FB(void *pData);


static struct fb_bitfield g_r32 = {16,8, 0};
static struct fb_bitfield g_g32 = {8, 8, 0};
static struct fb_bitfield g_b32 = {0, 8, 0};
static struct fb_bitfield g_a32 = {24,8, 0};

typedef struct HIFB_INFO_HIFB
{
    int fd;
    int layer;
    int ctrlkey;
}HIFB_INFO;

//HI_BOOL  Setup_FB(void *pData)
int  Setup_FB() {

	HIFB_INFO stInfo0;
    stInfo0.layer   =  0;
    stInfo0.fd      = -1;
    stInfo0.ctrlkey =  2;

	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;

	HI_U32 u32FixScreenStride = 0;
	unsigned char *pShowScreen;


	HIFB_ALPHA_S stAlpha;
	HIFB_POINT_S stPoint = { 40, 112 };

	char file[12] = "/dev/fb0";

	HI_BOOL g_bCompress = HI_FALSE;

	HI_BOOL bShow;
	HIFB_INFO *pstInfo;
	HIFB_COLORKEY_S stColorKey;

/*
	if (HI_NULL == pData)
	{
		return HI_FALSE;
	}


	pstInfo = (HIFB_INFO *) pData;*/
	pstInfo=&stInfo0;

	printf("pstInfo->layer is %d   \n", pstInfo->layer);

	switch (pstInfo->layer)
	{
	case 0:
		strcpy(file, "/dev/fb0");
		break;

	case 1:
		strcpy(file, "/dev/fb1");
		break;

	case 2:
		strcpy(file, "/dev/fb2");
		break;

	case 3:
		strcpy(file, "/dev/fb3");
		break;

	default:
		strcpy(file, "/dev/fb0");
		break;
	}

	/* 1. open framebuffer device overlay 0 */

	pstInfo->fd = open(file, O_RDWR, 0);

	if (pstInfo->fd < 0)
	{
		printf("open %s failed!\n", file);
		return HI_FALSE;
	}

	printf("open framebuffer device overlay 0!\n");

	if (pstInfo->layer == HIFB_LAYER_0)
	{
		if (ioctl(pstInfo->fd, FBIOPUT_COMPRESSION_HIFB, &g_bCompress) < 0)
		{
			printf("Func:%s line:%d FBIOPUT_COMPRESSION_HIFB failed!\n",
			__FUNCTION__, __LINE__);
			close(pstInfo->fd);
			 return HI_FALSE;
		}
	}

	bShow = HI_FALSE;

	if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
	{
		printf("FBIOPUT_SHOW_HIFB failed!\n");
		 return HI_FALSE;
	}

	/* 2. set the screen original position */

	switch (pstInfo->ctrlkey)
	{
	case 3:
	{
		stPoint.s32XPos = 150;
		stPoint.s32YPos = 150;
	}
		break;

	default:
	{
		stPoint.s32XPos = 0;
		stPoint.s32YPos = 0;
	}

	}

	printf(" FBIOPUT_SCREEN_ORIGIN_HIFB 0!\n");
	if (ioctl(pstInfo->fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
	{
		printf("set screen original show position failed!\n");
		close(pstInfo->fd);
		return HI_FALSE;
	}

	/* 3.set alpha */

	stAlpha.bAlphaEnable = HI_FALSE;
	stAlpha.bAlphaChannel = HI_FALSE;

	stAlpha.u8Alpha0 = 0x0;
	stAlpha.u8Alpha1 = 0xff;
	stAlpha.u8GlobalAlpha = 0x80;

	printf(" FBIOPUT_ALPHA_HIFB 0!\n");

	if (ioctl(pstInfo->fd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
	{
		printf("Set alpha failed!\n");
		close(pstInfo->fd);
		 return HI_FALSE;
	}

	/*all layer surport colorkey*/

	stColorKey.bKeyEnable = HI_TRUE;
	stColorKey.u32Key = 0x0; //

	//stColorKey.u32Key =0x0000ff00;//zly2014-0219

	if (ioctl(pstInfo->fd, FBIOPUT_COLORKEY_HIFB, &stColorKey) < 0)
	{

		printf("FBIOPUT_COLORKEY_HIFB!\n");
		close(pstInfo->fd);
		 return HI_FALSE;

	}

	/* 4. get the variable screen info */

	printf(" get the variable screen info!\n");

	if (ioctl(pstInfo->fd, FBIOGET_VSCREENINFO, &var) < 0)
	{
		printf("Get variable screen info failed!\n");
		close(pstInfo->fd);
		 return HI_FALSE;
	}


	usleep(4  * 1000);

	switch (pstInfo->ctrlkey)
	{
	case 3:
	{
		var.xres_virtual = 48;
		var.yres_virtual = 48;
		var.xres = 48;
		var.yres = 48;
	}
		break;

	default:
	{
		var.xres_virtual = 1024;
		var.yres_virtual = 768 * 2;
		var.xres = 1024;
		var.yres = 768;
	}

	}

	var.transp = g_a32;
	var.red = g_r32;
	var.green = g_g32;
	var.blue = g_b32;
	var.bits_per_pixel = 32;

	var.activate = FB_ACTIVATE_NOW;

	/* 6. set the variable screeninfo */

	if (ioctl(pstInfo->fd, FBIOPUT_VSCREENINFO, &var) < 0)
	{
		printf("Put variable screen info failed!\n");
		close(pstInfo->fd);
		 return HI_FALSE;
	}

	/* 7. get the fix screen info */

	if (ioctl(pstInfo->fd, FBIOGET_FSCREENINFO, &fix) < 0)
	{
		printf("Get fix screen info failed!\n");
		close(pstInfo->fd);
		 return HI_FALSE;;
	}

	u32FixScreenStride = fix.line_length; /*fix screen stride*/

	printf(" 8. map the physical video memory for user use!\n");

	/* 8. map the physical video memory for user use */

	pShowScreen = mmap(HI_NULL, fix.smem_len, PROT_READ | PROT_WRITE,
			MAP_SHARED, pstInfo->fd, 0);

	if (MAP_FAILED == pShowScreen)
	{
		printf("mmap framebuffer failed!\n");
		close(pstInfo->fd);
		 return HI_FALSE;
	}

	memset(pShowScreen, 0x00, fix.smem_len);

	/* time to paly*/

	bShow = HI_TRUE;

	if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
	{
		printf("FBIOPUT_SHOW_HIFB failed!\n");
		munmap(pShowScreen, fix.smem_len);
		 return HI_FALSE;
	}
	return HI_TRUE;
}

int Stop_FB(void *pData){
	return HI_TRUE;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

