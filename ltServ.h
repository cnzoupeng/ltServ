
#ifndef __LOTTORY_SERV_H__
#define __LOTTORY_SERV_H__

#include <string>
#include "ltAi.h"
using namespace std;

#define  CMD_REQ "LTREQ"
#define  CMD_RES "LTRES"
#define  CMD_REQ_LEN		5
#define  CMD_RES_LEN		5
#define  MAX_RAND_PER_TIME	20

class LtServ
{
public:
	LtServ(const char* ip, u16 port, LtAi&	ai);
	~LtServ();

	int run_loop();
	

private:

	int _recv_msg(char* msgRecv, int recvLen, char* outMsg, int* outLen);

	int _rand_build(vector<string>& args, char* outMsg, int* outLen);

	int _update_history(vector<string>& args, char* outMsg, int* outLen);

private:
	string  _ip;
	u16		_port;
	LtAi&	_ltAi;
};


#endif //__LOTTORY_SERV_H__

