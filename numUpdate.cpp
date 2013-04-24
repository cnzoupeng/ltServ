
#include "numUpdate.h"
#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <time.h>
#include "mongo/client/dbclient.h"
#include <boost/algorithm/string.hpp>
#include "httpClient.h"
#include "ltAi.h"

using namespace boost;
using namespace mongo;


NumUpdate::NumUpdate()
	: _dbAddr(DB_ADDR)
	, _dbHisName(DB_HISTORY_NAME)
	, _dbRandName(DB_RAND_NAME)
{
}

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

int NumUpdate::get_last_id_from_db(int& lastId)
{
	string errMsg;
	string strItem;
	DBClientConnection dbCon;
	if(!dbCon.connect(_dbAddr.c_str(), errMsg))
	{
		LogErr("Connect to db %s failed: %s\n", _dbAddr.c_str(), errMsg.c_str());
		return -1;
	}

	Lottory item;
	char ltIdBuf[32];
	mongo::Query qSort;
	qSort.sort("id", -1);
	std::auto_ptr<DBClientCursor> cursor = dbCon.query(_dbHisName , qSort, 1);	
	if (cursor->more())
	{
		BSONObj obj = cursor->next();
		sprintf(ltIdBuf, "%d ", obj.getIntField("id"));
		strItem = ltIdBuf;
		strItem += obj.getStringField("num");
		if (item.Set(strItem) < 0)
		{
			LogErr("Convert item failed\n");
			return -1;
		}
	}
	else
	{
		LogErr("No data response\n");
		return -1;
	}

	lastId = (int)item.no;
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
	string strVal = strTmp.substr(pos, 5);
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

		string itemVal;
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

int NumUpdate::save_item_to_db(vector<string>& items)
{
	string errMsg;
	char strItem[128];
	char strTmp[128];
	vector<BSONObj> vecObj;
	DBClientConnection dbCon;

	if(!dbCon.connect(_dbAddr.c_str(), errMsg))
	{
		LogErr("Connect to db %s failed: %s\n", _dbAddr.c_str(), errMsg.c_str());
		return -1;
	}

	for (u32 i = 0; i< items.size(); i++)
	{
		strcpy(strItem, items[i].c_str());
		strItem[5] = 0;
		sprintf(strTmp, "{id:20%s, num:\"%s\"}", strItem, &strItem[6]);
		vecObj.push_back(fromjson(strTmp));
	}

	dbCon.resetError();
	dbCon.insert(_dbHisName, vecObj);
	string err = dbCon.getLastError();
	if (!err.empty())
	{
		LogErr("Insert History Err:%s\n", err.c_str());
		return -1;
	}

	LogMsg("Insert History %u items to db Success\n", (u32)vecObj.size());
	return 0;
}

int NumUpdate::generate_new_rand(int lastId)
{
	string errMsg;
	vector<BSONObj> vecObj;
	DBClientConnection dbCon;

	if(!dbCon.connect(_dbAddr.c_str(), errMsg))
	{
		LogErr("Connect to db %s failed: %s\n", _dbAddr.c_str(), errMsg.c_str());
		return -1;
	}

	char idStr[32];
	char newRand[512];
	LtAi& ltAi = LtAi::getInst();
	Lottory item[RAND_PER_COUNT];
	int len = sprintf(newRand, "{id:%d ,nums:\"", lastId + 1);

	for(int i = 0; i < RAND_PER_COUNT; i++)
	{		
		ltAi.getRand(item[i]);
		if (i == 0)
		{
			len += sprintf(newRand + len,  "%02u %02u %02u %02u %02u %02u %02u", 
				item[i].red[0], item[i].red[1], item[i].red[2], item[i].red[3],
				item[i].red[4], item[i].red[5], item[i].blue);
		}
		else
		{
			len += sprintf(newRand + len, ":%02u %02u %02u %02u %02u %02u %02u", 
				item[i].red[0], item[i].red[1], item[i].red[2], item[i].red[3],
				item[i].red[4], item[i].red[5], item[i].blue);
		}
	}
	sprintf(newRand + len, "\"}");
	sprintf(idStr, "{id: %d}", lastId + 1);
	dbCon.resetError();
	dbCon.update(_dbRandName, fromjson(idStr), fromjson(newRand), true);
	//dbCon.insert(_dbRandName, fromjson(newRand));
	string err = dbCon.getLastError();
	if (!err.empty())
	{
		LogErr("Insert Rand Err:%s\n", err.c_str());
		return -1;
	}

	LogMsg("Insert Rand:%d to db Success\n", lastId + 1);
	return 0;
}

int NumUpdate::dbInit()
{
	string errMsg;
	DBClientConnection dbCon;
	if(!dbCon.connect(_dbAddr.c_str(), errMsg))
	{
		LogErr("Connect to db %s failed: %s\n", _dbAddr.c_str(), errMsg.c_str());
		return NULL;
	}

	BSONObjBuilder bob;
	bob.append("id", -1);
	bob.append("unique", 1);

	if(!dbCon.ensureIndex(_dbHisName, bob.obj()))
	{
		LogErr("ensureIndex %s failed: %s\n", _dbHisName.c_str(), errMsg.c_str());
		return NULL;
	}
	LogDbg("ensureIndex: %s\n", _dbHisName.c_str());

	BSONObjBuilder bob2;
	bob2.append("id", -1);
	bob2.append("unique", 1);
	if(!dbCon.ensureIndex(_dbRandName, bob2.obj()))
	{
		LogErr("ensureIndex %s failed: %s\n", _dbRandName.c_str(), errMsg.c_str());
		return NULL;
	}
	LogDbg("ensureIndex: %s\n", _dbRandName.c_str());

	dbCon.update(_dbHisName, fromjson("{id: 2003001}"), fromjson("{id: 2003001, num: \"10 11 12 13 26 28 11\"}"), true);
	LogDbg("insert first History: 2003001\n");
	LogMsg("Db init success\n");

	return 0;
}

void* NumUpdate::run(void* arg)
{	
	HttpClient http;
	string page;
	char url[128];
	bool runFirst = true;
	vector<string> newItems;
	int webLastId, dbLastId;
	assert(arg != NULL);
	NumUpdate& numUp = *(NumUpdate*)arg;

	if (numUp.dbInit() < 0)
	{
		return NULL;
	}

	LogMsg("NumUpdate start success\n");

	while(1)
	{
		if (!runFirst)
		{
			time_t curTm = time(NULL);
			struct tm* lcTm = ::localtime(&curTm);
			if ((lcTm->tm_wday == 2 || lcTm->tm_wday == 4 || lcTm->tm_wday == 6)
				&& (lcTm->tm_hour > 21 && lcTm->tm_hour < 23))
			{
				sleep(10 * 60);
			}
			else
			{
				sleep(60 * 60);
			}
		}
		runFirst = false;

 		page.clear();
 		if(http.getPage("datachart.500.com", "/ssq/history/inc/history_same.php", page) != 0)
		{
			LogErr("getPage failed:/ssq/history/inc/history_same.php\n");
			continue;
		}

		if (numUp.get_last_id_from_db(dbLastId) < 0)
		{
			LogErr("get_last_id_from_db failed");
			continue;
		}

		if (numUp.get_last_id_from_html(page, webLastId) < 0)
		{
			LogErr("get_last_id_from_html failed");
			continue;
		}

		dbLastId -= 2000000;
		if (webLastId <= (dbLastId + 1))
		{
			LogMsg("DB Last=%d WEB Last=%d\n", dbLastId + 2000000, webLastId + 2000000);
			continue;
		}
		
		page.clear();
		sprintf(url, "/ssq/history/inc/history_same.php?start=%d&end=%d", dbLastId + 1, webLastId);		
		if(http.getPage("datachart.500.com", url, page) != 0)
		{
			LogErr("getPage failed:%s\n", url);
			continue;
		}
		//cout<<page;

		//从页面解析号码
		if (numUp.get_item_from_page(page, newItems) < 0)
		{
			LogErr("get_item_from_page failed\n");
			continue;
		}

		//存入数据库
		if (numUp.save_item_to_db(newItems) < 0)
		{
			continue;
		}

		//通知AI更新数据
		LtAi& ltAi = LtAi::getInst();
		ltAi.update_history();

		if (numUp.generate_new_rand(webLastId + 2000000 - 1) < 0)
		{
			LogErr("generate_new_rand failed\n");
			continue;
		}
	}
	return NULL;
}

