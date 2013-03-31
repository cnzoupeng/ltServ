
#include "ltAi.h"
#include "mongo/client/dbclient.h"
#include "cmRandom.h"
#include "combin.h"
#include <string.h>
#include <cerrno>
#include <cstdlib>

using namespace mongo;

LtAi::LtAi(void)
{
	_ltBase = NULL;
	_ltBaseCount = 0;
}


LtAi::~LtAi(void)
{

}

int LtAi::load_item(const char* dbAddr, const char* dbName)
{
	char ltIdBuf[32];
	Lottory item;
	string strItem;	
	string errMsg;
	mongo::Query qSort;
	int err = 0;

	mongo::DBClientConnection dbCon;
	if(!dbCon.connect(dbAddr, errMsg))
	{
		LogErr("Connect to db %s failed: %s\n", dbAddr, errMsg.c_str());
		return -1;
	}

	qSort.sort("id", 1);
	std::auto_ptr<DBClientCursor> cursor = dbCon.query(dbName , qSort);	
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
		_ltHis.push_back(item);
		//ltt.dump_item(item);
	}

	if (err != 0)
	{
		_ltHis.clear();
		return err;
	}

	//
	if (_initBase() < 0)
	{
		return -1;
	}

	LogMsg("Load %u History items\n", (u32)_ltHis.size());
	return 0;
}

int LtAi::_initBase()
{	
	if (_ltBase == NULL)
	{
		Combin cmb(33, 6);
		_ltBaseCount = cmb.getCount();
		_ltBase = new u64[_ltBaseCount];
		if (_ltBase == NULL)
		{
			LogErr("Alloc %u failed:%s\n", _ltBaseCount, strerror(errno));
			return -1;
		}

		u32 index = 0;
		while(cmb.getNext(_ltBase[index]) && index < _ltBaseCount)
		{
			index++;
		}

		if (index != _ltBaseCount)
		{
			LogErr("ID=%d All=%u\n", index, _ltBaseCount);
			return -1;
		}
	}

	return 0;
}

int LtAi::getRand(Lottory& item)
{
	u32 rdIndex = CmRandom::getUint();
	rdIndex %= _ltBaseCount;

	u64 rdMap = _ltBase[rdIndex];
	if(item.Set(rdMap) < 0)
	{
		abort();
	}
	return 0;
}

