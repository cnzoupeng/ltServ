
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "numUpdate.h"
#include "lottory.h"
#include "combin.h"
#include "cmRandom.h"
#include "httpClient.h"
#include "ltCalendar.h"
#include "netCore.h"
#include "jobDisph.h"

using namespace std;
static bool thisAppDebug = false;
static char logFile[256];
static char workDir[256];
static u16 tcpPort = 5000;

void help_exit()
{
	printf("Useage:\n");
	printf("\t-d      : debug model\n");
	printf("\t-p port : listen port\n");
	printf("\t-l logfile : log file name\n");
	printf("\t-w dir : work dir\n");
	exit(0);
}

void parse_cmd(int argc, char* argv[])
{
	int result;
	logFile[0] = 0;
	workDir[0] = 0;
	opterr = 0;
	while( (result = getopt(argc, argv, "dp:l:w:")) != -1 )
	{
		switch(result)
		{
		case 'd':
			thisAppDebug = true;
			break;
		case 'p':	
			tcpPort = atoi(optarg);
			break;
		case 'l':		
			strcpy(logFile, optarg);
			break;
		case 'w':		
			strcpy(workDir, optarg);
			break;
		default:
			help_exit();
			break;
		}
	}
}

void daemonize(void)
{
	int logFd = -1;
	if (logFile[0] != 0)
	{
		logFd = open(logFile, O_CREAT | O_APPEND | O_RDWR);
		if (logFd < 0)
		{
			LogErr("Open logFile failed:%s\n", logFile);			
		}
		LogMsg("LogFile:%s\n", logFile);
	}

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

	if (logFd >= 0)
	{
		close(STDIN_FILENO);		
		close(STDERR_FILENO);
		dup2(logFd, STDOUT_FILENO);
	}
	else
	{
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	if (workDir[0] != 0)
	{
		chdir(workDir);
	}
	else
	{
		chdir("/");
	}
	
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

void ltCalTest()
{
	LtCalen cal;
	int ssqId = 2013053;
	tm drawTime;
	printf("getLastSsqId: %d\n", cal.getLastSsqId());

	
	if(cal.getDrawTimeOfId(ssqId, drawTime) < 0)
	{
		printf("getDrawTimeOfId failed\n");
	}
	drawTime.tm_year -= 1900;
	time_t tmD = mktime(&drawTime);
	printf("getDrawTimeOfId: %d : %s\n", ssqId, ::ctime(&tmD));
	printf("getDrawTimeDist: %d\n", cal.getDrawTimeDist(ssqId));
	exit(0);
}


int main(int argc, char* argv[]) 
{
	dbgOn = LOG_LV_ERR | LOG_LV_MSG | LOG_LV_DBG;

	parse_cmd(argc, argv);

	if (!thisAppDebug)
	{
		daemonize();
	}

	NetCore& netCore = NetCore::getInst();
	if (netCore.init(tcpPort) < 0)
	{
		return -1; 
	}

	NumUpdate& numup = NumUpdate::getInst();
	numup.runBack();

	JobDisph jobDph;
	if (jobDph.run() < 0)
	{
		return -1;
	}

	netCore.runLopp();

	return 0;
}


