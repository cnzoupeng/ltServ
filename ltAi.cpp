
#include "ltAi.h"
#include "cmRandom.h"
#include "combin.h"
#include <string.h>
#include <cerrno>
#include <cstdlib>


LtAi::LtAi(void)
{
	_ltBase = NULL;
	_ltBaseCount = 0;
	initBase();
}


LtAi::~LtAi(void)
{

}

int LtAi::initBase()
{	
	if (_ltBase == NULL)
	{
		Combin cmb(33, 6);
		_ltBaseCount = cmb.getCount();
		_ltBase = new u64[_ltBaseCount];
		if (_ltBase == NULL)
		{
			LogErr("Alloc %u failed:%s\n", _ltBaseCount, strerror(errno));
			abort();
		}

		u32 index = 0;
		while(cmb.getNext(_ltBase[index]) && index < _ltBaseCount)
		{
			index++;
		}

		if (index != _ltBaseCount)
		{
			LogErr("ID=%d All=%u\n", index, _ltBaseCount);
			abort();
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

