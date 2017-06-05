/*******************************************************************
* °æÈ¨ËùÓĞ (C) 2017, ±±¾©ÊÓÁª¶¯Á¦¿Æ¼¼·¢Õ¹ÓĞÏŞ¹«Ë¾
* ÎÄ¼şÃû³Æ£º my_sample_venc.c
* ÄÚÈİÕªÒª£º 4k²úÆ·3516a¶Ë²É¼¯demo
* ÆäËüËµÃ÷£º ÎŞ
******************************************************************/

#include "common.h"
//#include "sample_comm.h"


#define  SERVERPORT     8809
#define  CHNUM		5


typedef struct my_sample_venc_getstream_s
{
    HI_BOOL bThreadStart;    // ÊÇ·ñ¿ªÆôÏß³Ì
    HI_S32  s32Cnt;			//Í¨µÀÊı	
	s32 s32HDVideoDataFd;	//	¸ßÇåÍ¨µÀµÄsocketÎÄ¼şÃèÊö·û
	s32 s32SDVideoDataFd;	//	±êÇåÍ¨µÀµÄsocketÎÄ¼şÃèÊö·
} my_SAMPLE_VENC_GETSTREAM_PARA_S;


Args stArgs;
my_SAMPLE_VENC_GETSTREAM_PARA_S my_gs_stPara;
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
s32 g_StopVideoThread = 0;
s32 g_CmdFd = 0;
pthread_t g_VideoThread_t;
pthread_t g_VencPid;


/**********************************************************************
* º¯ÊıÃû³Æ£º CreatePthread
* ¹¦ÄÜÃèÊö£º´´½¨Ïß³Ìº¯Êı
* ÊäÈë²ÎÊı£º SFun    :  void* (*Fun)(void*) ĞÍ, Ïß³Ìº¯ÊıÃû
				args	    :  	´«µİ¸ø´ËÏß³ÌµÄ²ÎÊı
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»ØÏß³ÌID£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º ÎŞ
***********************************************************************/
pthread_t CreatePthread(Fun SFun, void* args)
{
	int iError;
	pthread_attr_t tAttr;
	struct sched_param	tSchedParam;

	if ((iError = pthread_attr_init (&tAttr)) != 0) 
	{
		fprintf (stderr, "pthread_tAttr_init: %s\n",	strerror (iError));
		return -1;
	}

	if ((iError = pthread_attr_setdetachstate (&tAttr,	PTHREAD_CREATE_DETACHED)) != 0) 
	{
		fprintf (stderr, "pthread_tAttr_setdetachstate: %s\n",	strerror (iError));
		return -1;
	}
#if 0
	if (pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED)) {
		fprintf (stderr, "Failed to set schedule inheritance tAttribute: %s\n",
			strerror (iError));
		return -1;
	}

	/* Set the thread to be fifo real time scheduled */
	if (pthread_attr_setschedpolicy(&tAttr, SCHED_FIFO)) {
		fprintf (stderr, "Failed to set FIFO scheduling policy: %s\n",
			strerror (iError));
		return -1;
	}
		
	/* Create the uart threads */
	tSchedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (pthread_attr_setschedparam(&tAttr, &tSchedParam))
	{
		fprintf (stderr, "Failed to set scheduler parameters: %s\n",
			strerror (iError));
		return -1;
	}
#endif
	pthread_t tTid;
	if ((iError = pthread_create (&tTid, &tAttr, SFun, args)) != 0) 
	{
		fprintf (stderr, "pthread_create: %s\n", strerror (iError));
		return -1;
	}

	if ((iError = pthread_attr_destroy (&tAttr)) != 0) 
	{
		fprintf (stderr, "pthread_tAttr_destroy: %s\n", strerror (iError));
		return -1;
	}

	return tTid;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º CreateSocket
* ¹¦ÄÜÃèÊö£º ´´½¨TCPµÄsocketÃèÊö·û
* ÊäÈë²ÎÊı£º port	:  ¶Ë¿ÚºÅ
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»ØsocketÃèÊö·û£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º ÎŞ
***********************************************************************/
s32  CreateSocket(unsigned int port)
{
	int sockfd = socket (AF_INET, SOCK_STREAM, 0);	
	if (sockfd == -1)
	{
		dbg_perror ("socket");	
		return -1;	
	}
	
	int on = 1;	
	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) == -1) 
	{		
		dbg_perror ("setsockopt");		
		return -1;
	}	
	
	struct sockaddr_in addr;	
	addr.sin_family = AF_INET;	
	addr.sin_port = htons (port);	
	addr.sin_addr.s_addr = INADDR_ANY;	
	if (bind (sockfd, (struct sockaddr*)&addr, sizeof (addr)) == -1) 
	{		
		dbg_perror ("bind");		
		return -1;	
	}	
		
	if (listen (sockfd, 1024) == -1) 
	{		
		dbg_perror ("listen");		
		return -1;
	}

	return sockfd;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º SendStream
