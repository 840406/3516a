

#include "common.h"
//#include "sample_comm.h"


#define  SERVERPORT     8888


typedef struct my_sample_venc_getstream_s
{
    HI_BOOL bThreadStart;
    HI_S32  s32Cnt;
	s32 s32HDVideoDataFd;
} my_SAMPLE_VENC_GETSTREAM_PARA_S;


Args stArgs;
my_SAMPLE_VENC_GETSTREAM_PARA_S my_gs_stPara;
VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
s32 g_StopVideoThread = 0;
pthread_t g_VideoThread_t;

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

int  CreateSocket(unsigned int port)
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

s32 GetConFdAndChannalType(const s32 s32ListenFd, u32* pu32Type)
{
	s32 s32ConFd;
	s8  as8Buf[50] = {0};
	Package stPackage= {0};
	struct sockaddr_un addrcli = {0};
	socklen_t addrlen = sizeof (addrcli);

	if(s32ListenFd<=0 ||NULL == pu32Type)
	{
		return -1;
	}
		
	s32ConFd = accept (s32ListenFd, (struct sockaddr*)&addrcli,&addrlen);
	if (s32ConFd == -1) 
	{
		dbg_perror ("accept");
		return -1;
	}

	printf("6666-%d\n", s32ConFd);
	if(-1 == recv(s32ConFd, as8Buf, sizeof(as8Buf), 0))
	{
		dbg_perror("send");
		return -1;
	}
	
	if(NULL == strstr(as8Buf, "Hello"))
	{
		return -1;
	}
	
	if(-1 ==send(s32ConFd, "who", strlen("who"), 0))//who are you
	{
		dbg_perror("send");
		return -1;
	}

	if(-1 == recv(s32ConFd, &stPackage, sizeof(Package), 0))
	{
		dbg_perror("send");
		return -1;
	}

	
	if(CMD == stPackage.enChType)
	{
		*pu32Type = CMD;
	}
	else if(HDVIDEODATA == stPackage.enChType)
	{
		*pu32Type = HDVIDEODATA;
	}
	else if(SDVIDEODATA == stPackage.enChType)
	{
		*pu32Type = SDVIDEODATA;
	}
	else
	{
		dbg_printf(DBG_ERROR, "can not get  stCmd.enChType\n");
		return -1;
	}

	dbg_printf(DBG_INFO, "222222222  pu32Type == %d\n", *pu32Type);
	
	return s32ConFd;
}

s32 SendStream(s32 s32HDVideoDataFd,  VENC_STREAM_S* pstStream)
{
	s32 i;
	s32 s32Ret;
	for (i = 0; i < pstStream->u32PackCount; i++)
	{
		s32Ret = send(s32HDVideoDataFd, pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset, \
									pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 0);
		if(-1 == s32Ret)
		{
			dbg_perror("sned");
			return -1;
		}
	}

	return 0;
}


/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* my_SAMPLE_COMM_VENC_GetVencStreamProc(HI_VOID* p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
	s32 s32HDVideoDataFd;
    //VENC_CHN_ATTR_S stVencChnAttr;
    my_SAMPLE_VENC_GETSTREAM_PARA_S* pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    //HI_CHAR aszFileName[VENC_MAX_CHN_NUM][FILE_NAME_LEN];
    FILE* pFile[VENC_MAX_CHN_NUM];
//    char szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
//    VENC_CHN VencChn;
   // PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];

    pstPara = (my_SAMPLE_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;
s32HDVideoDataFd = pstPara->s32HDVideoDataFd;

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
        /* decide the stream file name, and open file to save stream */
        
#if 0
	VencChn = i;	
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                       VencChn, s32Ret);
            return NULL;
        }
        enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

        s32Ret = SAMPLE_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", \
                       stVencChnAttr.stVeAttr.enType, s32Ret);
            return NULL;
        }
        snprintf(aszFileName[i], FILE_NAME_LEN, "stream_chn%d%s", i, szFilePostfix);
        pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i])
        {
            SAMPLE_PRT("open file[%s] failed!\n",
                       aszFileName[i]);
            return NULL;
        }
