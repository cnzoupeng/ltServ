
#include "ltCalendar.h"
#include <cassert>
#include "common.h"

LtCalen::LtCalen()
{
}

int LtCalen::getDaysFromYearBegin(int year, int month/*0-11*/, int day/*1-31*/)
{
	assert(month >=0 && month < 12);
	assert(day >=0 && day < 32);

	static int _monthDays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	static int _monthDaysLeap[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

	if (isLeapYear(year))
	{
		return (_monthDaysLeap[month] + day);
	}
	return (_monthDays[month] + day);
}

int LtCalen::getWdayOfDay(int year, int month, int day)
{
	assert(month >=0 && month < 12);
	assert(day >=0 && day < 31);
	assert(year > 2000);

	//from 2001.1.1 monday
	int allDays = 0;

	for (int i = 2001; i < year; i++)
	{
		if (isLeapYear(i))
		{
			allDays += 366;
		}
		else
		{
			allDays += 365;
		}
	}

	allDays += getDaysFromYearBegin(year, month, day);

	return (allDays % 7);
}

int LtCalen::getLastSsqId(struct tm* tmDay)
{
	static int firstWeekIdCount[7] = {1, 3, 3, 2, 2, 1, 1};
	static int firstWeekDays[7] = {1, 7, 6, 5, 4, 3, 2};
	static int WeekIdCount[7] = {3, 0, 1, 1, 2, 2, 2};

	if (tmDay->tm_year != 2013)
	{
		LogErr("Year must == 2013\n");
		return -1;
	}

	int index = tmDay->tm_year * 1000;
	int firstDayWday = getWdayOfDay(tmDay->tm_year, 0, 1);
	int daysFromYear = getDaysFromYearBegin(tmDay->tm_year, tmDay->tm_mon, tmDay->tm_mday);

	//计算第一周的期数
	if (daysFromYear <= firstWeekDays[firstDayWday])
	{
		index += WeekIdCount[firstDayWday];
		if ((tmDay->tm_wday == 2 || tmDay->tm_wday == 4 || tmDay->tm_wday == 6)
			&& ((tmDay->tm_hour ==  20 && tmDay->tm_min < 29) || tmDay->tm_hour < 20))
		{
			index -= 1;
		}

		//上一年的最后一期
		if (index % 1000 == 0)
		{
			tmDay->tm_year -= 1;
			tmDay->tm_mon = 11;
			tmDay->tm_mday = 31;
			tmDay->tm_hour = 23;
			return getLastSsqId(tmDay);
		}
		return index;
	}

	//计算第一周之后的期数
	int daysLeft = daysFromYear - firstWeekDays[firstDayWday];
	index += firstWeekIdCount[firstDayWday];
	index += (daysLeft / 7) * 3;
	index += WeekIdCount[(daysLeft % 7)];

	if ((tmDay->tm_wday == 2 || tmDay->tm_wday == 4 || tmDay->tm_wday == 6)
		&& ((tmDay->tm_hour ==  20 && tmDay->tm_min < 29) || tmDay->tm_hour < 20))
	{
		index -= 1;
	}

	index -= 3;
	return index ;
}

int LtCalen::getLastSsqId()
{
	time_t curTm = time(NULL);
	struct tm* lcTm = ::localtime(&curTm);
	lcTm->tm_year += 1900;
	return getLastSsqId(lcTm);
}

int LtCalen::getDrawTimeOfId(int ssqId, tm& drawTime)
{
	static int firstWeekIdCount[7] = {1, 3, 3, 2, 2, 1, 1};
// 	static int firstWeekDays[7] = {1, 7, 6, 5, 4, 3, 2};
// 	static int WeekIdCount[7] = {3, 0, 1, 1, 2, 2, 2};

	if(ssqId < 2013001)
	{
		LogErr("Wrong ssqid %d\n", ssqId);
		return -1;
	}
	
	int year = ssqId / 1000;
	int index = ssqId - (year * 1000);
	if (index == 0)
	{
		LogErr("Wrong ssqid %d\n", ssqId);
		return -1;
	}

	drawTime.tm_year = year;
	drawTime.tm_hour = 20;
	drawTime.tm_min = 29;
	int firstDayWday = getWdayOfDay(year, 0, 1);
	if (index <= firstWeekIdCount[firstDayWday])
	{
		drawTime.tm_mon = 0;
		if (index == 1)
		{
			drawTime.tm_mday = 2 - firstDayWday;
		}
		else if(index == 2)
		{
			drawTime.tm_mday = 4 - firstDayWday;
		}
		else if(index == 3)
		{
			drawTime.tm_mday = 4 - firstDayWday;
		}
	}
	return 0;
}

int LtCalen::getDrawTimeDist(int ssqId)
{
	tm tm_drawTime;
	time_t drawTime;
	time_t curTm = time(NULL);

	if (getDrawTimeOfId(ssqId, tm_drawTime) < 0)
	{
		return -1;
	}

	drawTime = mktime(&tm_drawTime);

	return drawTime - curTm;
}




