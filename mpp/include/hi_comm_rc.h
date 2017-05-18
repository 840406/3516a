/******************************************************************************

  Copyright (C), 2001-2012, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_rc.h
  Version       : 
  Author        : Hisilicon Hi35xx MPP Team
  Created       : 2006/11/24
  Last Modified :
  Description   : common struct definition for Rate control
  Function List :
  History       :
******************************************************************************/
#ifndef __HI_COMM_RC_H__
#define __HI_COMM_RC_H__

#include "hi_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef HI_U32 HI_FR32;

typedef enum hiVENC_RC_MODE_E
{
    VENC_RC_MODE_H264CBR = 1,    
    VENC_RC_MODE_H264VBR, 
    VENC_RC_MODE_H264AVBR,        
    VENC_RC_MODE_H264ABR,        
    VENC_RC_MODE_H264FIXQP,      

    VENC_RC_MODE_MJPEGCBR,    
    VENC_RC_MODE_MJPEGVBR,        
    VENC_RC_MODE_MJPEGABR,        
    VENC_RC_MODE_MJPEGFIXQP,  

    VENC_RC_MODE_MPEG4CBR,    
    VENC_RC_MODE_MPEG4VBR,        
    VENC_RC_MODE_MPEG4ABR,        
    VENC_RC_MODE_MPEG4FIXQP,  

    VENC_RC_MODE_H265CBR,    
    VENC_RC_MODE_H265VBR, 
    VENC_RC_MODE_H265AVBR,
    VENC_RC_MODE_H265FIXQP,    

    
    VENC_RC_MODE_BUTT,

}VENC_RC_MODE_E;


typedef struct hiVENC_ATTR_H264_FIXQP_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */     
    HI_U32      u32IQp;                                 /* qp of the i frame */
    HI_U32      u32PQp;                                 /* qp of the p frame */
} VENC_ATTR_H264_FIXQP_S;

typedef struct hiVENC_ATTR_H264_CBR_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */ 
    HI_U32      u32BitRate;                             /* average bitrate */
    HI_U32      u32FluctuateLevel;                      /* level [0..5].scope of bitrate fluctuate. 1-5: 10%-50%. 0: SDK optimized, recommended; */
} VENC_ATTR_H264_CBR_S;                                         
                                         
typedef struct hiVENC_ATTR_H264_VBR_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */     
    HI_U32      u32MaxBitRate;                          /* the max bitrate */                      
    HI_U32      u32MaxQp;                               /* the max qp */
    HI_U32      u32MinQp;                               /* the min qp */
}VENC_ATTR_H264_VBR_S;

typedef struct hiVENC_ATTR_H264_AVBR_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */     
    HI_U32      u32MaxBitRate;                          /* the max bitrate */                      
}VENC_ATTR_H264_AVBR_S;

typedef struct hiVENC_ATTR_H264_ABR_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */    
    HI_U32      u32AvgBitRate;                          /* average bitrate */        
    HI_U32      u32MaxBitRate;                          /* the max bitrate */
}VENC_ATTR_H264_ABR_S;

typedef struct hiVENC_ATTR_H264_CBR_S   VENC_ATTR_H265_CBR_S;
typedef struct hiVENC_ATTR_H264_VBR_S   VENC_ATTR_H265_VBR_S;
typedef struct hiVENC_ATTR_H264_AVBR_S   VENC_ATTR_H265_AVBR_S;
typedef struct hiVENC_ATTR_H264_FIXQP_S VENC_ATTR_H265_FIXQP_S;



typedef struct hiVENC_ATTR_MPEG4_FIXQP_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */     
    HI_U32      u32IQp;                                 /* qp of the i frame */
    HI_U32      u32PQp;                                 /* qp of the p frame */
}VENC_ATTR_MPEG4_FIXQP_S;

typedef struct hiVENC_ATTR_MPEG4_CBR_S
{
    HI_U32      u32Gop;                                 /*the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */ 
    HI_U32      u32BitRate;                             /* average bitrate */
    HI_U32      u32FluctuateLevel;                      /* level [0..5].scope of bitrate fluctuate. 1-5: 10%-50%. 0: SDK optimized, recommended; */
}VENC_ATTR_MPEG4_CBR_S;

