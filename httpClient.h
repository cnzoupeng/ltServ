
#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <string>
using namespace std;

class HttpClient
{
public:
	HttpClient(){}
	~HttpClient(){}

	int getPage(const char* host, const char* path, string& page);

};

#endif
