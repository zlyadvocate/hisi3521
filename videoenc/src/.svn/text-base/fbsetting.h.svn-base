/*
 * fbsetting.h
 *
 *  Created on: Mar 18, 2014
 *      Author: root
 */

#ifndef FBSETTING_H_
#define FBSETTING_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define DIF_LAYER_NAME_LEN 20
#define HIL_MMZ_NAME_LEN 32
#define HIFB_RED_1555   0xfc00
#define SAMPLE_VIR_SCREEN_WIDTH	    SAMPLE_IMAGE_WIDTH			/*virtual screen width*/
#define SAMPLE_VIR_SCREEN_HEIGHT	SAMPLE_IMAGE_HEIGHT*2		/*virtual screen height*/

#define HIL_MMB_NAME_LEN 16


typedef enum
{
    HIFB_LAYER_0 = 0x0,
    HIFB_LAYER_1,
    HIFB_LAYER_2,
    HIFB_LAYER_CURSOR_0,
    HIFB_LAYER_ID_BUTT

} HIFB_LAYER_ID_E;




int  Setup_FB();
int Stop_FB(void *pData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* FBSETTING_H_ */
