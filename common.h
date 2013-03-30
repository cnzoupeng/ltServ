
#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdio>

typedef unsigned char		u8;
typedef	unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;


#define LOG_LV_ERR	0x01
#define	LOG_LV_MSG	0x02
#define	LOG_LV_DBG	0x04


extern int dbgOn;
#define LogDbg(fmt...){if(dbgOn & LOG_LV_DBG){printf("DBG:%s:%d: ", __FILE__, __LINE__);printf(fmt);}}
#define LogMsg(fmt...){if(dbgOn & LOG_LV_MSG){printf("MSG:%s:%d: ", __FILE__, __LINE__);printf(fmt);}}
#define LogErr(fmt...){if(dbgOn & LOG_LV_ERR){printf("ERR:%s:%d: ", __FILE__, __LINE__);printf(fmt);}}

u8	bit_count(u8 u);
u32	bit_count(u64 u);



#endif //__COMMON_H__

