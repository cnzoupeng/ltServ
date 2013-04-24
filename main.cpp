
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>


#include "lottory.h"
#include "combin.h"
#include "cmRandom.h"
#include "ltAi.h"
#include "ltServ.h"
#include "httpClient.h"
#include "numUpdate.h"

using namespace std;


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

void rand_test()
{
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

	LtAi& aiCore = LtAi::getInst();
	if (aiCore.load_history() < 0)
	{
		return -1;
	}

	NumUpdate& numup = NumUpdate::getInst();
	numup.runBack();

	LtServ serv(LISTEN_IP, LISTEN_PORT, aiCore);
	serv.run_loop();
		
	return 0;
}


