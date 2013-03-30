
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

	int initBase();

	vector<Lottory>& getHis()
	{
		return _ltHis;
	}

	int getRand(Lottory& item);

private:
	vector<Lottory> _ltHis;
	u64* _ltBase;
	u32  _ltBaseCount;
};

#endif  //__LOTTORY_AI_H__