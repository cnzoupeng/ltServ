
#ifndef __LT_PROTOCOL_H__
#define __LT_PROTOCOL_H__

#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define MAX_REQ_DATA_LEN 1024

#define TCP_FREG_LEG (8 * 1024)

#define LT_MAGIC	0x56ADF65B

enum
{
	JOB_MIN = 900001,
	JOB_HISTORY = JOB_MIN,
	JOB_VERSION,
	JOB_FILE,
	JOB_MAX = JOB_FILE
};

struct CommHead
{
	u32 key;
	u32 magic;
	u32 job;
	u32 len;
};

struct SockData
{
	int sock;
	u32 key;
	struct sockaddr_in addr;
};

struct ReqData
{
	int len;
	SockData sock;	
	CommHead head;
	char data[MAX_REQ_DATA_LEN];
};

struct ResData
{
	CommHead head;
	int result;
	char data[0];
};

struct ReqHis
{
	u32 start;
	u32 stop;
};

#endif