* ¹¦ÄÜÃèÊö£º Ïò3536 ·¢ËÍ±àÂëºóµÄÊÓÆµÁ÷	
* ÊäÈë²ÎÊı£º s32VideoDataFd	:  ¸ßÇå»ò±êÇåµÄsoecketÃèÊö·û
 				pstStream		:  º£Ë¼µÄÊÓÆµÁ÷½á¹¹ÌåÖ¸Õë
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»Ø0£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º ÎŞ
***********************************************************************/
s32 SendStream(s32 s32VideoDataFd,  VENC_STREAM_S* pstStream)
{
	s32 i;
	s32 s32Ret;
	for (i = 0; i < pstStream->u32PackCount; i++)
	{
		s32Ret = send(s32VideoDataFd, pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset, \
									pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 0);
		if(-1 == s32Ret)
		{
			dbg_perror("sned");
			return -1;
		}
	}
	//printf("k7777777\n");
	return 0;
}

#if 0
void* TestThread(void* args)
{
	Args* pstArg = (Args*)args;
	//s32 fd = pstArg->s32HDVideoDataFd;
	s32 s32Ret =0 ;
	s8 as8Buf1[100] = {0};
	s8 as8Buf2[100] = {0};

	sprintf(as8Buf1, "this is HDThread == %d", stArgs.s32HDVideoDataFd);
	sprintf(as8Buf2, "this is SDThread 23== %d", stArgs.s32SDVideoDataFd);
	dbg_printf(DBG_INFO, "555-%s\n", as8Buf1);
	dbg_printf(DBG_INFO, "555-%s\n", as8Buf2);

	while(1)
	{			
		
		sleep(1);
		s32Ret = send(stArgs.s32SDVideoDataFd, as8Buf2, strlen(as8Buf2), 0);
		if(s32Ret < 0)
		{
			dbg_perror("send");
		}
		printf("-=-=-=%d\n", s32Ret);	

		sleep(1);
		s32Ret = send(stArgs.s32HDVideoDataFd, as8Buf1, strlen(as8Buf1), 0);
		if(s32Ret < 0)
		{
			dbg_perror("send");
		}
		printf("===%d\n", s32Ret);	
	}
}
#endif
/**********************************************************************
* º¯ÊıÃû³Æ£º my_SAMPLE_COMM_VENC_StopGetStream
* ¹¦ÄÜÃèÊö£º Í£Ö¹my_SAMPLE_COMM_VENC_GetVencStreamProc Ïß³Ì	
* ÊäÈë²ÎÊı£º ÎŞ
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»ØHI_SUCCESS£¬´íÎó·µ»ØHI_FALSE
* ÆäËüËµÃ÷£º ÎŞ
***********************************************************************/
HI_S32 my_SAMPLE_COMM_VENC_StopGetStream()
{
    if (HI_TRUE == my_gs_stPara.bThreadStart)
    {
        my_gs_stPara.bThreadStart = HI_FALSE;
       pthread_join(g_VencPid, 0);
    }
    return HI_SUCCESS;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º my_SAMPLE_COMM_VENC_GetVencStreamProc
* ¹¦ÄÜÃèÊö£º »ñÈ¡±àÂëºóµÄÊÓÆµÁ÷£¬²¢·¢ËÍ¸ø3536µÄÏß³Ì	
* ÊäÈë²ÎÊı£º p	:  ´«µİ¸ø´ËÏß³ÌµÄ²ÎÊı
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ·µ»ØNULL
* ÆäËüËµÃ÷£º ²Î¿¼º£Ë¼demoÖĞµÄSAMPLE_COMM_VENC_GetVencStreamProc() º¯Êı
***********************************************************************/
/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* my_SAMPLE_COMM_VENC_GetVencStreamProc(HI_VOID* p)
{
  HI_S32 i;
    HI_S32 s32ChnTotal;
	//s32 s32HDVideoDataFd;
	//s32 s32SDVideoDataFd;
    my_SAMPLE_VENC_GETSTREAM_PARA_S* pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
 s32 as32VideoDataFd[VENC_MAX_CHN_NUM];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;

    pstPara = (my_SAMPLE_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;
	as32VideoDataFd[0] = pstPara->s32HDVideoDataFd;
	as32VideoDataFd[1] = pstPara->s32SDVideoDataFd;

    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM)
    {
        SAMPLE_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++)
    {	
        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                       VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == pstPara->bThreadStart)
    {
          FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
             for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }
					
			/*******************************************************
			 step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
			 if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
			 {
				SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
				continue;
			 }
			*******************************************************/
			if(0 == stStat.u32CurPacks)
			{
				  SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
				  continue;
			}
                    /*******************************************************
                     step 2.3 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    /*******************************************************
                     step 2.4 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                                   s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.5 : send stream
                    *******************************************************/
               s32Ret = SendStream(as32VideoDataFd[i], &stStream);
		  if (HI_SUCCESS != s32Ret)
                   {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("send stream failed!\n");
		  }
		 
                    /*******************************************************
                     step 2.6 : release stream
                    *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
					
                    /*******************************************************
                     step 2.7 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }

    /*******************************************************
    * step 3 : return 
    *******************************************************/
	for(i=0; i<s32ChnTotal; i++)
	{
		close(as32VideoDataFd[i]);
	}

	printf("quit my_SAMPLE_COMM_VENC_GetVencStreamProc !!!\n");
    return NULL;
}


/**********************************************************************
* º¯ÊıÃû³Æ£º my_SAMPLE_VENC_1080P_CLASSIC
* ¹¦ÄÜÃèÊö£º ²É¼¯±àÂëH264 Ïß³Ì	
* ÊäÈë²ÎÊı£º pArgs	:  ´«µİ¸ø´ËÏß³ÌµÄ²ÎÊı
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ·µ»ØNULL
* ÆäËüËµÃ÷£º ²Î¿¼º£Ë¼demoÖĞµÄSAMPLE_VENC_1080P_CLASSIC() º¯Êı
***********************************************************************/
/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
void* my_SAMPLE_VENC_1080P_CLASSIC(void* pArgs)
{
	Args* pstThreadArg = (Args*)pArgs;
	
	g_StopVideoThread = 0;
	PAYLOAD_TYPE_E enHDPayLoad = PT_H264;
	PAYLOAD_TYPE_E enSDPayLoad = PT_H264;

	PIC_SIZE_E enHDSize = PIC_HD1080; //pstThreadArg->enHDPicSize;
	PIC_SIZE_E enSDSize = PIC_D1;  //pstThreadArg->enSDPicSize;

    HI_U32 u32Profile = 0;

    VB_CONF_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig = {0};

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;

    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
						
    HI_S32 s32ChnNum = pstThreadArg->s32VideoChNum;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stHDSize;
	SIZE_S stSDSize;
  
	SAMPLE_VI_MODE_E SENSOR_TYPE = SAMPLE_VI_MODE_BT1120_1080P; 

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

  
    stVbConf.u32MaxPoolCnt = 128;


    /*video buffer*/
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enHDSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 20;

	
	
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSDSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 20;


    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1080P_CLASSIC_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    stViConfig.enViMode   = SENSOR_TYPE;
    stViConfig.enRotate   = ROTATE_NONE;
    stViConfig.enNorm     = VIDEO_ENCODING_MODE_AUTO;
    stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
    stViConfig.enWDRMode  = WDR_MODE_NONE;
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

	system(" himm 0x20580100   0x80000000");//by cuibo   /*ÍâÍ¬²½*/

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enHDSize, &stHDSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }

    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = stHDSize.u32Width;
    stVpssGrpAttr.u32MaxH = stHDSize.u32Height;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_2;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1080P_CLASSIC_3;
    }

    VpssChn = 0;
    stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
    stVpssChnMode.bDouble        = HI_FALSE;
    stVpssChnMode.enPixelFormat  = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.u32Width       = stHDSize.u32Width;
    stVpssChnMode.u32Height      = stHDSize.u32Height;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_SEG;
    memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnAttr.s32DstFrameRate = -1;
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Enable vpss chn failed!\n");
        goto END_VENC_1080P_CLASSIC_4;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSDSize, &stSDSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1080P_CLASSIC_1;
    }
	
	VpssChn = 1;  
        stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
        stVpssChnMode.bDouble		= HI_FALSE;
        stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
        stVpssChnMode.u32Width		= stSDSize.u32Width;
        stVpssChnMode.u32Height 	=  	stSDSize.u32Height;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        stVpssChnAttr.s32SrcFrameRate = -1;
        stVpssChnAttr.s32DstFrameRate = -1;

        s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Enable vpss chn failed!\n");
            goto END_VENC_1080P_CLASSIC_4;
        }
	  
	
    /******************************************
     step 5: start stream venc
    ******************************************/

	/*** HD1080P **/
    VpssGrp = 0;
    VpssChn = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enHDPayLoad, \
                                    gs_enNorm, enHDSize, enRcMode, u32Profile);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

	
    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1080P_CLASSIC_5;
    }

 	   /*** D1 **/
	VpssChn = 1;	
	VencChn = 1;	
	s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enSDPayLoad, \
	                                gs_enNorm, enSDSize, enRcMode, u32Profile);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Start Venc failed!\n");
	    goto END_VENC_1080P_CLASSIC_5;
	}
	s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Start Venc failed!\n");
	    goto END_VENC_1080P_CLASSIC_5;
	}
  
	
    /******************************************
     step 6: stream venc process -- get stream, then save it to file.
    ******************************************/
	my_gs_stPara.bThreadStart = HI_TRUE;
	my_gs_stPara.s32Cnt = s32ChnNum;
	my_gs_stPara.s32HDVideoDataFd = pstThreadArg->s32HDVideoDataFd;
	my_gs_stPara.s32SDVideoDataFd = pstThreadArg->s32SDVideoDataFd;
	pthread_create(&g_VencPid, 0, my_SAMPLE_COMM_VENC_GetVencStreamProc, (HI_VOID*)&my_gs_stPara);


   while(0 == g_StopVideoThread)
   {
	usleep(2000);	
   }

    /******************************************
     step 7: exit process
    ******************************************/	
    my_SAMPLE_COMM_VENC_StopGetStream();
	
