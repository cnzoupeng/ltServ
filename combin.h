#pragma once

#include "common.h"

class Combin
{
public:
	Combin(u32 base = 33, u32 sub = 6);
	~Combin(void);

	bool getNext(u64& next);

	u32 getCount()
	{
		return m_count;
	}

	u64 getFirst()
	{
		return (((u64)(0x1) << m_sub) - 1);
	}

	u64 getLast()
	{
		return m_lastOne;
	}
	
	static u32 calc(int base, int sub);


private:
	u32 m_base;
	u32 m_sub;
	u32 m_count;
	u64 m_curOne;
	u64 m_lastOne;
};

