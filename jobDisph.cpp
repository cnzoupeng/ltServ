
#include "jobDisph.h"
#include <sys/sysinfo.h>
#include <cassert>
#include "numUpdate.h"
#include <cstring>
#include <cerrno>

int JobDisph::run()
{
	_thdCount = get_nprocs();
	_thdCount *= 8;
	if (_thdCount > MAX_THREAD)
	{
		_thdCount = MAX_THREAD;
	}

	for (int i = 0; i < _thdCount; ++i)
	{
		if (pthread_create(&_thids[i], NULL, JobDisph::_runRecv, this) != 0)
		{
			return -1;
		}
		pthread_detach(_thids[i]);
	}
	return 0;
}

void* JobDisph::_runRecv(void* arg)
{
	NetCore& netCore = NetCore::getInst();
	ReqData* req = NULL;

	while(1)
	{
		req = netCore.getReq();
		switch(req->head.job)
		{
		case JOB_HISTORY:
			_historyGet(req);
			break;
		default:
			netCore.close(req->sock.sock);
			break;
		}
	}

	return NULL;
}

void JobDisph::_historyGet(ReqData* req)
{
	assert(req != NULL);
	NetCore& netCore = NetCore::getInst();
	NumUpdate& numUp = NumUpdate::getInst();
	int mustLen = sizeof(CommHead) + sizeof(ReqHis);

	if (req->len != mustLen)
	{
		LogErr("Recv len wrong =%d wanted=%d\n", req->len, mustLen);
		netCore.close(req->sock.sock);
		delete req;
		return;
	}

	if (req->head.len != (u32)mustLen)
	{
		LogErr("Recv head.len wrong =%d wanted=%d\n", req->head.len, mustLen);
		netCore.close(req->sock.sock);
		delete req;
		return;
	}

	string hisStr;
	ReqHis* reqHis = (ReqHis*)req->data;
	reqHis->start = ntohl(reqHis->start);
	reqHis->stop = ntohl(reqHis->stop);

	LogMsg("REQ: JOB_HISTORY: %d--%d\n", reqHis->start, reqHis->stop);
	
	if (numUp.get_range_str(reqHis->start, reqHis->stop, hisStr) < 0)
	{
		//netCore.close(req->sock.sock);
		//delete req;
		//return;
	}

	int resLen = sizeof(ResData);
	if (hisStr.length() > 0)
	{
		resLen += hisStr.length() + 1;
	}
	ResData* res = (ResData*)new char[resLen];
	
	res->head.key = 0;
	res->head.magic = ntohl(LT_MAGIC);	
	res->head.job = ntohl(JOB_HISTORY);
	res->head.len = ntohl(resLen);

	res->result = 0;
	if (hisStr.length() > 0)
	{
		strcpy(res->data, hisStr.c_str());
	}

	char* pSend = (char*)res;
	int freg = resLen / TCP_FREG_LEG;
	int minLen = resLen % TCP_FREG_LEG;

	for (int i = freg; i > 0; --i)
	{
		if(netCore.send(&req->sock, pSend, TCP_FREG_LEG) != TCP_FREG_LEG)
		{
			LogErr("Send failed %s\n", strerror(errno));
			break;
		}
		pSend += TCP_FREG_LEG;
	}

	if (minLen > 0)
	{
		if(netCore.send(&req->sock, pSend, minLen) != minLen)
		{
			LogErr("Send failed %s\n", strerror(errno));
		}
	}

	netCore.close(req->sock.sock);
	delete req;
	delete res;
}