
#include "ltServ.h"
#include <boost/algorithm/string.hpp>
#include "boost/thread.hpp"  
#include "boost/asio.hpp"


using boost::asio::ip::udp;
using namespace std;

LtServ::LtServ(const char* ip, u16 port, LtAi&	ai)
	: _ip(ip)
	, _ltAi(ai)
{
	_port = port;
}

LtServ::~LtServ()
{

}

int LtServ::_rand_build(vector<string>& args, char* outMsg, int* outLen)
{
	strcpy(outMsg, CMD_RES);

	int count = 1;
	if (args.size() > 1)
	{
		count = atoi(args[1].c_str());
		if (count > MAX_RAND_PER_TIME || count < 1)
		{
			sprintf(outMsg + CMD_RES_LEN, ":1:count max < %d\n", MAX_RAND_PER_TIME);
			*outLen = strlen(outMsg);
			return 0;
		}
	}

	for(int i = 0; i < count; i++)
	{
		Lottory item;
		_ltAi.getRand(item);
		strcat(outMsg + CMD_RES_LEN, ":");
		int msgOutLen = strlen(outMsg);
		item.toString(outMsg + msgOutLen, *outLen - msgOutLen);
	}	
	
	LogDbg("Rand: %s\n", outMsg + CMD_RES_LEN);
	*outLen = strlen(outMsg);
	return 0;
}

int LtServ::_recv_msg(char* msgRecv, int recvLen, char* outMsg, int* outLen)
{
	assert(msgRecv != NULL);
	assert(outMsg != NULL);
	

	if (recvLen <= CMD_REQ_LEN + 1)
	{
		LogErr("Recv wrong Msg:%s\n", msgRecv);
		return -1;
	}

	if (memcmp(msgRecv, CMD_REQ, CMD_REQ_LEN) != 0)
	{
		LogErr("Recv wrong Msg:%s\n", msgRecv);
		return -1;
	}

	vector<string> request;
	string recv(msgRecv + CMD_REQ_LEN + 1);	
	boost::algorithm::split(request, recv, boost::algorithm::is_any_of(":\n"));
	if (request.size() == 0)
	{
		msgRecv[recvLen - 1] = 0;
		LogErr("Recv wrong Msg: %s\n", recv.c_str());
		return -1;
	}

	if (request[request.size() - 1] == "\n")
	{
		request.erase(request.begin() + request.size());
	}

	//-------------------------
	if(request[0] == "RAND")
	{
		return _rand_build(request, outMsg, outLen);
	}

	LogErr("Recv wrong Msg: %s\n", recv.c_str());
	return -1;
}

int LtServ::run_loop()
{
	char recvBuf[4096];
	char sendBuf[4096];
	int sendLen = 0;
	boost::system::error_code err;
	boost::asio::io_service io_service;  
	udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint send_point;
	udp::endpoint local_add(boost::asio::ip::address::from_string(_ip), _port);  
	udp_socket.open(local_add.protocol());
	udp_socket.bind(local_add, err);
	if (err)
	{
		LogErr("Bind failed: %s\n", err.message().c_str());
		return -1;
	}

	LogMsg("Serv start on %s:%u\n", _ip.c_str(), _port);

	while (true)  
	{	
		size_t recvLen = udp_socket.receive_from(boost::asio::buffer(recvBuf, sizeof(recvBuf)), send_point);
		//LogDbg("Recv From %s:%u: %s\n", send_point.address().to_string().c_str(), send_point.port(), recvBuf);

		sendLen = sizeof(sendBuf);
		if(_recv_msg(recvBuf, (u32)recvLen, sendBuf, &sendLen) == 0)
		{
			udp_socket.send_to(boost::asio::buffer(sendBuf, sendLen), send_point);
		}
	}
	return -1;
}




