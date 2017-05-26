
#ifndef  COMMON_H
#define   COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <net/if.h>   
#include <fcntl.h>
#include <ctype.h>  
#include <sys/un.h>  
#include <sys/ioctl.h>  
#include <linux/types.h>  
#include <linux/netlink.h>  
#include <errno.h>  
#include <signal.h>
#include <termios.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <time.h>
#include <math.h>
#include <sys/file.h>
#include <sys/time.h>

#include "sample_comm.h"

 typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef  char             s8;
typedef short                   s16;
typedef int                     s32;


#define DBG_DETAILED    1
#define DBG_INFO        2
#define DBG_WARNING        3
#define DBG_ERROR        4
#define DBG_FATAL        5
#define DBG_LEVEL        DBG_INFO
#define dbg_printf(level, fmt,args...) if(level>=DBG_LEVEL){\
								printf("[%s:%d]: "fmt,__func__, __LINE__, ##args);  \
								}
#define dbg_perror(x)  printf("[%s:%d]perror:\n",__func__,__LINE__);\
							perror(x);

typedef void* (*Fun)(void*)  ;

typedef enum ChType{
	  CMD  		=	1,	
	  HDVIDEODATA   	,
	  SDVIDEODATA   	,
	  AUDEODATA   	,
	  HEART,
}ChType;

typedef enum Opt{
	  StartVideo  		=	1,
	  StopVideo   	,
}Opt;

typedef struct Package{
	ChType enChType;
	Opt       enOpt;
	PIC_SIZE_E  enPicSize;
	s32    s32VideoChNum;
}Package;

typedef struct Args{
	PIC_SIZE_E  enHDPicSize;
	PIC_SIZE_E  enSDPicSize;
	s32    s32VideoChNum;
	s32 s32HDVideoDataFd;
	s32 s32SDVideoDataFd;
}Args;
pthread_t CreatePthread(Fun SFun, void* args);



#endif  //COMMON_H






