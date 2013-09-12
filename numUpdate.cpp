
#include "numUpdate.h"
#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <time.h>
#include "httpClient.h"
#include<boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;


int NumUpdate::runBack()
{
	pthread_t thread_id;
	int ret = pthread_create(&thread_id, NULL, NumUpdate::run, this);
	if (ret != 0)
	{
		printf("pthread_create error:%s\n", strerror(errno));
		return ret;
	}
	return 0;
}

int NumUpdate::runLoop()
{
	run(this);
	return 0;
}


int NumUpdate::get_last_id_from_html(string& page, int& lastId)
{
	iterator_range<string::iterator> result = boost::algorithm::find_first(page, "option value");
	if(result.empty())
	{
		LogErr("[option value] not found int page\n");
		return -1;
	}

	string strTmp(result.begin(), page.end());
	result = boost::algorithm::find_first(strTmp, "\"");
	if(result.empty())
	{
		LogErr("[\"] not found int page\n");
		return -1;
	}
	int pos = result.begin() - strTmp.begin() + 1;
	string strVal("20");
	strVal += strTmp.substr(pos, 5);
	lastId = atoi(strVal.c_str());

	LogMsg("Get last id: %d\n", lastId);
	return 0;
}

int NumUpdate::get_xml_value(string& xml, char* value)
{
	iterator_range<string::iterator> posStart = boost::algorithm::find_first(xml, ">");
	iterator_range<string::iterator> posEnd = boost::algorithm::find_first(xml, "<");
	if(posStart.empty() || posEnd.empty())
	{
		LogErr("[><] not found int page\n");
		return -1;
	}
	int begin = posStart.begin() - xml.begin() + 1;
	int len = posEnd.begin() - posStart.begin() - 1;
	memcpy(value, xml.c_str() + begin, len);
	value[len] = 0;
	return 0;
}

int NumUpdate::get_item_from_page(string& page, vector<string>& items)
{
	iterator_range<string::iterator> posStart = boost::algorithm::find_first(page, "tbody");
	if(posStart.empty())
	{
		LogErr("[tbody] not found int page\n");
		return -1;
	}

	iterator_range<string::iterator> posEnd = boost::algorithm::find_nth(page, "tbody", 1);
	if(posEnd.empty())
	{
		LogErr("[tbody]2 not found int page\n");
		return -1;
	}

	string strCont = page.substr(posStart.begin() - page.begin(), posEnd.begin() - posStart.begin());

	vector<string> tr_all;
	my_split(strCont, "<tr", tr_all);
	if (tr_all.size() == 0)
	{
		LogErr("no item found\n");
		return -1;
	}

	vector<string>::iterator it;
	for (it = tr_all.begin(); it != tr_all.end(); ++it)
	{
		vector<string> td_all;
		my_split(*it, "<td", td_all);
		if (td_all.size() < 10)
		{			
			continue;
		}

		string itemVal("20");
		char val[64];
		int err = 0;
		for (int i = 2; i < 10; i++)
		{			
			if (get_xml_value(td_all[i], val) < 0)
			{
				err = 1;
				break;
			}
			
			itemVal += val;
			if (i < 9)
			{
				itemVal += " ";
			}			
		}
		if (err == 0)
		{
			LogMsg("Get: %s\n", itemVal.c_str());
			items.push_back(itemVal);
		}
	}
	return 0;
}