typedef struct hiVENC_ATTR_MPEG4_VBR_S
{
    HI_U32      u32Gop;                                 /* the interval of ISLICE. */
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */     
    HI_U32      u32MaxBitRate;                          /* the max bitrate */                      
    HI_U32      u32MaxQp;                               /* the max qp */
    HI_U32      u32MinQp;                               /* the min qp */
}VENC_ATTR_MPEG4_VBR_S;
 
typedef struct hiVENC_ATTR_MJPEG_FIXQP_S
{
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate;                         /* the target frame rate of the venc chnnel */
    HI_U32      u32Qfactor;                             /* image quality :[1,99]*/
}VENC_ATTR_MJPEG_FIXQP_S;

typedef struct hiVENC_ATTR_MJPEG_CBR_S
{
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate ;                        /* the target frame rate of the venc chnnel */
    HI_U32      u32BitRate;                             /* average bitrate */
    HI_U32      u32FluctuateLevel;                      /* reserved, level [1..5].scope of bitrate fluctuate. 1-5: 10%-50%. 1: SDK optimized, recommended; */
} VENC_ATTR_MJPEG_CBR_S;

typedef struct hiVENC_ATTR_MJPEG_VBR_S
{
    HI_U32      u32StatTime;                            /* the rate statistic time, the unit is senconds(s) */
    HI_U32      u32SrcFrmRate;                          /* the input frame rate of the venc chnnel */
    HI_FR32     fr32DstFrmRate;                         /* the target frame rate of the venc chnnel */
    HI_U32      u32MaxBitRate;                          /* max bitrate */
    HI_U32      u32MaxQfactor;                          /* max image quailty allowed */
    HI_U32      u32MinQfactor;                          /* min image quality allowed */
}VENC_ATTR_MJPEG_VBR_S;

typedef struct hiVENC_RC_ATTR_S
{
    VENC_RC_MODE_E enRcMode;                            /*the type of rc*/
    union
    {
        VENC_ATTR_H264_CBR_S    stAttrH264Cbr;
        VENC_ATTR_H264_VBR_S    stAttrH264Vbr;
		VENC_ATTR_H264_AVBR_S   stAttrH264AVbr;
        VENC_ATTR_H264_FIXQP_S  stAttrH264FixQp;
        VENC_ATTR_H264_ABR_S    stAttrH264Abr;

        VENC_ATTR_MPEG4_CBR_S   stAttrMpeg4Cbr;
        VENC_ATTR_MPEG4_FIXQP_S stAttrMpeg4FixQp;
        VENC_ATTR_MPEG4_VBR_S   stAttrMpeg4Vbr;

        VENC_ATTR_MJPEG_CBR_S   stAttrMjpegeCbr;
        VENC_ATTR_MJPEG_FIXQP_S stAttrMjpegeFixQp;
        VENC_ATTR_MJPEG_VBR_S   stAttrMjpegeVbr;

        VENC_ATTR_H265_CBR_S    stAttrH265Cbr;
        VENC_ATTR_H265_VBR_S    stAttrH265Vbr;
        VENC_ATTR_H265_AVBR_S   stAttrH265AVbr;
		VENC_ATTR_H265_FIXQP_S  stAttrH265FixQp;
    };   
    HI_VOID*       pRcAttr ;        /*the rc attribute which could be specified by user*/
 
}VENC_RC_ATTR_S;

typedef enum hiRC_SUPERFRM_MODE_E
{
    SUPERFRM_NONE=0,                /* sdk don't care super frame */
    SUPERFRM_DISCARD,               /* the super frame is discarded */
    SUPERFRM_REENCODE,              /* the super frame is re-encode */
    SUPERFRM_BUTT
}VENC_SUPERFRM_MODE_E;

typedef struct hiVENC_PARAM_H264_CBR_S
{
    HI_U32  u32MinIprop;            /* the min ratio of i frame and p frame */           
    HI_U32  u32MaxIprop;            /* the max ratio of i frame and p frame */
    HI_U32  u32MaxQp;               /* the max QP value */
    HI_U32  u32MinQp;               /* the min QP value */
    HI_S32  s32IPQPDelta;           /* the qp difference between the i frame and the before gop avarage qp; == Qp(P) - Qp(I) */
    HI_S32  s32QualityLevel;        /* quality of picture [1, 5] */
    HI_S32  s32MaxReEncodeTimes;    /* max number of re-encode times [0, 3]*/ 
    HI_U32  u32MinIQp;              /* min qp for I frame */
    HI_U32  u32MaxIQp;              /* max qp for I frame */
}VENC_PARAM_H264_CBR_S;

