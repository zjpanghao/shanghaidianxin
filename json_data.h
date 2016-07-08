/*
*
*  Created on: 2016.1.30
*      Author: chen
*/

#ifndef JSON_DATA_H_INCLUDED
#define JSON_DATA_H_INCLUDED

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <sstream>

#include "curl/curl.h"
#include "json.h"
#include "KafkaWrapper.h"

size_t http_data_writer(void* data, size_t size, size_t nmemb, void* content);

std::string http_access(const char* szUrl);

Json::Value get_json_array(const std::string &str);

std::string get_toKen_by_url(const std::string &url);

std::string int2str(const int &int_temp);

std::string get_str_by_time();

std::string get_url_by_toKen(const std::string &url_toKen, std::string &last_time);

std::string get_value_by_url(const std::string &url);

std::set<std::string> get_data_from_shanghai(int flag, std::string &last_update_time);
void init_kafka();
void* thread_function(void *arg);

#endif // JSON_DATA_H