#endif

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
                   #if 0
                    s32Ret = SAMPLE_COMM_VENC_SaveStream(enPayLoadType[i], pFile[i], &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("save stream failed!\n");
                        break;
                    }
		 #endif

		  s32Ret = SendStream(s32HDVideoDataFd, &stStream);
		  if (HI_SUCCESS != s32Ret)
                   {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("save stream failed!\n");
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
    * step 3 : close save-file
    *******************************************************/
    for (i = 0; i < s32ChnTotal; i++)
    {
        fclose(pFile[i]);
    }

    return NULL;
}


/******************************************************************************
* function :  H.264@1080p@30fps+H.265@1080p@30fps+H.264@D1@30fps
******************************************************************************/
void* my_SAMPLE_VENC_1080P_CLASSIC(void* pArgs)
{
	Args* pstThreadArg = (Args*)pArgs;

	pthread_t VencPid;
	PAYLOAD_TYPE_E enHDPayLoad = PT_H264;
	PAYLOAD_TYPE_E enSDPayLoad = PT_H264;

	PIC_SIZE_E enHDSize = pstThreadArg->enHDPicSize;
	PIC_SIZE_E enSDSize = pstThreadArg->enSDPicSize;

   // PAYLOAD_TYPE_E enPayLoad[3] = {PT_H264, PT_H265, PT_H264};
   // PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD1080, PIC_D1};
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
   // char c;

	SAMPLE_VI_MODE_E SENSOR_TYPE = SAMPLE_VI_MODE_BT1120_1080P; 

    /******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    SAMPLE_COMM_VI_GetSizeBySensor(&enHDSize);

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

	system(" himm 0x20580100   0x80000000");//by cuibo   /*外同步*/

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
    
#if 0	
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
	printf("\t a) Avbr.\n");
    printf("\t f) fixQp\n");
    printf("please input choose rc mode!\n");
    c = (char)getchar();
    switch (c)
    {
        case 'c':
            enRcMode = SAMPLE_RC_CBR;
            break;
        case 'v':
            enRcMode = SAMPLE_RC_VBR;
            break;
					
        case 'a':
            enRcMode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        default:
            printf("rc mode! is invaild!\n");
            goto END_VENC_1080P_CLASSIC_4;
    }
#endif
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
     pthread_create(&VencPid, 0, my_SAMPLE_COMM_VENC_GetVencStreamProc, (HI_VOID*)&my_gs_stPara);


  //  printf("please press twice ENTER to exit this sample\n");
   // getchar();
   // getchar();
   while(0 == g_StopVideoThread)
   {
	sleep(1);	
   }

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

END_VENC_1080P_CLASSIC_5:
    VpssGrp = 0;

    VpssChn = 0;
    VencChn = 0;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);
#if 0 //by cuibo
    VpssChn = 1;
    VencChn = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VENC_Stop(VencChn);


    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        VencChn = 2;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
#endif 
    SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
END_VENC_1080P_CLASSIC_4:	//vpss stop
    VpssGrp = 0;
    VpssChn = 0;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);

#if 0	//by cuibo
    VpssChn = 1;
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    if (SONY_IMX178_LVDS_5M_30FPS != SENSOR_TYPE)
    {
        VpssChn = 2;
        SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    }
#endif
	
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


void* TestThread(void* args)
{
	Args* pstArg = (Args*)args;
	s32 fd = pstArg->s32HDVideoDataFd;
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
		//printf("-=-=-=%d\n", s32Ret);
	

		sleep(1);
		s32Ret = send(stArgs.s32HDVideoDataFd, as8Buf1, strlen(as8Buf1), 0);
		if(s32Ret < 0)
		{
			dbg_perror("send");
		}
		//printf("===%d\n", s32Ret);

	
	}
	
}

