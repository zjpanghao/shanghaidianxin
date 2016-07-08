/*
*  Created on: 2016.2.1
*      Author: chenxun
*/
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "json_data.h"
//#include "base64.h"
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "common.h"
#include "CurlWrapper.h"
using namespace std;
#if 0
size_t http_data_writer(void* data, size_t size, size_t nmemb, void* content)
{
	long totalSize = size*nmemb;
	std::string* symbolBuffer = (std::string*)content;
	if (symbolBuffer)
	{
		symbolBuffer->append((char *)data, ((char*)data) + totalSize);
	}
	return totalSize;
}

std::string http_access(const char* szUrl)
{
	std::string strData = "";
	CURL* curl = NULL;
	curl = curl_easy_init();
	CURLcode code;

	/*SET connect type: keep-alive*/
	struct curl_slist *header_list = NULL;
	header_list = curl_slist_append(header_list, "Connection: keep-alive");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

	code = curl_easy_setopt(curl, CURLOPT_URL, szUrl);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_data_writer);

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strData);
	code = curl_easy_perform(curl);
	if (code == CURLE_OK)
	{
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		if (responseCode < 200 || responseCode >= 300 || strData.empty())
		{
			std::cout << "code != CURLE_OK" << std::endl;
		}
		if (responseCode != 0) {
			strData = "";
		}
	}
	curl_slist_free_all(header_list);
	curl_easy_cleanup(curl);
	return strData;
}
#endif
#if 0
Json::Value get_json_array(const std::string &str)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(str, root, false))
	{
		std::cout << "--Plz check your url make sure your url is available--" << std::endl;
		return 0;
	}

	return root;
}
#endif
std::string get_toKen_by_url(const std::string &url)
{
	std::string toKen = "";
	Json::Value root;

	string str = "";
	str = CurlWrapper::get_instance()->access_http(url.c_str());
	
	printf("The str is %s\n", str.c_str());
	Json::Reader reader; 
	if (str.length() == 0)
		return "";
	if (!reader.parse(str, root)) {
		return "";
	}
	printf("The str is %s\n", str.c_str());
	if (root["result"].isNull())
		return "";
	toKen = root["result"].asString();
	//std::cout << toKen << std::endl;
	return toKen;
}

std::string int2str(const int &int_temp)
{
	stringstream stream;
	stream << int_temp;
	std::string string_temp = stream.str();   //此处也可以用 stream>>string_temp
	return string_temp;
}

std::string get_str_by_time()
{
	string str = "";
	const time_t t = time(NULL);
	struct tm now_time;
	localtime_r(&t, &now_time);
	struct tm* current_time = &now_time;
	while (1)
	{
		const time_t t1 = time(NULL);
		struct tm now_time1;
		localtime_r(&t1, &now_time1);
		struct tm* current_time1 = &now_time1;
		if (current_time1->tm_sec < 30)
		{
			//std::cout << "***wait the data of Shanghai telecom server****" << std::endl;
			sleep(2);
		}
		else
			break;
	}

	char buffer[13] = { 0 };
	sprintf(buffer, "%d%02d%02d%02d%02d",
		current_time->tm_year + 1900,
		current_time->tm_mon + 1,
		current_time->tm_mday,
		current_time->tm_hour,
		current_time->tm_min);


	str = buffer;
	//std::cout << str << "-----------" << str.size() << std::endl;
	return str;
}

std::string get_url_by_toKen(const std::string &url_toKen,std::string &last_time)
{
	std::string url1_ip = "http://61.129.39.71/telecom-dmp/kv/getValueByKey?token=";
	//std::string url2_toKen = "";
	std::string url3_table = "table=kunyan_to_upload_inter_tab_sk&key=";
	std::string url4_key = "";
	std::string url5_key = "_kunyan_";

	//std::cout<<"last time: "<<last_time<<std::endl;
	std::string temp = "";
	while (1)
	{
		temp = get_str_by_time();
		if (last_time != temp)
		{
			last_time = url4_key = temp;
			break;
		}
		else
			sleep(1);
	}
	std::cout<<"the run time:"<<temp<<std::endl;
	static std::string url2_toKen = "";
	if (url2_toKen == "") {
		url2_toKen = get_toKen_by_url(url_toKen);
		if (url2_toKen == "") {
		  printf("acquire token %s failed\n", url_toKen.c_str());
			return "";
		}
	}
	//std::cout << "The toKen is :" << url2_toKen << std::endl;
	//std::cout << "the thread id:" << pthread_self() << std::endl;
	std::string url = url1_ip + url2_toKen + "&" + url3_table + url4_key + "_kunyan_";
	//std::cout << url << std::endl;
	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;

	//std::vector<std::string>::iterator it;
	//for (it = url_vec.begin(); it != url_vec.end(); it++)
	//{
	//	std::cout << *it << std::endl;
	//}

	return url;
}

