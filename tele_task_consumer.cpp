#include "tele_task_consumer.h"
#include "tele_task_producer.h"
#include "json_data.h"
#include <json/json.h>
#include <json/reader.h>
#include "CurlWrapper.h"
#include "base64.h"
#include "common.h"
#include "KafkaWrapper.h"
TeleTaskConsumer * TeleTaskConsumer::instance_ = NULL;
TeleTaskConsumer::TeleTaskConsumer() {
  url_pre_ = "http://61.129.39.71/telecom-dmp/kv/getValueByKey?token=";
  url_table_ = "&table=kunyan_to_upload_inter_tab_sk&key=";
  url_flag_ = "_kunyan_";
}

TeleTaskConsumer* TeleTaskConsumer::GetInstance() {
  if (instance_ == NULL) {
    instance_ = new TeleTaskConsumer();
  }
  return instance_;
}

int TeleTaskConsumer::Init(int nthread) {
 pthread_t *consumer_id = new pthread_t[nthread];
 int i;
 for (i = 0; i < nthread; i++) {
   pthread_create(&consumer_id[i], NULL, TeleConsumerThread, 0); 
 }
 return 0; 
}
void *TeleTaskConsumer::TeleConsumerThread(void *arg) {
  TeleTaskProducer *producer = TeleTaskProducer::GetInstance();
  TeleTaskConsumer *consumer = TeleTaskConsumer::GetInstance();
  while (true) {
    TeleTask *task = producer->GetTask();
    if (task == NULL) {
      printf("fetch NUll task\n");
      sleep(1);
      continue;
    }
    printf("fetch task %d\n", task->index);
    std::string url = consumer->BuildUrlFromTask(task);
    std::string value = get_value_by_url(url);    
    if (value == "") {
      printf("fetch http error index:%d, url:%s\n", task->index, url.c_str());
      delete task;
      continue;
    }
    if (task->index == 0) {
      printf("Fetch url:%s value:%s\n", url.c_str(), value.c_str());
    }
    std::string json_value = consumer->GetJsonValue(value);
    if (json_value == "") {
      delete task;
      continue;
    }

    if (json_value == "error" || json_value == "errortoken" ) {
      printf("json value error: %s url:%s\n", value.c_str(), url.c_str());
      if (task->index == 0 && json_value == "errortoken")
        producer->SetIdentityError();
      delete task;
      continue;
    }
     
    std::string base64_result = consumer->DeBase64(json_value); 
    if (base64_result.length() == 0) {
      printf("process error value:%s \n", json_value.c_str());
        delete task;
        continue;
    }
    if (consumer->Store(base64_result.c_str(), base64_result.length()) != 0) {
      printf("push data failed time:%s, value:%s\n", task->time_str.c_str(), base64_result.c_str());
    } else {
      printf("push data success time: %s value:%s\n", task->time_str.c_str(), base64_result.c_str());
    }
    delete task; 
  }
  

}
std::string TeleTaskConsumer::GetJsonValue(std::string value) {
  Json::Reader reader; 
  Json::Value root;
    if (!reader.parse(value, root)) 
      return "error";
    if (root["message"].isNull()) {
      return "error";
    }
    if (root["message"].asString() != "OK" ) {
      return "errortoken";
    }
    if (root["result"].isNull())
      return "";
    if (root["result"]["value"].isNull())
      return "";
   std::string  json_value = root["result"]["value"].asString();
    if (json_value.length() == 0 || json_value.length() >= 100)
      return "";
    return json_value;
}
std::string TeleTaskConsumer::BuildUrlFromTask(TeleTask *task) {
  char index_str[20];
  sprintf(index_str, "%d", task->index);
  return url_pre_ + task->token + url_table_ + task->time_str + url_flag_ + index_str;
}

int TeleTaskConsumer::Store(const char *value, int len) {
    wrapper_Info *test_info = get_kafka_info_g();
    if (PUSH_DATA_SUCCESS == producer_push_data(value, strlen(value), test_info))
      return 0;
    return -1;
}
std::string TeleTaskConsumer::DeBase64(std::string value) {
    char base64result[256];
    int len = Uncompressbase64(value.c_str(), value.length(), (unsigned char*)base64result, 255);
    base64result[len] = 0;
    char *push_data = base64result;
    if (push_data[len - 1] == '\n')
          push_data[--len] = '\0';
    return push_data;
    wrapper_Info *test_info = get_kafka_info_g();
    //if (PUSH_DATA_SUCCESS == producer_push_data(push_data, strlen(push_data), test_info))
}
