
#ifndef __NUM__UPDATE_H__
#define __NUM__UPDATE_H__

#include "common.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

#define HISTORY_FILE "./history.txt"

struct HisItem
{
	int id;
	char num[40];
};

class NumUpdate
{
public:
	static NumUpdate& getInst()
	{
		static NumUpdate numUp;
		return numUp;
	}

	int runBack();

	int runLoop();

	int get_range_str(int start, int stop, string& outStr);

private:
	NumUpdate(){};

	~NumUpdate(){}

	static void* run(void*);

	int load_from_file(const char* fileName = HISTORY_FILE);

	int write_to_file(const char* fileName = HISTORY_FILE);

	int get_xml_value(string& xml, char* value);

	int get_last_id_from_html(string& page, int& lastId);

	int get_item_from_page(string& page, vector<string>& items);

	vector<HisItem> _history;
	map<int, int>	_mapHisId;
};

#endif

