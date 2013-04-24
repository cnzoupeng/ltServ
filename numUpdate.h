
#ifndef __NUM__UPDATE_H__
#define __NUM__UPDATE_H__

#include <string>
#include <vector>
using namespace std;

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

private:
	NumUpdate();

	~NumUpdate(){}

	static void* run(void*);

	int dbInit();

	int get_xml_value(string& xml, char* value);

	int get_last_id_from_db(int& lastId);

	int get_last_id_from_html(string& page, int& lastId);

	int get_item_from_page(string& page, vector<string>& items);

	int save_item_to_db(vector<string>& items);

	int generate_new_rand(int lastId);

private:
	string _dbAddr;
	string _dbHisName;
	string _dbRandName;
};

#endif

