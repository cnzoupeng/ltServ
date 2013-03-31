
#ifndef __LOTTORY_AI_H__
#define __LOTTORY_AI_H__

#include "lottory.h"
#include <vector>
#include <string>
using namespace std;

class LtAi
{
public:
	LtAi(const char* dbAddr, const char* dbName);

	virtual ~LtAi(void);

	int load_history();

	int update_history();

	vector<Lottory>& getHis()
	{
		return _ltHis;
	}

	int getRand(Lottory& item);

private:
	int _initBase();

private:
	u64* _ltBase;
	u32  _ltBaseCount;
	
	string _dbAddr;
	string _dbName;

	vector<Lottory> _ltHis;
};

#endif  //__LOTTORY_AI_H__