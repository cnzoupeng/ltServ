
#include "common.h"

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




