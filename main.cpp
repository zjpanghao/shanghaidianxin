/*
*    author: chenxun
*/

#include <iostream>
#include <set>
#include <string>
#include <pthread.h>

#include "json_data.h"
#include "KafkaWrapper.h"

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
		int code;
		int i;
		test_info_g = new wrapper_Info;
		init_kafka(test_info_g);
		
		for (i = 0; i < 10; i++)
		{
				code = pthread_create(&tid[i], NULL, thread_function, (void*)i);
				if (code != 0)
				{
						fprintf(stderr, "Create new thread failed: %s\n", strerror(code));
						exit(1);
				}
				fprintf(stdout, "New thread created.\n");
		}

		for (i = 0; i < 10; i++)
		{
				pthread_join(tid[i],NULL);
				fprintf(stderr, "Join thread 1 error: %s\n", strerror(code));
				exit(1);
		}

		return 0;
}