END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);

	
END_VENC_1080P_CLASSIC_3:    //vpss stop
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_2:    //vpss stop
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_VENC_1080P_CLASSIC_1:	//vi stop
    SAMPLE_COMM_VI_StopVi(&stViConfig);
END_VENC_1080P_CLASSIC_0:	//system exit
    SAMPLE_COMM_SYS_Exit();

    return NULL;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º ParamAndExcuteCmd
* ¹¦ÄÜÃèÊö£º ·ÖÎöÃüÁî²¢ÇÒÖ´ĞĞÃüÁî		
* ÊäÈë²ÎÊı£º s32CmdFd 	:  ÃüÁîÍ¨µÀµÄsocketÃèÊö·û
				pstPackage	:   3536 ·¢À´µÄ°ü
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»Ø0£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º Èç¹ûÊÕµ½StopVideo ÃüÁî£¬Ïòmy_SAMPLE_VENC_1080P_CLASSICÏß³Ì
				·¢ËÍSIGUSR1ĞÅºÅ
***********************************************************************/
s32 ParamAndExcuteCmd(s32 s32CmdFd, const Package* pstPackage)
{	
	s32 s32Ret;
	if(NULL == pstPackage)
	{
		return -1;
	}

	//printf("---pstPackage.enChType ==%d, pstCmd.enOpt == %d\n",pstPackage->enChType, pstPackage->enOpt);

	if(CMD == pstPackage->enChType && StartVideo ==pstPackage->enOpt)
	{
		stArgs.enHDPicSize = pstPackage->enHDPicSize;
		stArgs.enSDPicSize = pstPackage->enSDPicSize;
		stArgs.s32VideoChNum = pstPackage->s32VideoChNum;
		#if 1
		g_VideoThread_t = CreatePthread(my_SAMPLE_VENC_1080P_CLASSIC, &stArgs);
		#else
		g_VideoThread_t = CreatePthread(TestThread, &stArgs);		
		#endif
	}

	if(CMD == pstPackage->enChType && StopVideo ==pstPackage->enOpt)
	{
		s32Ret = pthread_kill(g_VideoThread_t, SIGUSR1);
		if(ESRCH == s32Ret)
		{
			dbg_printf(DBG_ERROR, "the thread is not exist !!!\n");
			return -1;
		}
		else if(EINVAL== s32Ret)
		{
			dbg_printf(DBG_ERROR, "the SIG is wrong !!!\n");
			return -1;
		}
	}
	
	return 0;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º GetCmdAndChannalType
* ¹¦ÄÜÃèÊö£º »ñÈ¡µ½3536a¶Ë·¢ËÍÀ´µÄÃüÁîºÍ»ñÈ¡Í¨µÀÀàĞÍ
				Í¨µÀÀàĞÍ    :
				CMD  		  :	ÃüÁîÍ¨µÀ	
				HDVIDEODATA   :	¸ßÇåÊÓÆÁÍ¨µÀ
	  			SDVIDEODATA   :	±êÇåÊÓÆµÍ¨µÀ
				AUDEODATA   	  :	ÒôÆµÍ¨µÀ				
* ÊäÈë²ÎÊı£º s32ConFd 		  :	Í¨µÀµÄsocketÃèÊö·û
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º     ³É¹¦·µ»Ø0£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º 3516aÃ¿ÊÕµ½Ò»´ÎÃüÁîºó£¬Ïò3536 ·µ»ØÒ»´Î"OK"
***********************************************************************/
s32 GetCmdAndChannalType(const s32 s32ConFd)
{
	s32 s32Ret;
	Package stPackage= {0};

	if(s32ConFd <= 0 )
	{
		return -1;
	}	

	s32Ret = recv(s32ConFd, &stPackage, sizeof(stPackage), 0);
	if(-1 == s32Ret)
	{
		if(ECONNRESET != errno)
		{
			dbg_perror("recv");
			return -1;
		}
	}
	else if(0 == s32Ret)
	{
		return 0;
	}
	else
	{
		//dbg_printf(DBG_DETAILED, "stPackage.enChType == %d, stPackage.enOpt == %d\n", stPackage.enChType, stPackage.enOpt);
		if(HDVIDEODATA == stPackage.enChType)
		{
			stArgs.s32HDVideoDataFd = s32ConFd;
		
			if(-1 == send(s32ConFd, "OK" ,strlen("OK"), 0) )
			{
				dbg_perror("send");
				return -1;
			}
		}
		else if(SDVIDEODATA == stPackage.enChType)
		{
			stArgs.s32SDVideoDataFd = s32ConFd;
			
			if(-1 == send(s32ConFd, "OK" ,strlen("OK"), 0) )
			{
				dbg_perror("send");
				return -1;
			}
		}	
		else if(HEART == stPackage.enChType)
		{
			if(-1 == send(s32ConFd, "OK" ,strlen("OK"), 0) )
			{
				dbg_perror("send");
				return -1;
			}
			dbg_printf(DBG_DETAILED, "this is heart\n");
		}
		else if((CMD == stPackage.enChType) && (1== stPackage.enOpt))
		{
			g_CmdFd = s32ConFd;
			if(-1 == ParamAndExcuteCmd(s32ConFd, &stPackage))
			{
				dbg_printf(DBG_ERROR, "can not excute the cmd !!!\n");
			}
			
			if(-1 == send(s32ConFd, "OK" ,strlen("OK"), 0) )
			{
				dbg_perror("send");
				return -1;
			}
		}
		else if(CMD == stPackage.enChType)
		{
			g_CmdFd = s32ConFd;
			if(-1 == send(s32ConFd, "OK" ,strlen("OK"), 0) )
			{
				dbg_perror("send");
				return -1;
			}
		}
		else
		{
			dbg_printf(DBG_ERROR, "get wrong  Cmd\n");
			return -1;
		}
	}

	return s32ConFd;
}

/**********************************************************************
* º¯ÊıÃû³Æ£º SAMPLE_VENC_HandleSig
* ¹¦ÄÜÃèÊö£º ÏòÏµÍ³×¢²áĞÅºÅ
* ÊäÈë²ÎÊı£º HI_S32 signo: ĞÅºÅÖµ
* Êä³ö²ÎÊı£º ÎŞ
* ·µ »Ø Öµ£º 	ÎŞ
* ÆäËüËµÃ÷£º SIGUSR1ĞÅºÅÊÇÖ÷Ïß³ÌÏò  my_SAMPLE_VENC_1080P_CLASSICÏß³Ì·¢ËÍµÄĞÅºÅ
				SIGPIPEĞÅºÅÊÇ TCP¶Ï¿ªÁ¬½ÓÊ±»ñÈ¡µ½µÄĞÅºÅ
***********************************************************************/
void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
	exit(-1);
    }

	if(SIGUSR1 ==signo ||SIGPIPE ==signo) 
	{
		g_StopVideoThread = 1;
	}    
}