typedef struct hiVENC_PARAM_H264_VBR_S
{
    HI_S32 s32IPQPDelta;            /* the qp difference between the i frame and the before gop avarage qp; == Qp(P) - Qp(I) */
    HI_S32 s32ChangePos;            /* Indicates the ratio of the current bit rate to the maximum 
                                        bit rate when the QP value starts to be adjusted */
    HI_U32  u32MinIprop;            /* the min ratio of i frame and p frame */           
    HI_U32  u32MaxIprop;            /* the max ratio of i frame and p frame */                                          
    HI_U32  u32MinIQP;              /* min qp for i frame */
}VENC_PARAM_H264_VBR_S;

typedef struct hiVENC_PARAM_H264_AVBR_S
{
    HI_S32  s32IPQPDelta;           /* the qp difference between I frame and P frame; s32IPQPDelta == Qp(P) - Qp(I) */
    HI_S32  s32ChangePos;           /* when bitrate >= (maxbitrate * s32ChangePos / 100), the QP value starts to be adjusted*/
    HI_U32  u32MinIprop;            /* the min ratio of i frame and p frame */
    HI_U32  u32MaxIprop;            /* the max ratio of i frame and p frame */

    HI_U32  u32MinIQp;              /* the min QP value of I frame */
    HI_U32  u32MaxIQp;              /* the max QP value of I frame */
    HI_U32  u32MinQp;              /* the min QP value of  frame */
    HI_U32  u32MaxQp;              /* the max QP value of  frame */
    
    HI_S32  s32MinStillPercent;     /* the min percent of target bitrate for still scene */
    HI_U32  u32MaxStillQP;          /* the max QP value of I frame for still scene*/
    HI_U32  u32MinStillPSNR;        /* reserved */
    HI_S32  s32MaxReEncodeTimes;    /* max number of re-encode times [0, 3]*/ 
} VENC_PARAM_H264_AVBR_S;

typedef struct hiVENC_PARAM_MJPEG_CBR_S
{
    HI_U32  u32MaxQfactor;                              /* the max Qfactor value*/
    HI_U32  u32MinQfactor;                              /* the min Qfactor value */    
    HI_U32  u32RQRatio[RC_RQRATIO_SIZE];                              /* the rate stabilization weight, 
                                                            100-u32RQRatio[i] is the sequence quality stabilization weight */ 

}VENC_PARAM_MJPEG_CBR_S;

typedef struct hiVENC_PARAM_MJPEG_VBR_S
{
    HI_S32 s32DeltaQfactor;                             /* Indicates the maximum change of Qfactor values of frames 
                                                            when the picture quality changes */
    HI_S32 s32ChangePos;                                /* Indicates the ratio of the current bit rate to the maximum 
                                                            bit rate when the Qfactor value starts to be adjusted */                                               
}VENC_PARAM_MJPEG_VBR_S;

typedef struct hiVENC_PARAM_MPEG4_CBR_S
{
    HI_U32  u32MinIprop;                                /* the min ratio of i frame and p frame*/           
    HI_U32  u32MaxIprop;                                /* the max ratio of i frame and p frame */
     
    HI_U32  u32MaxQp;                                   /* the max QP value*/
    HI_U32  u32MinQp;                                   /* the min QP value */
    HI_U32  u32MaxPPDeltaQp;                            /* the max qp value difference between two successive P frame */
    HI_U32  u32MaxIPDeltaQp;                            /* the max qp value difference between p frame and the next i frame */    
    HI_S32 s32IPQPDelta;                                /* the qp difference between the i frame and the before gop avarage qp*/

    HI_U32  u32RQRatio[RC_RQRATIO_SIZE];                              /* the rate stabilization weight, 
                                                            100-u32RQRatio[i] is the sequence quality stabilization weight */
}VENC_PARAM_MPEG4_CBR_S;

