
#ifndef __LT_CALENDAR_H__
#define __LT_CALENDAR_H__

#include <ctime>

class LtCalen
{
public:
	LtCalen();
	~LtCalen(){}

	bool isLeapYear(int year)
	{
		return ((0 == year % 4 && 0 != year % 100) || (0 == year % 400));
	}

	int getDaysFromYearBegin(int year, int month, int day);

	int getWdayOfDay(int year, int month, int day);

	int getLastSsqId(tm* tmDay);

	int getLastSsqId();

	int getNextSsqId()
	{
		return (getLastSsqId() + 1);
	}

	int getDrawTimeOfId(int ssqId, tm& drawTime);

	int getDrawTimeDist(int ssqId);

private:

};

#endif
