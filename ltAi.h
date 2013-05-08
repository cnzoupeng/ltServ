
#ifndef __LOTTORY_AI_H__
#define __LOTTORY_AI_H__

#include "lottory.h"
#include <vector>
#include <string>
using namespace std;

class LtAi
{
public:
	static LtAi& getInst()
	{
		static LtAi ai;
		return ai;
	}

	int load_history();

	int update_history();

	int update_history(string num);

	vector<Lottory>& getHis()
	{
		return _ltHis;
	}

	int getRand(Lottory& item);

private:

	LtAi();

	virtual ~LtAi(void);

	int _initBase();

private:
	u64* _ltBase;
	u32  _ltBaseCount;
	
	string _dbAddr;
	string _dbName;

	vector<Lottory> _ltHis;
};

#endif  //__LOTTORY_AI_H__