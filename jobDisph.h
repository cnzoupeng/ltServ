
#ifndef __LTSERV_JOB_DISP_H__
#define __LTSERV_JOB_DISP_H__

#include "common.h"
#include "netCore.h"

#define MAX_THREAD	64

class JobDisph
{
public:
	int run();

private:
	static void* _runRecv(void* arg);

	static void _historyGet(ReqData*);

private:
	int _thdCount;
	pthread_t _thids[MAX_THREAD];
};

#endif
