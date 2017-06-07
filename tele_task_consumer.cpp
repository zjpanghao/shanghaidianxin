#include "tele_task_consumer.h"
#include "tele_task_producer.h"
#include "json_data.h"
#include <json/json.h>
#include <json/reader.h>
#include "glog/logging.h"
#include "CurlWrapper.h"
#include "base64.h"
#include "common.h"
#include "KafkaWrapper.h"
#include "tele_mat.h"
#define MAX_DELAY_SIZE 20000
TeleTaskConsumer * TeleTaskConsumer::instance_ = NULL;
TeleTaskConsumer::TeleTaskConsumer():total_fail_first_(0), total_refetch_success_(0) {
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
 pthread_t refetch_id;
 int i;
 pthread_mutex_init(&fail_task_mutex_, NULL);
 for (i = 0; i < nthread; i++) {
   pthread_create(&consumer_id[i], NULL, TeleConsumerThread, 0); 
 }
 pthread_create(&refetch_id, NULL, TeleConsumerRefetchThread, 0); 
 return 0; 
}

void TeleTaskConsumer::ShowState() {
  LOG(INFO) << "Total_fail_first :" << total_fail_first_ << "total_refetch_success:" << total_refetch_success_ << std::endl;
}

bool TeleTaskConsumer::SaveFailTask(TeleTask *task) {
  bool r = false;
  pthread_mutex_lock(&fail_task_mutex_);
  if (fail_task_list_.size() < 300) {
    fail_task_list_.push_back(task);
    r = true;
  }
  task->retry_count++;
  pthread_mutex_unlock(&fail_task_mutex_);
  return r;
}

TeleTask *TeleTaskConsumer::GetFailTask() {
  TeleTask *task = NULL;
  pthread_mutex_lock(&fail_task_mutex_);
  
  if (!fail_task_list_.empty()) {
    task = fail_task_list_.front();
    fail_task_list_.pop_front();
  }
  pthread_mutex_unlock(&fail_task_mutex_);
  return task;
}

void TeleTaskConsumer::CreateExtroTasks(std::string token, std::string time_str, std::queue<TeleTask*> *delay_queue) {
  
 int i = MAX_FETCH_INDEX;
 while (i < 30000) {
   TeleTask *ta = TeleTaskFactory::BuildTask(i, token, time_str, TASK_DELAY);
   if (ta && delay_queue->size() < MAX_DELAY_SIZE)
     delay_queue->push(ta);
   else
     delete ta;
   i++;
 }

}

void *TeleTaskConsumer::TeleConsumerThread(void *arg) {
  TeleTaskProducer *producer = TeleTaskProducer::GetInstance();
  TeleTaskConsumer *consumer = TeleTaskConsumer::GetInstance();
  int count = 0;
  CURL *curl = NULL;
  curl = CurlWrapper::get_instance()->CreateCurl();
  std::queue<TeleTask*>  delay_task_queue;  
  while (true) {
    TeleTask *task = NULL;
    if (delay_task_queue.empty()) {
      task = producer->GetTask();
    } else {
      task = delay_task_queue.front();
      delay_task_queue.pop();
    }
    if (task == NULL) {
      LOG(ERROR) << "fetch NUll task";
      sleep(1);

      if (count++ >= 600) {
        consumer->ShowState();
        count = 0;
      }
      continue;
    }
    LOG(INFO) << "fetch task " <<  task->index;
    std::string url = consumer->BuildUrlFromTask(task);
    std::string value = get_value_by_url(url, curl);    
    if (value == "") {
      consumer->total_fail_first_++;
      LOG(ERROR) << "fetch http error index:" << task->index << "url:" << url;
      if (consumer->SaveFailTask(task) == false) {
        LOG(ERROR) << "Save refetch task failed index" << task->index;
        delete task;
      }
      continue;
    }
    if (task->index == 0) {
      LOG(INFO) << "Fetch url: " << url << "value:" << value;
    } 
    std::string json_value = consumer->GetJsonValue(value);
    if (json_value == "") {
      delete task;
      continue;
    }

    if (json_value == "error" || json_value == "errortoken" ) {
      LOG(ERROR) << "json value " << value.c_str() << "url " <<  url;
      if (task->index == 0 && json_value == "errortoken")
        producer->SetIdentityError();
      delete task;
      continue;
    }
     
    std::string base64_result = consumer->DeBase64(json_value); 
    if (base64_result.length() == 0) {
      LOG(ERROR) << "process error value:" <<  json_value;
        delete task;
        continue;
    }


    if (consumer->Store(base64_result.c_str(), base64_result.length()) != 0) {
      LOG(ERROR) << "push data failed time:" << task->time_str.c_str() <<"value:" <<  base64_result.c_str();
    } else {
      LOG(INFO) << "task_index:" << task->index <<" push data success time: " << task->time_str.c_str() << "value:" <<  base64_result.c_str();
      if (task->type == TASK_NORMAL && task->index < MAX_FETCH_INDEX) {
        SET_BIT_INDEX(tele::GetShareMem(), task->minute_index, task->index);
        if (task->index == 9999) {
          consumer->CreateExtroTasks(task->token, task->time_str, &delay_task_queue);
        }
      }
    }
    delete task; 
  }
  

}

void *TeleTaskConsumer::TeleConsumerRefetchThread(void *arg) {
  TeleTaskConsumer *consumer = TeleTaskConsumer::GetInstance();
  CURL *cu = CurlWrapper::get_instance()->CreateCurl();
  while (true) {
    sleep(1);
    TeleTask *task = consumer->GetFailTask();
    if (task == NULL) {
      sleep(1);
      continue;
    }
    LOG(INFO) << "refetch task: " <<  task->index <<" "<< task->time_str;
    std::string url = consumer->BuildUrlFromTask(task);
    std::string value = get_value_by_url(url, cu);    
    if (value == "") {
      LOG(ERROR) << "refetch http error index:" << task->index << "url:" << url;
      if (task->retry_count < 3) {
        if (consumer->SaveFailTask(task) == false) {
          LOG(ERROR) << "Save refetch task failed index" << task->index;
          delete task;
          continue;
        }
      } else {
        LOG(ERROR) << "Task failed more times index "<< task->index << " key" << task->time_str << std::endl;
        delete task;
      }
      continue;
    }
    consumer->total_refetch_success_++;
    if (task->index == 0) {
      LOG(INFO) << "reFetch url: " << url << "value:" << value;
    }
    std::string json_value = consumer->GetJsonValue(value);
    if (json_value == "") {
      delete task;
      continue;
    }

    if (json_value == "error" || json_value == "errortoken" ) {
      LOG(ERROR) << "json value " << value.c_str() << "url " <<  url;
      delete task;
      continue;
    }
     
    std::string base64_result = consumer->DeBase64(json_value); 
    if (base64_result.length() == 0) {
      LOG(ERROR) << "refetch process error value:" <<  json_value;
        delete task;
        continue;
    }
    if (consumer->Store(base64_result.c_str(), base64_result.length()) != 0) {
      LOG(ERROR) << "refetch push data failed time:" << task->time_str.c_str() <<"value:" <<  base64_result.c_str();
    } else {
      LOG(INFO) << "refetch task_index:" << task->index <<" push data success time: " << task->time_str.c_str() << "value:" <<  base64_result.c_str();
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
