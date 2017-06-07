/*
*    author: chenxun
*/

#include <iostream>
#include <set>
#include <string>
#include <pthread.h>
#include "glog/logging.h"
#include "json_data.h"
#include "KafkaWrapper.h"
#include "tele_task_producer.h"
#include "tele_task_consumer.h"
#include "tele_mat.h"
using namespace std;
enum TeleCommand{
	INVALID_COMMAND = 0,
	KAFKA_COMMAND = 1,
	THREAD_NUM_COMMAND = 3,
        DELAY_MIN_CMD = 4
};

wrapper_Info *test_info_g;
bool is_slave_g = false;

static int init_kafka(wrapper_Info *test_info, const char* kafka_server)
{
  int partition = 0;
  const char* topic1 = "test";
  const char* topic2 = "stock_heat";
  /*init kafka*/
  if (PRODUCER_INIT_SUCCESS == producer_init(partition, topic2, kafka_server, &test_msg_delivered, test_info)) {
    LOG(INFO) << "producer init success!";
    return 0;
  } else {
    LOG(ERROR) << "producer init failed!";
    return -1;
  }
  return 0;
}

wrapper_Info * get_kafka_info_g() {
  return test_info_g;
}

static void glog_init(std::string name) {
  google::InitGoogleLogging(name.c_str());
  google::SetLogDestination(google::INFO, "telelog/info");	
  google::SetLogDestination(google::WARNING, "telelog/warn");   
  google::SetLogDestination(google::ERROR, "telelog/error");   
  FLAGS_logbufsecs = 10;
}

bool IsSlave() {
  return is_slave_g;
}

const char *ver = "1.1.2";
int main(int argc, char* argv[]) {
  pthread_t tid[10] = {0};
  int code = 0;
  int i;
  int consumer_threads_num = 1;
  TeleCommand cmd;
  int delay_min = 3;
  const char *kafka_server = "127.0.0.1";
  daemon(1, 0);
  glog_init(argv[0]);
  tele::TeleMem *mem = tele::TeleShmLoad();
  LOG(INFO) << mem;
  for (int i = 0; i < argc; i++) {
    switch (cmd) {
      case KAFKA_COMMAND:
        kafka_server = argv[i];
        break;
      case THREAD_NUM_COMMAND:
        consumer_threads_num = atoi(argv[i]);
	break;
      case DELAY_MIN_CMD:
        delay_min = atoi(argv[i]);
	break;
    }
    cmd = INVALID_COMMAND;
    if (strcmp(argv[i], "-k") == 0) {
      cmd = KAFKA_COMMAND;
    } 
    if (strcmp(argv[i], "-p") == 0) {
      cmd = THREAD_NUM_COMMAND;
    }
    if (strcmp(argv[i], "-m") == 0) {
      cmd = DELAY_MIN_CMD;
    }
    if (strcmp(argv[i], "--slave") == 0) {
      is_slave_g = true;
    }
  }
  LOG(INFO) << ver;
  test_info_g = new wrapper_Info;
  init_kafka(test_info_g, kafka_server);
  pthread_t producerid;
  std::string url_fetch_token = "http://61.129.39.71/telecom-dmp/getToken?apiKey=98f5103019170612fd3a486e3d872c48&sign=6a653929c81a24ba14e41e25b6047e5dec55e76e";
  TeleTaskProducer::GetInstance()->Init(url_fetch_token, delay_min);
  TeleTaskConsumer::GetInstance()->Init(consumer_threads_num);	

  while (true) {
    sleep(1024*1024);
  }
  return 0;
}


