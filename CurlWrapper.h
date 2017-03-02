#ifndef _CURL_WRAPPER_H_
#define _CURL_WRAPPER_H_

#include "curl/curl.h"
#include <string>
#include <iostream>
using namespace std;

class CurlWrapper
{
public:
	CurlWrapper();
	~CurlWrapper();
	static CurlWrapper* get_instance();
	std::string access_http(const char* szUrl);
    std::string access_http(const char* szUrl, CURL *curl);
    CURL *CreateCurl();
    void FreeCurl(CURL* curl);
	CURL* curl_;
	static CurlWrapper* wrapper_;
};

#endif