/**********************************************************************
* º¯ÊıÃû³Æ£º InitFdSet
* ¹¦ÄÜÃèÊö£º ³õÊ¼»¯struct pollfd ½á¹¹Ìå
* ÊäÈë²ÎÊı£º	u32Num:  Í¨µÀÊı 
* Êä³ö²ÎÊı£º pFds:   	 struct pollfd* ĞĞ½á¹¹Ìå£¬poll()º¯ÊıĞèÒª
* ·µ »Ø Öµ£º 	³É¹¦·µ»Ø0£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º ÎŞ
***********************************************************************/
s32 InitFdSet(struct pollfd* pstFds, const u32 u32Num)
{
	s32 i;
	
	if(NULL == pstFds ||0== u32Num)
	{
		return -1;
	}
	
	for(i=0; i<u32Num; i++)
	{
		pstFds[i].fd = -1;
		pstFds[i].events= POLLIN;
	}

	return 0;	
}

/**********************************************************************
* º¯ÊıÃû³Æ£º GetConFd
* ¹¦ÄÜÃèÊö£º »ñÈ¡Óë3536a¶ËÍ¨ĞÅµÄsocketÃèÊö·û
* ÊäÈë²ÎÊı£º	u32Num:  Í¨µÀÊı 
* Êä³ö²ÎÊı£º pFds:   struct pollfd* ĞĞ½á¹¹Ìå£¬poll()º¯ÊıĞèÒª
* ·µ »Ø Öµ£º 	ÕıÈ··µ»Ø0£¬´íÎó·µ»Ø-1
* ÆäËüËµÃ÷£º Í¨¹ıaccept()º¯Êı»ñÈ¡µ½socketÃèÊö·û£¬È»ºó½«ÃèÊö·û
				¸¶¸øpFds[i].fd
***********************************************************************/
s32 GetConFd(struct pollfd* pFds, const u32 u32Num)
{
	s32 i;
	struct sockaddr_un addrcli = {0};
	socklen_t addrlen = sizeof (addrcli);

	if(NULL == pFds || 0 == u32Num)
	{
		return -1;
	}
	
	for(i=1; i<u32Num; i++)
	{
		if(-1 ==pFds[i].fd )
		{
			break;
		}
	}
	
	pFds[i].fd = accept (pFds[0].fd , (struct sockaddr*)&addrcli, &addrlen);
	if (pFds[i].fd == -1) 
	{
		dbg_perror ("accept");
		return -1;
	}

	pFds[i].events= POLLIN;
	
	return 0;
}

