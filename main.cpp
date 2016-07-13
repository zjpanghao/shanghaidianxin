/*
*    author: chenxun
*/

#include <iostream>
#include <set>
#include <string>
#include <pthread.h>

#include "json_data.h"
#include "KafkaWrapper.h"
#include "tele_task_producer.h"
#include "tele_task_consumer.h"
using namespace std;

wrapper_Info *test_info_g;
static int init_kafka(wrapper_Info *test_info)
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
	return 0;
}

wrapper_Info * get_kafka_info_g() {
	return test_info_g;
}
int main(int argc, char* argv[])
{
		pthread_t tid[10] = {0};
		int code = 0;
		int i;
		test_info_g = new wrapper_Info;
		init_kafka(test_info_g);
    pthread_t producerid;
    std::string url_fetch_token = "http://61.129.39.71/telecom-dmp/getToken?apiKey=98f5103019170612fd3a486e3d872c48&sign=6a653929c81a24ba14e41e25b6047e5dec55e76e";
    TeleTaskProducer::GetInstance()->Init(url_fetch_token);
	  TeleTaskConsumer::GetInstance()->Init(10);	

    while (true) {
      sleep(1024*1024);
    }
		return 0;
}