typedef struct hiVENC_PARAM_MPEG4_VBR_S
{
    HI_S32 s32IPQPDelta;                                  /* the qp difference between the i frame and the before gop avarage qp*/
    
    HI_S32 s32ChangePos;                                /* Indicates the ratio of the current bit rate to the maximum 
                                                            bit rate when the QP value starts to be adjusted */

    HI_U32  u32MinIprop;                                /* the min ratio of i frame and p frame */           
    HI_U32  u32MaxIprop;                                /* the max ratio of i frame and p frame */                                               
}VENC_PARAM_MPEG4_VBR_S;

typedef struct hiVENC_PARAM_H264_CBR_S   VENC_PARAM_H265_CBR_S; 
typedef struct hiVENC_PARAM_H264_VBR_S   VENC_PARAM_H265_VBR_S; 
typedef struct hiVENC_PARAM_H264_AVBR_S  VENC_PARAM_H265_AVBR_S; 

typedef struct hiVENC_RC_PARAM_S
{     
    HI_U32 u32ThrdI[RC_TEXTURE_THR_SIZE];                     /* just useful for h264/h265 and mpeg4 for now */
    HI_U32 u32ThrdP[RC_TEXTURE_THR_SIZE];
    HI_U32 u32RowQpDelta;
    HI_S32 s32FirstFrameStartQp;
    union
    {
        VENC_PARAM_H264_CBR_S     stParamH264Cbr;
        VENC_PARAM_H264_VBR_S     stParamH264VBR;
		VENC_PARAM_H264_AVBR_S    stParamH264AVbr;
        VENC_PARAM_MJPEG_CBR_S    stParamMjpegCbr;
        VENC_PARAM_MJPEG_VBR_S    stParamMjpegVbr;
        VENC_PARAM_MPEG4_CBR_S    stParamMpeg4Cbr;
        VENC_PARAM_MPEG4_VBR_S    stParamMpeg4Vbr;
        VENC_PARAM_H265_CBR_S     stParamH265Cbr;
        VENC_PARAM_H265_VBR_S     stParamH265Vbr; 
		VENC_PARAM_H265_AVBR_S    stParamH265AVbr;       
    };

    HI_VOID* pRcParam;                      /*RC parameter which could be specified by usrer*/
}VENC_RC_PARAM_S;



typedef enum hiVENC_FRAMELOST_MODE_E
{   
    FRMLOST_NORMAL=0,                                /*normal mode*/         
    FRMLOST_PSKIP,                                  /*pskip*/ 
    FRMLOST_BUTT,
}VENC_FRAMELOST_MODE_E;

typedef struct hiVENC_PARAM_FRAMELOST_S
{ 
    HI_BOOL bFrmLostOpen;                           /* Indicates whether to discard frames to ensure 
                                                            stable bit rate when the instant bit rate is exceeded */
    HI_U32  u32FrmLostBpsThr;                       /* the instant bit rate threshold */
    VENC_FRAMELOST_MODE_E enFrmLostMode;            /*frame lost strategy*/
    HI_U32  u32EncFrmGaps;                          /*the gap of frame lost*/
}VENC_PARAM_FRAMELOST_S;


typedef struct hiVENC_SUPERFRAME_CFG_S
{ 
      VENC_SUPERFRM_MODE_E  enSuperFrmMode;               /* Indicates the mode of processing the super frame,[SUPERFRM_NONE,SUPERFRM_DISCARD,SUPERFRM_REENCODE]*/
      HI_U32  u32SuperIFrmBitsThr;                        /* Indicate the threshold of the super I frame 
                                                               for enabling the super frame processing mode */              
      HI_U32  u32SuperPFrmBitsThr;                        /* Indicate the threshold of the super P frame */                                                     
      HI_U32  u32SuperBFrmBitsThr;                        /* Indicate the threshold of the super B frame */                                                   
}VENC_SUPERFRAME_CFG_S;

typedef enum hiVENC_RC_PRIORITY_E
{
    VENC_RC_PRIORITY_BITRATE_FIRST = 1,    
    VENC_RC_PRIORITY_FRAMEBITS_FIRST,

    VENC_RC_PRIORITY_BUTT,
} VENC_RC_PRIORITY_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_COMM_RC_H__ */
