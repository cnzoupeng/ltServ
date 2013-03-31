
#ifndef __LOTTORY_AI_H__
#define __LOTTORY_AI_H__

#include "lottory.h"
#include <vector>
using namespace std;

class LtAi
{
public:
	LtAi(void);

	virtual ~LtAi(void);

	int load_item(const char* dbAddr, const char* dbName);

	vector<Lottory>& getHis()
	{
		return _ltHis;
	}

	int getRand(Lottory& item);

private:
	int _initBase();

private:
	vector<Lottory> _ltHis;
	u64* _ltBase;
	u32  _ltBaseCount;
};

#endif  //__LOTTORY_AI_H__