int main()
{
	s32 i;	
	s32 s32Ret;
	struct pollfd fds[CHNUM] = {};	 

	signal(SIGINT, SAMPLE_VENC_HandleSig);
    	signal(SIGTERM, SAMPLE_VENC_HandleSig);
	signal(SIGUSR1, SAMPLE_VENC_HandleSig);	
	signal(SIGPIPE, SAMPLE_VENC_HandleSig);	
	
	s32Ret = InitFdSet(fds,  CHNUM);
	if(s32Ret < 0)
	{
		dbg_printf(DBG_ERROR, "InitFdSet is failed\n");
	}
	
	fds[0].fd = CreateSocket(SERVERPORT);
	if(fds[0].fd  < 0)
	{
		dbg_printf(DBG_ERROR, "CreateSocket is failed!!!\n");
		goto EXIT;
	}
	
	while (1)
	{	
		s32Ret = poll(fds, CHNUM, -1);
		if (s32Ret < 0)
		{
			dbg_printf(DBG_ERROR, "select failed!\n");
			break;
		}
		else if (s32Ret == 0)
		{
			continue;
		}
		else
		{
			if(fds[0].revents & POLLIN) 
			{			
				 s32Ret = GetConFd(fds, CHNUM);
				 if(s32Ret < 0)
				 {
				 	dbg_printf(DBG_ERROR, "GetConFd is failed!!!\n");
					goto EXIT;
				 }				
			}
			else
			{ 	
				for(i = 1; i<CHNUM; i++)
				{
					if(fds[i].revents & POLLIN)
					{	
						s32Ret =  GetCmdAndChannalType(fds[i].fd);
						if(s32Ret < 0)
						{
							goto EXIT;
						}
						else if(0 == s32Ret)
						{
							close(g_CmdFd);
							fds[i].fd = -1;
							fds[i].events = 0;											
						}
						break;
					}					
				}
			}				
		}		
	}

	//return 0;

EXIT:
	/*´íÎó´¦Àí*/
	for(i=0; i<CHNUM; i++)
	{
		if(fds[i].fd > 0)
		{
			close(fds[i].fd);
		}
	}
	
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
	return -1;
}





