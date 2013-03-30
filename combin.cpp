
#include "combin.h"
#include <cassert>
#include <cstdlib>

Combin::Combin(u32 base, u32 sub)
{
	assert(base < 65 && sub < 20);
	const u64 bit1 = 1;
	m_base = base > sub ? base : sub;
	m_sub  = base < sub ? base : sub;	
	m_curOne = (bit1 << sub) - 1;
	m_lastOne = m_curOne << (m_base - m_sub);
	m_count = Combin::calc(m_base, m_sub);
}


Combin::~Combin(void)
{
}

bool Combin::getNext(u64& next)
{
	u64 tmpmap = 0;
	const u64 one = 1;	

	if (m_curOne <= m_lastOne)
	{
		//return current one
		next = m_curOne;

		//get next map
		for (u32 i = 0; i < m_base; i++)
		{
			if ((m_curOne & (one << i)) && ((m_curOne & (one << (i + 1))) == 0))
			{
				m_curOne &= ~(one << i);
				m_curOne |= one << (i + 1);

				if(i > 1)
				{
					tmpmap = m_curOne & ((one << i) - 1);
					int bit1 = bit_count(tmpmap);
					m_curOne &= ~((one << i) - 1);
					m_curOne |=  ((one << bit1) - 1);
				}
				break;
			}
		}//end for
	}
	else
	{
		return false;
	}

	return true;
}


u32 Combin::calc(int base, int sub)
{
	int bigNum = base;
	u64 ret =  bigNum;
	u64 zero = 0;
	u64 one = 1;
	u64 smap = 0;
	u64 smRet = 1;
	int bit = (base - sub) > sub ? sub : (base - sub);
	smap = (one << bit) - 1;

	int bitBig = bit;
	int bitSmall = bit;
	while (bitBig > 1)
	{
		//找到所有能够整除的数 然后整除 
		for (int i = bitSmall;(i > 0 && smap > 0); i--)
		{
			if(((smap & (one << (i - 1))) > 0) && ret % i == 0)
			{
				ret /= i;
				smap &= ~(one << (i - 1));
			}
		}

		bitBig --;
		bigNum --;

		//越界
		if((~zero / ret) < (u32)bigNum)
		{
			LogErr("Big out aero\n");
			abort();
			return 0;
		}

		ret *= bigNum;

		//找到所有能够整除的数 然后整除 
		for (int i = bitSmall;(i > 0 && smap > 0); i--)
		{
			if(((smap & (one << (i - 1))) > 0) && ret % i == 0)
			{
				ret /= i;
				smap &= ~(one << (i - 1));
			}
		}
	}

	//计算所有未整除的数字 的剩积
	for (int j = bitSmall ; (j > 0 && smap > 0); j--)
	{
		if(((smap & (one << (j - 1))) > 0))
		{
			if((~zero / smRet) < (u32)j)
			{
				LogErr("Small out aero\n");
				abort();
				return 0;
			}
			smRet *= j;
			smap &= ~(one << (j - 1));
		}
	}

	ret /= smRet;

	//printf("combination(%d, %d)=%llu\n", base, sub, ret);
	return (u32)ret;
}


