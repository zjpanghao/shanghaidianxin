/*
*  Created on: 2016.2.1
*      Author: chenxun
*/
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "common.h"
#include "CurlWrapper.h"
#include "json_data.h"
#include <json/json.h>
#include <json/reader.h>
using namespace std;

std::string get_toKen_by_url(const std::string &url) {
	std::string toKen = "";
	Json::Value root;

	string str = "";
	str = CurlWrapper::get_instance()->access_http(url.c_str());
	Json::Reader reader; 
	if (str.length() == 0)
		return "";
	if (!reader.parse(str, root)) {
		return "";
	}
	if (root["result"].isNull())
		return "";
	toKen = root["result"].asString();
	return toKen;
}

std::string get_value_by_url(const std::string &url) {
  std::string value = "";
  Json::Value root;

  string str = "";
  str = CurlWrapper::get_instance()->access_http(url.c_str());
  if (str == "") {
    printf("access %s failed\n", url.c_str());
  }
  return str;
}

int init_kafka(wrapper_Info *test_info) {
  int partition = 0;
  const char* topic1 = "test";
  const char* topic2 = "stock_heat";
  /* init kafka */
  if (PRODUCER_INIT_SUCCESS == producer_init(partition, topic2, "222.73.34.101:9092", &test_msg_delivered, test_info)) {
    printf("producer init success!\n");
    return 0;
  }  else {
     printf("producer init failed\n");
     return -1;
  }
  return 0;
}

