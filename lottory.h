
#ifndef __LOTTORY_H__
#define __LOTTORY_H__

#include "common.h"
#include <string>
using namespace std;

#define RED_BOLL_NUM 6

class Lottory
{
public:
	//base
	u32 no;
	u8 red[RED_BOLL_NUM];
	u8 blue;

	//sum
	u8 sum_red;
	u8 sum_all;

	//ji ou
	u8 num_ji;
	u8 num_ou;
	u16 map_jo;

	//miss
	u8 map_miss_row;
	u8 num_miss_row;
	u8 map_miss_column;	
	u8 num_miss_column;

	//ac
	u8 ac;

	//É¢¶È
	u8 sandu;

	u64 map;

public:
	Lottory(void);
	~Lottory(void);

	int Set(string& str);

	int Set(const char* str);

	int Set(u64 map);

	char* toString(char* str, int len);

	string& toString(string& str);

	u32 lianHaoCount(Lottory& lt);

	u32 sameRedCount(Lottory& lt)
	{
		return bit_count(map & lt.map & 0x1FFFFFFFF);
	}

	bool operator == (Lottory& lt)
	{
		return sameRedCount(lt) == RED_BOLL_NUM;
	}
private:

	void _calc_ac();
	void _calc_sandu();
	void _calc_miss();
	void _zero();
};


#endif //__LOTTORY_H__
