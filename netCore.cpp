
#include "netCore.h"
#include <cerrno>
#include <cassert>
#include <ctime>
#include <sys/epoll.h>
#include <cstring>

NetCore* NetCore::_netCore = NULL;

NetCore& NetCore::getInst()
{
	if(_netCore == NULL)
	{
		_netCore = new NetCore();
	}
	return *_netCore;
}

int NetCore::_initEpoll()
{
	_eplFd = epoll_create(100);
	if (_eplFd < 0)
	{
		LogErr("epoll_create failed:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}
int NetCore::_listen(u16 port)
{
	struct sockaddr_in my_addr; 
	if ((_servSock = socket(AF_INET,  SOCK_STREAM,  0)) < 0) 
	{
		LogErr("create socket failed: %s\n", strerror(errno));
		return -1;
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	int flag = 1, len = sizeof(int); 
	if(setsockopt(_servSock, SOL_SOCKET, SO_REUSEADDR, &flag, len) < 0) 
	{ 
		LogErr("setsockopt Failed: %s\n", strerror(errno));
		::close(_servSock);
		return -1;
	} 

	if (bind(_servSock,  (struct sockaddr *)&my_addr,  sizeof(struct sockaddr)) == -1)
	{
		LogErr("bind 0.0.0.0:%u failed: %s\n", port, strerror(errno));
		::close(_servSock);
		return -1;
	}

	if (listen(_servSock,  EPL_MAX_EVENTS) == -1) 
	{
		LogErr("listen on 0.0.0.0:%u failed: %s\n", port, strerror(errno));
		::close(_servSock);
		return -1;
	}

	SockData* skData = new SockData();
	skData->sock = _servSock;
	skData->key = 0;
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = skData;

	if (epoll_ctl(_eplFd, EPOLL_CTL_ADD, _servSock, &ev) < 0)
	{
		LogErr("epoll_ctl failed: %s\n", strerror(errno));
		::close(_servSock);
		delete skData;
		return -1;
	}
	return 0;
}

int NetCore::init(u16 port)
{
	_port = port;

	if (_initEpoll() < 0)
	{
		return -1;
	}

	if (_listen(_port) < 0)
	{
		return -1;
	}

	if (sem_init(&_semReq, 0, 0) < 0)
	{
		LogErr("sem_init Failed %s\n", strerror(errno));
		return -1;
	}

	srand(time(NULL));
	return 0;
}

ReqData* NetCore::getReq()
{
	ReqData* req = NULL;
	
	while(1)
	{
		sem_wait(&_semReq);

		_reqListLock.get();
		if (_reqList.empty())
		{
			LogDbg(" ");
			_reqListLock.free();
			continue;
		}

		req = _reqList.front();
		_reqList.pop_front();
		_reqListLock.free();
		break;
	}

	return req;
}

void NetCore::_connect(SockData* sockData)
{
	assert(sockData != NULL);
	SockData* newSock = new SockData;
	socklen_t addrLen = sizeof(sockaddr_in);
	
	newSock->sock = accept(sockData->sock, (sockaddr*)&(newSock->addr), &addrLen);
	if (newSock->sock < 0)
	{
		LogErr("accept Failed: %s\n", strerror(errno));
		delete newSock;
		return;
	}

	newSock->key = rand();
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = newSock;

	if (epoll_ctl(_eplFd, EPOLL_CTL_ADD, newSock->sock, &ev) < 0)
	{
		LogErr("epoll_ctl failed: %s\n", strerror(errno));		
		::close(newSock->sock);
		delete newSock;
		return;
	}

	_cliMap.insert(make_pair(newSock->sock, newSock));
	LogMsg("%s:%u connect in\n", inet_ntoa(newSock->addr.sin_addr), ntohs(newSock->addr.sin_port));
}

int NetCore::send(SockData* sockData, char* data, int len)
{
	return ::send(sockData->sock, data, len, 0);
}

void NetCore::_recv(SockData* sockData)
{
	assert(sockData != NULL);

	if (_reqList.size() >= MAX_REQ)
	{
		LogErr("Arrive max req = %u\n", MAX_REQ)
		_close(sockData);
		return;
	}

	ReqData* req = new ReqData();
	memcpy(&req->sock, sockData, sizeof(SockData));
	req->len = recv(sockData->sock, &req->head, sizeof(req->data), 0);
	if (req->len < 0)
	{
		LogErr("recv Failed: %s\n", strerror(errno));		
		_close(sockData);
		delete req;
		return;
	}

	if (req->len == 0)
	{
		_close(sockData);
		delete req;
	}

	if (req->len < (int)sizeof(CommHead))
	{
		LogErr("Recv len=%d < head=%u\n", req->len, (int)sizeof(CommHead));
		_close(sockData);
		delete req;
		return;
	}

	//decode

	//magic
	req->head.magic = ntohl(req->head.magic);
	req->head.job = ntohl(req->head.job);
	req->head.len = ntohl(req->head.len);

	if (req->head.magic != LT_MAGIC)
	{
		LogErr("Magic Wrong recv=0x%x wanted=0x%x\n", req->head.magic, LT_MAGIC);
		_close(sockData);
		delete req;
		return;
	}
	if (req->head.job < JOB_MIN || req->head.job > JOB_MAX)
	{
		LogErr("Job Wrong recv=%u [%u-%u]\n", req->head.magic, JOB_MIN, JOB_MAX);
		_close(sockData);
		delete req;
		return;
	}
	if (req->head.len != (u32)req->len)
	{
		LogErr("Head len =%u recved=%u\n", req->head.len, req->len);
		_close(sockData);
		delete req;
		return;
	}

	_reqListLock.get();
	_reqList.push_back(req);
	_reqListLock.free();
	
	sem_post(&_semReq);
}

void NetCore::close(int fd)
{
	_closeListLock.get();
	_closeList.push_back(fd);
	_closeListLock.free();
}

void NetCore::_close(SockData* sockData)
{
	map<int, SockData*>::iterator itMap;
	itMap = _cliMap.find(sockData->sock);
	if (itMap == _cliMap.end())
	{
		return ;		
	}
	_cliMap.erase(itMap);

	if (epoll_ctl(_eplFd, EPOLL_CTL_DEL, sockData->sock, NULL) < 0)
	{
		LogErr("epoll_ctl failed: %s\n", strerror(errno));
	}
	LogMsg("%s:%u connection closed\n", inet_ntoa(sockData->addr.sin_addr), ntohs(sockData->addr.sin_port));

	::close(sockData->sock);
	delete sockData;
}

void NetCore::_dealClose()
{
	int fdCli = 0;
	SockData* sock = NULL;
	map<int, SockData*>::iterator itMap;

	while(1)
	{
		_closeListLock.get();
		if (_closeList.empty())
		{
			_closeListLock.free();
			break;
		}
		fdCli = _closeList.front();
		_closeList.pop_front();

		_closeListLock.free();
		itMap = _cliMap.find(fdCli);
		if (itMap != _cliMap.end())
		{
			sock = itMap->second;
			_close(sock);
		}
	}
}

int NetCore::runLopp()
{
	int nfds = 0;
	epoll_event events[EPL_MAX_EVENTS];
	SockData* sockData;
	 while(1)
	 {
		 
		 nfds = epoll_wait(_eplFd, events, EPL_MAX_EVENTS, EPL_SLEEP);
		 if (nfds == -1) 
		 {
			LogErr("epoll_wait failed: %s\n", strerror(errno));
			continue;
		 }
		 
		  _dealClose();

		 if (nfds == 0)
		 {			
			 continue;
		 }

		 for (int i = 0; i < nfds; ++i) 
		 {
			 sockData = (SockData*)events[i].data.ptr;
			 if (sockData->sock == _servSock)
			 {
				 _connect(sockData);
				 continue;
			 }

			 _recv(sockData);
		 }
	 }
	 return -1;
}