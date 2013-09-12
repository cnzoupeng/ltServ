
#ifndef __LTSERV_NET_CORE_H__
#define __LTSERV_NET_CORE_H__

#include "common.h"
#include "protocol.h"
#include "lock.h"
#include <list>
#include <map>
#include <pthread.h>
using namespace std;


#define MAX_REQ		128

#define EPL_SLEEP	50
#define EPL_MAX_EVENTS	64

class NetCore
{
public:
	static NetCore& getInst();

	int runLopp();

	int init(u16 port);

	ReqData* getReq();

	void close(int fd);

	int send(SockData* sockData, char* data, int len);

private:
	NetCore(){};
	~NetCore(){};

	int _initEpoll();
	int _listen(u16 port);
	void _connect(SockData* sockData);
	void _recv(SockData* sockData);
	void _close(SockData* sockData);
	void _dealClose();
private:
	u16 _port;
	int _eplFd;
	int _servSock;	
	
	sem_t _semReq;
	map<int, SockData*> _cliMap;
	list<ReqData*> _reqList;
	list<int> _closeList;
	Lock _closeListLock;
	Lock _reqListLock;	
	static NetCore* _netCore;
};

#endif


