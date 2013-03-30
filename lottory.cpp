
#include "lottory.h"
#include <vector>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <boost/algorithm/string.hpp>

Lottory::Lottory(void)
{
	_zero();
}

Lottory::~Lottory(void)
{
}

void Lottory::_zero()
{
	memset(this, 0, sizeof(Lottory));
}

void Lottory::_calc_ac()
{
	u64 acmap = 0;
	u64 one = 1;
	int ac_val = 0;

	int j = 0;
	for(int i = 0; i < 5; i++)
	{
		for(j = i + 1; j < 6; j++)
		{
			ac_val = abs(red[j] - red[i]);
			acmap |= one << (ac_val - 1);
		}
	}

	this->ac = bit_count(acmap) - 5;
}

void Lottory::_calc_sandu()
{
	int max = 0;
	for(int i = 1; i < 34; i++)
	{		
		int min = 0xFF;
		int cal = 0;
		for(int j = 0; j < 6; j++)
		{
			cal = abs(i - this->red[j]);
			if (cal < min)
			{
				min = cal;
			}
		}

		if(min > max)
		{
			max = min;
		}
	}
	this->sandu = max;
}

void Lottory::_calc_miss()
{
	const u64 one = 1;
	u64 missBase = 0x3F;
	for (int i = 0; i < 6; i++)
	{
		if ((this->map & missBase) == 0)
		{
			this->map_miss_row |= 	one << i;
			this->num_miss_row ++;
		}
		missBase = missBase << 6;
	}

	missBase = 0x555;
	for(int i = 0; i < 3; i++)
	{
		if ((this->map & missBase) == 0)
		{
			this->map_miss_column |= one << i;
			this->num_miss_column ++;
		}

		missBase = missBase << 1;
		if ((this->map & missBase) == 0)
		{
			this->map_miss_column |= one << (i + 3);
			this->num_miss_column ++;
		}
		missBase = missBase << 11;
	}
}

int Lottory::Set(string& str)
{
	int rsum = 0;	
	const u64 one = 1;

	vector<string> vec;
	boost::algorithm::split(vec, str, boost::algorithm::is_any_of(" "));  
	if (vec.size() != 8)
	{
		LogErr("wrong str=%s\n", str.c_str());
		return -1;
	}

	_zero();
	this->no = atoi(vec[0].c_str());
	this->blue = atoi(vec[7].c_str());
	if (this->blue > 16)
	{
		LogErr("wrong str=%s\n", str.c_str());
		return -1;
	}

	for (int j = 0; j < 6; j++)
	{
		this->red[j] = atoi(vec[j + 1].c_str());
		if (this->red[j] > 33)
		{
			LogErr("wrong str=%s\n", str.c_str());
			return -1;
		}

		rsum += this->red[j];
		this->map |= one << (this->red[j] - 1);
		if ((this->red[j] % 2) > 0)
		{
			this->map_jo |= one << j;
		}
	}

	if (bit_count(this->map) != 6)
	{
		LogErr("wrong str=%s\n", str.c_str());
		return -1;
	}

	if(this->blue > 0)
	{
		this->map |= one << (this->blue + 32);
	}

	this->sum_red = rsum;
	this->sum_all = rsum + this->blue;
	if ((this->blue % 2) > 0)
	{
		this->map_jo |= one << 6;
	}

	sort(red, red + RED_BOLL_NUM, less<u8>());

	_calc_ac();
	_calc_sandu();
	_calc_miss();
	return 0;
}

int Lottory::Set(u64 map)
{
	int redId = 0;
	const u64 bit1 = 1;

	if (bit_count(map & 0x1FFFFFFFF) != 6)
	{
		return -1;
	}
	if (bit_count(map & 0x1FFFE00000000) > 1)
	{
		return -1;
	}

	//red
	_zero();
	for(int i = 0; i < 33; i++)
	{
		if ((bit1 << i ) & (map))
		{
			this->red[redId] = i + 1;
			this->sum_red += this->red[redId];

			if ((this->red[redId] % 2) > 0)
			{
				this->map_jo |= bit1 << redId;
			}

			redId ++;
			if (redId == 6)
			{
				break;
			}			
		}
	}

	//blue
	for (int i = 33; i < 49; i++)
	{
		if ((bit1 << i ) & (map))
		{
			this->blue = i - 32;
			break;
		}
	}

	//
	this->no = 2022666;

	//
	this->sum_all = this->sum_all + this->blue;
	if (this->blue > 0 && (this->blue % 2) > 0)
	{
		this->map_jo |= bit1 << 6;
	}

	sort(red, red + RED_BOLL_NUM, less<u8>());

	_calc_ac();
	_calc_sandu();
	_calc_miss();
	return 0;
}

char* Lottory::toString(char* str, int len)
{
	if (len < 32)
	{
		return NULL;
	}
	snprintf(str, len, "%u %02u %02u %02u %02u %02u %02u %02u",
		no, red[0], red[1], red[2], red[3], red[4], red[5], blue);
	return str;
}

string& Lottory::toString(string& str)
{
	char tmpStr[40];
	snprintf(tmpStr, sizeof(tmpStr), "%u %02u %02u %02u %02u %02u %02u %02u",
		no, red[0], red[1], red[2], red[3], red[4], red[5], blue);
	str = tmpStr;
	return str;
}

u32 Lottory::lianHaoCount(Lottory& lt)
{
	return 0;
}