s32 ParamAndExcuteCmd(const Package* pstPackage)
{	
	s32 s32Ret;
	if(NULL == pstPackage)
	{
		return -1;
	}

	printf("---pstPackage.enChType ==%d, pstCmd.enOpt == %d\n",pstPackage->enChType, pstPackage->enOpt);

	if(CMD == pstPackage->enChType && StartVideo ==pstPackage->enOpt)
	{
		stArgs.enHDPicSize = pstPackage->enPicSize;
		stArgs.enSDPicSize = pstPackage->enPicSize;
		stArgs.s32VideoChNum = pstPackage->s32VideoChNum;
		//g_VideoThread_t = CreatePthread(my_SAMPLE_VENC_1080P_CLASSIC, &stArgs);

		g_VideoThread_t = CreatePthread(TestThread, &stArgs);
	
	}

	if(CMD == pstPackage->enChType && StopVideo ==pstPackage->enOpt)
	{
		s32Ret = pthread_kill(g_VideoThread_t, SIGUSR1);
		if(ESRCH == s32Ret)
		{
			dbg_printf(DBG_ERROR, "the thread is not exist !!!\n");
		}
		else if(EINVAL== s32Ret)
		{
			dbg_printf(DBG_ERROR, "the SIG is wrong !!!\n");
		}

		return -1;
	}

	return 0;
}

void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
	exit(-1);
    }

	if(SIGUSR1 ==signo)
	{
		g_StopVideoThread = 1;
	}
    
}

int main()
{
	s32 s32ListenFd;
	s32 s32ConFd =0;
	s32 s32CmdFd = 0;
	
	s32 s32MaxFd = 0;
	s32 s32Ret;
	fd_set stFds;
	 struct timeval stTimeoutVal;

	s32ListenFd = CreateSocket(SERVERPORT);
	
	s32MaxFd =s32ListenFd;

	signal(SIGINT, SAMPLE_VENC_HandleSig);
    	signal(SIGTERM, SAMPLE_VENC_HandleSig);
	signal(SIGUSR1, SAMPLE_VENC_HandleSig);	

	while (1)
	{	
		FD_ZERO(&stFds);	
		FD_SET(s32ListenFd,&stFds);
		FD_SET(s32CmdFd,&stFds); 
		FD_SET(stArgs.s32HDVideoDataFd,&stFds); 
	
		s32MaxFd = ( s32CmdFd > s32MaxFd) ? s32CmdFd : s32MaxFd;
		s32MaxFd = ( stArgs.s32HDVideoDataFd > s32MaxFd) ? stArgs.s32HDVideoDataFd : s32MaxFd;
	
		stTimeoutVal.tv_sec  = 1;
		stTimeoutVal.tv_usec = 0;
		
		s32Ret = select(s32MaxFd + 1, &stFds, NULL, NULL, &stTimeoutVal);
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
			if(FD_ISSET(s32ListenFd,&stFds)) 
			{	
				u32 u32Type;
				s32ConFd= GetConFdAndChannalType(s32ListenFd,  &u32Type);
				if(-1 == s32ConFd)
				{
					/*错误处理*/	
					printf(" is wrong\n");
				}
				else if(CMD == u32Type)
				{
					s32CmdFd = s32ConFd;
				}
				else if(HDVIDEODATA == u32Type) 
				{
					stArgs.s32HDVideoDataFd = s32ConFd;
				}	
				else if(SDVIDEODATA == u32Type) 
				{
					stArgs.s32SDVideoDataFd = s32ConFd;
				}
			}
			else if(FD_ISSET(s32CmdFd, &stFds))
			{
				Package stPackage;
				s32 s32RLen;

				s32RLen = recv(s32CmdFd, &stPackage, sizeof(Package), 0);
				if(-1 == s32RLen)
				{
					dbg_perror("send");
					return -1;
				}
				else if(0 == s32RLen)
				{
					s32CmdFd = 0;
				}
				else
				{
					s32Ret = ParamAndExcuteCmd(&stPackage);
					if(-1 == s32Ret)
					{
						dbg_printf(DBG_ERROR, "can not excute the cmd !!!\n");
					}
				}
			}				
		}		
	}
	
	return 0;
}











