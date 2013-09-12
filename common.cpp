
#include "common.h"
#include <cstring>
#include <ctime>
using namespace std;

int dbgOn = 0;

u8 bit_count(u8 u)
{
	u8 ret = 0;
	while (u)
	{
		u = (u & (u - 1));
		ret ++;
	}
	return ret;
}

u32 bit_count(u64 u)
{
	u32 ret = 0;
	while (u)
	{
		u = (u & (u - 1));
		ret ++;
	}
	return ret;
}

void my_split(std::string& str, const char* delim, std::vector<std::string>& out)
{
	string strSub;
	int delimLen = strlen(delim);	
	const char* strCont = str.c_str();
	const char* strEnd = strCont + str.length();
	const char* strFind = strstr(strCont, delim);
	while(strFind && strCont < strEnd)
	{	
		if (strCont == strFind)
		{
			strCont += delimLen;
			strFind = strstr(strCont, delim);
			continue;
		}
		strSub.clear();
		strSub.append(strCont, strFind - strCont);
		strCont = strFind + delimLen;
		out.push_back(strSub);
		strFind = strstr(strCont, delim);
	}

	if (strCont != strEnd)
	{
		strSub.clear();
		strSub.append(strCont, strEnd - strCont);
		out.push_back(strSub);
	}
}

char* nowTime()
{
	static char tmStr[64];
	time_t rawtime;	
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	sprintf(tmStr, "%02d/%02d %02d:%02d:%02d", timeinfo->tm_mon + 1, timeinfo->tm_mday, 
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return tmStr;
}

