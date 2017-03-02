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
#include "KafkaWrapper.h"

std::string get_toKen_by_url(const std::string &url, CURL *cu);

std::string get_value_by_url(const std::string &url, CURL *cu);

void init_kafka();

#endif // JSON_DATA_H