void* NumUpdate::run(void* arg)
{	
	HttpClient http;
	string page;
	char url[128];	
	vector<string> newItems;
	int webLastId, dbLastId;
	assert(arg != NULL);
	NumUpdate& numUp = *(NumUpdate*)arg;
	bool runFirst = true, runFailed = false;

	numUp.load_from_file();

	LogMsg("NumUpdate start success\n");

	while(1)
	{
		if (!runFirst)
		{
			if (runFailed)
			{
				LogDbg("Sleep 10\n");
				sleep(10);
			}
			else
			{
				time_t curTm = time(NULL);
				struct tm* lcTm = ::localtime(&curTm);
				if ((lcTm->tm_wday == 2 || lcTm->tm_wday == 4 || lcTm->tm_wday == 6)
					&& (lcTm->tm_hour > 21 && lcTm->tm_hour < 23))
				{
					LogDbg("Sleep 10 * 60\n");
					sleep(10 * 60);
				}
				else
				{
					LogDbg("Sleep 60 * 60\n");
					sleep(60 * 60);
				}
			}
		}
		runFirst = false;

		page.clear();
		if(http.getPage("datachart.500.com", "/ssq/history/inc/history_same.php", page) != 0)
		{
			LogErr("getPage failed:/ssq/history/inc/history_same.php\n");
			runFailed = true;
			continue;
		}

		dbLastId = numUp._history.back().id;
		if (numUp.get_last_id_from_html(page, webLastId) < 0)
		{
			LogErr("get_last_id_from_html failed");
			runFailed = true;
			continue;
		}

		if (webLastId <= (dbLastId + 1))
		{
			LogMsg("Local Last=%d WEB Last=%d\n", dbLastId, webLastId  - 1);
			runFailed = false;
			continue;
		}
		
		page.clear();
		sprintf(url, "/ssq/history/inc/history_same.php?start=%d&end=%d", dbLastId + 1 - 2000000, webLastId - 2000000);		
		if(http.getPage("datachart.500.com", url, page) != 0)
		{
			LogErr("getPage failed:%s\n", url);
			runFailed = true;
			continue;
		}

		//从页面解析号码
		newItems.clear();
		if (numUp.get_item_from_page(page, newItems) < 0)
		{
			LogErr("get_item_from_page failed\n");
			runFailed = true;
			continue;
		}

		//存入文件
		HisItem item;
		for(u32 i = 0; i < newItems.size(); ++i)
		{			
			memset(&item, 0, sizeof(HisItem));
			item.id = atoi(newItems[i].c_str());
			strcpy(item.num, newItems[i].c_str());
			numUp._history.push_back(item);
			numUp._mapHisId.insert(make_pair(item.id, numUp._history.size() - 1));
		}

		numUp.write_to_file();
	}
	return NULL;
}

int NumUpdate::load_from_file(const char* fileName)
{
	HisItem item;
	string str;
	ifstream fin(fileName);
	_history.clear();
	_mapHisId.clear();
	int index = 0;	

	while(getline(fin, str))
	{
		if (str.length() < 20)
		{
			continue;
		}

		memset(&item, 0, sizeof(HisItem));
		item.id = atoi(str.c_str());
		strcpy(item.num, str.c_str());
		_history.push_back(item);
		_mapHisId.insert(make_pair(item.id, index++));
	}

	//初始化第一个
	if (_history.size() == 0)
	{
		memset(&item, 0, sizeof(HisItem));
		item.id = 2003001;
		strcpy(item.num, "2003001 10 11 12 13 26 28 11");
		_history.push_back(item);
		_mapHisId.insert(make_pair(item.id, 0));
	}
	return 0;
}

int NumUpdate::write_to_file(const char* fileName)
{
	char tmpBuf[64];
	ofstream fout(fileName, ios::trunc);
	for(u32 i = 0; i < _history.size(); ++i)
	{
		strcpy(tmpBuf, _history[i].num);
		strcat(tmpBuf, "\n");
		fout.write(tmpBuf, strlen(tmpBuf));
	}
	fout.flush();
	fout.close();
	return 0;
}

int NumUpdate::get_range_str(int start, int stop, string& outStr)
{
	if (start < 2003001)
	{
		LogErr("Start ID =%d must < 2003001\n", start);
		return -1;
	}

	if (stop > _history.back().id)
	{
		stop = _history.back().id;
	}

	map<int, int>::iterator itMap = _mapHisId.find(start);
	if (itMap == _mapHisId.end())
	{
		LogErr("ID %d not found\n", start);
		return -1;
	}
	int posStart = itMap->second;

	itMap = _mapHisId.find(stop);
	if (itMap == _mapHisId.end())
	{
		LogErr("ID %d not found\n", stop);
		return -1;
	}
	int posStop = itMap->second;

	outStr = "";
	for (int i = posStart; i <= posStop; ++i)
	{
		outStr += _history[i].num;
		outStr += "\n";
	}

	return 0;
}

