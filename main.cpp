
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>

#include "mongo/client/dbclient.h"
#include <boost/algorithm/string.hpp>
#include "boost/thread.hpp"  
#include "boost/asio.hpp"
#include "lottory.h"
#include "combin.h"
#include "cmRandom.h"
#include "ltAi.h"

using namespace std;
using namespace mongo;
using boost::asio::ip::udp;


#define CL_NAME			"ssq.number"
#define LISTEN_IP		"127.0.0.1"
#define LISTEN_PORT		6666

LtAi aiCore;


int load_item_from_db(vector<Lottory>& ltHis)
{	
	char ltIdBuf[32];
	Lottory item;
	string strItem;	
	string errMsg;
	mongo::Query qSort;
	int err = 0;

	mongo::DBClientConnection dbCon;
	if(!dbCon.connect("localhost", errMsg))
	{
		LogErr("Connect to db failed: %s\n", errMsg.c_str());
		return -1;
	}

	qSort.sort("id", 1);
	std::auto_ptr<DBClientCursor> cursor = dbCon.query(CL_NAME , qSort);	
	while ( cursor->more())
	{
		BSONObj obj = cursor->next();
		sprintf(ltIdBuf, "%d ", obj.getIntField("id"));
		strItem = ltIdBuf;
		strItem += obj.getStringField("num");
		if (item.Set(strItem) < 0)
		{
			err = -1;
			break;
		}
		ltHis.push_back(item);
		//ltt.dump_item(item);
	}

	if (err != 0)
	{
		ltHis.clear();
		return err;
	}

	LogMsg("Load %u History items\n", (u32)ltHis.size());
	return 0;
}

int recv_cmd(char* cmd, u32 len, char* out, int* outLen)
{
	memcpy(out, cmd, len);
	string recv(cmd);

	vector<string> request;
	boost::algorithm::split(request, recv, boost::algorithm::is_any_of(":\n"));
	if (request.size() < 2)
	{
		LogErr("Recv wrong Msg: %s\n", recv.c_str());
		return -1;
	}

	if (request[0] != "LTSRV")
	{
		LogErr("Recv wrong Msg: %s\n", recv.c_str());
		return -1;
	}

	//-------------------------
	if(request[1] == "RAND")
	{
		Lottory item;
		aiCore.getRand(item);
		strcpy(out, "LTSRV:");
		item.toString(out + 6, *outLen - 6);
		LogDbg("Rand: %s\n", out + 6);
		*outLen = strlen(out);
		return 0;
	}

	LogErr("Recv wrong Msg: %s\n", recv.c_str());
	return -1;
}

int run_loop()
{
	char recvBuf[4096];
	char sendBuf[4096];
	int sendLen = 0;
	boost::system::error_code err;
	boost::asio::io_service io_service;  
	udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint send_point;
	udp::endpoint local_add(boost::asio::ip::address::from_string(LISTEN_IP), LISTEN_PORT);  
	udp_socket.open(local_add.protocol());  
	udp_socket.bind(local_add, err); 
	if (err)
	{
		printf("Bind failed: %s\n", err.message().c_str());
		return -1;
	}
	
	LogMsg("Serv start on %s:%u\n", LISTEN_IP, LISTEN_PORT);

	while (true)  
	{	
		size_t recvLen = udp_socket.receive_from(boost::asio::buffer(recvBuf, sizeof(recvBuf)), send_point);
		//LogDbg("Recv From %s:%u: %s\n", send_point.address().to_string().c_str(), send_point.port(), recvBuf);

		sendLen = sizeof(sendBuf);
		if(recv_cmd(recvBuf, (u32)recvLen, sendBuf, &sendLen) == 0)
		{
			udp_socket.send_to(boost::asio::buffer(sendBuf, sendLen), send_point);
		}			
	}
	return -1;
}

void daemonize(void)
{
	pid_t pid;
	pid = fork();
	if (pid < 0) {
		fprintf(stderr, "fork, fail.\n");
		exit(1);
	}

	if (pid > 0) {
		exit(0);
	}

	setsid();

	close(0);
	close(1);
	close(2);

	chdir("/");
}

void help_exit()
{
	printf("Useage:\n");
	printf("\t-d      : debug model\n");
	printf("\t-p port : listen port\n");
	exit(0);
}

void parse_cmd(int argc, char* argv[])
{
	int result;
	opterr = 0;
	while( (result = getopt(argc, argv, "dp:")) != -1 )
	{
		switch(result)
		{
		case 'd':
			dbgOn |= LOG_LV_DBG;
			break;
		case 'p':			
			break;
		default:
			help_exit();
			break;
		}
	}
}

void rand_test()
{
	// 	for(int i = 0; i < 100; i ++)
	// 	{
	// 		printf("%-10u  %-10u %-10u %-10u %-10u\n", 
	// 			CmRandom::getUint(),
	// 			CmRandom::getUint(),
	// 			CmRandom::getUint(),
	// 			CmRandom::getUint(),
	// 			CmRandom::getUint()
	// 			);
	// 	}

	const u32 MAX_X = 0xFFFFFFFF;
	u32 countPt[10];
	for(int i = 0; i < 10; i++)
	{
		countPt[i] = MAX_X / 10 * (i + 1);
	}
	countPt[9] = MAX_X;

	u32 count[10] = {0};
	for (u32 i = 0; i < MAX_X; i++)
	{
		u32 rd = CmRandom::getUint() % MAX_X;
		for (int i = 0; i < 10; i++)
		{
			if (rd <= countPt[i])
			{
				count[i] ++;
				break;
			}
		}
	}

	for(int i = 0; i < 10; i++)
	{
		printf("[%d]: %u\n", i, count[i]);
	}

	exit(0);
}

int main(int argc, char* argv[]) 
{
	dbgOn = LOG_LV_ERR | LOG_LV_MSG | LOG_LV_DBG;
	
	parse_cmd(argc, argv);

	if (!(dbgOn & LOG_LV_DBG))
	{
		daemonize();
	}	

	if (load_item_from_db(aiCore.getHis()) < 0)
	{
		return -1;
	}

	run_loop();
	return 0;
}