std::string get_value_by_url(const std::string &url)
{
	std::string value = "";
	Json::Value root;

	string str = "";
	str = CurlWrapper::get_instance()->access_http(url.c_str());
	if (str == "") {
		printf("access %s failed\n", url.c_str());
	}
	return str;
}

int Uncompressbase64(const char*input, int inlen, unsigned char *output, int outlen) {
	int len = 0;
	BIO *b64,*bmem;
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf((void*)input, inlen);
	bmem = BIO_push(b64, bmem);
	len = BIO_read(bmem, output, outlen);
	output[len] = 0;
	BIO_free_all(bmem);
	return len;
}

std::set<std::string> get_data_from_shanghai(int flag, std::string &last_update_time)
{
	std::string url_ = "";
	std::string token;
	std::string value = "";
	std::vector<std::string> url_vec;
	std::set<std::string> value_set;
	std::string value_base64 = "";
	std::string url_1 = "http://61.129.39.71/telecom-dmp/getToken?apiKey=98f5103019170612fd3a486e3d872c48&sign=6a653929c81a24ba14e41e25b6047e5dec55e76e";

	int index = flag * 1000;
	int start_index = index;
	int end_index = index + 1000;
	url_ = get_url_by_toKen(url_1, last_update_time);
	if (url_ == "")
		return value_set;
	for (int i = start_index; i < end_index; i++)
	{
		char temp[10];
		sprintf(temp, "%d", i);
		string index_str = temp;
		std::string url = url_ + index_str;
		//std::cout<<url<<std::endl;
		value = get_value_by_url(url);
		Json::Reader reader; 
		Json::Value root;
		if (!reader.parse(value, root)) 
			continue;
		if (root["result"].isNull())
		continue;
		if (root["result"]["value"].isNull())
		continue;
		value = root["result"]["value"].asString();
		if (value.length() == 0 || value.length() >= 100)
			continue;
		char base64result[256];
		int len = Uncompressbase64(value.c_str(), value.length(), (unsigned char*)base64result, 255);
		base64result[len] = 0;
		value_base64 = base64result;
		//value_base64 = base64_decode(value);
		value_set.insert(base64result);
		value_base64 = value_base64 + " ||the url: " + url;
		std::cout << value_base64 << std::endl;
	}

	//std::cout << "size  of value_set: " << value_set.size() << std::endl;

	return value_set;
}

int init_kafka(wrapper_Info *test_info)
{
	int partition = 0;
  const char* topic1 = "test";
  const char* topic2 = "stock_heat";
	/*init kafka*/
	if (PRODUCER_INIT_SUCCESS == producer_init(partition, topic2, "222.73.34.101:9092", &test_msg_delivered, test_info)) {
		printf("producer init success!\n");
		return 0;
	}
	else {
		printf("producer init failed\n");
		return -1;
	}
}



void* thread_function(void *arg)
{
   // init_kafka(test_info);
   wrapper_Info *test_info = get_kafka_info_g();
	int td = (long)arg;
	char push_data[100] = { 0 };
	std::string last_update_time = "";
	std::cout << td <<std::endl;
	while (true)
	{
		int data_cnt = 0;
		std::set<std::string> value_set;
		value_set = get_data_from_shanghai(td,last_update_time);

		set<std::string>::iterator it;
		for (it = value_set.begin(); it != value_set.end(); it++)
		{
			if (*it != "")
			{
				data_cnt++;
				strcpy(push_data, (*it).c_str());
				size_t len = strlen(push_data);
				if (push_data[len - 1] == '\n')
					push_data[--len] = '\0';
				//producer_push_data(push_data, strlen(push_data), test_info);
				if (PUSH_DATA_SUCCESS == producer_push_data(push_data, strlen(push_data), test_info))
					printf("push data success:%s, len:%d\n", push_data, len);
				else
					printf("push data failed %s, len:%d\n", push_data, len);
			}
		}

		printf("------------------------------------------------------------------------------\n");
		printf("td %d last_update_time %s push data success and %d messages are delivered\n", td,last_update_time.c_str(), data_cnt);
		printf("------------------------------------------------------------------------------\n");
	}
	//producer_close(test_info_g);
	pthread_exit((void*) 0);
}
