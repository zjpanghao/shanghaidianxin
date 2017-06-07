#include "tele_task_producer.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "glog/logging.h"
#include "CurlWrapper.h"
#include "json_data.h"
#include "base64.h"
#include "tele_mat.h"
#include "common.h"
#define BATCHNUMS 200
#define MAX_TASK_QUEUE_SIZE 600 
TeleTaskProducer* TeleTaskProducer::instance_ = NULL;
TeleTaskProducer* TeleTaskProducer::GetInstance() {
  if (instance_ == NULL) {
    instance_ = new TeleTaskProducer();
  }
  return instance_;
}

void TeleTaskProducer::SetIdentityError() {
  pthread_mutex_lock(&task_lock_);
  need_update_token_ = true;
  pthread_mutex_unlock(&task_lock_);
}

bool TeleTaskProducer::UpdateToken() {
  std::string token_str;
  token_str = get_toKen_by_url(url_fetch_token_, cu_);
  if (token_str.length() == 0)
    return false;
  token_str_ = token_str;
  LOG(INFO) << "The token " << token_str_;
  need_update_token_ = false;
  token_update_minutes_ = 0;
  return token_str_ == "" ? false : true;
}
bool TeleTaskProducer::Init(std::string url_fetch_token, int delay_min) {
  url_fetch_token_ = url_fetch_token;
  delay_min_ = delay_min;
  cu_ = CurlWrapper::get_instance()->CreateCurl();
  UpdateToken(); 
  pthread_t producerid;
  pthread_create(&producerid, NULL, TeleTaskProducer::TeleProducerThread, NULL);
  return true;
}

int TeleTaskProducer::GetBufferedTaskSize() {
  int size = 0;
  pthread_mutex_lock(&task_lock_);
  size = producer_queue_.size();
  pthread_mutex_unlock(&task_lock_);
  return size;
}
void *TeleTaskProducer::TeleProducerThread(void *arg) {
  TeleTaskProducer *producer = GetInstance();
  while (true) {
   if ( producer->PushTaskBatch() == 0) {
      sleep(1); //the contition may not yet be ok
   }
   producer->CheckandUpdateTask(); 
   sleep(1);
  }
  return NULL;
}

void TeleTaskProducer::CheckTokenTimeout() {
  token_update_minutes_++;
  if (token_update_minutes_ >= 60) {
    need_update_token_ = true;
    token_update_minutes_ = 0;
  }
}

void TeleTaskProducer::UpdateTask(const char *time_str) {
  pthread_mutex_lock(&task_lock_);
  fetch_time_str_ = time_str;
  index_ = 0; 
  int size = producer_queue_.size();
  while (!producer_queue_.empty()) {
    TeleTask* task = producer_queue_.front();
    producer_queue_.pop();
    delete task;
  }
  
  CheckTokenTimeout();  

  if (need_update_token_) {
    UpdateToken();
  }
  pthread_mutex_unlock(&task_lock_);

  LOG(INFO) << "Update time to " << time_str << " clear queue " <<  size;
}

void TeleTaskProducer::BuildTeleTimeStr(struct tm *current_time, char *buffer) {
  
  sprintf(buffer, "%d%02d%02d%02d%02d",
    current_time->tm_year + 1900,
    current_time->tm_mon + 1,
    current_time->tm_mday,
    current_time->tm_hour,
    current_time->tm_min);
}
void TeleTaskProducer::CheckandUpdateTask() {
  time_t seconds = time(NULL);
  struct tm current_tm;
  localtime_r(&seconds, &current_tm);
  const int delay_min = delay_min_; 
  if (old_tm_->tm_min != current_tm.tm_min ) {
    char time_str[13];
    time_t fetch_seconds = seconds - delay_min * 60;
    struct tm fetch_tm;
    localtime_r(&fetch_seconds, &fetch_tm);
    BuildTeleTimeStr(&fetch_tm, time_str);
    UpdateTask(time_str); 
    *old_tm_ = current_tm;
  }
}
TeleTaskProducer::TeleTaskProducer():fetch_time_str_("") {
  pthread_mutex_init(&task_lock_, NULL);
  pthread_cond_init(&not_empty_, NULL);
  old_tm_ = new struct tm;
  time_t seconds = time(NULL);
  memset(old_tm_, 0, sizeof(struct tm));
}

int TeleTaskProducer::PushTaskBatch() {
  int i;
  int n = 0;
  pthread_mutex_lock(&task_lock_); 
  if (fetch_time_str_.length() != 12) {
    LOG(INFO) << "please wait for the fetch time str";
    pthread_mutex_unlock(&task_lock_);
    return 0;
  }
  char buf[4];
  strcpy(buf, fetch_time_str_.c_str() + 10);
  int minute = atoi(buf);
  memset(buf, 0, sizeof(buf));
  strncpy(buf, fetch_time_str_.c_str() + 8, 2);
  int hour = atoi(buf);
   
  for (i = index_; i < index_ + BATCHNUMS && i < MAX_FETCH_INDEX; i++) {
    int minute_index = hour * 60 + minute;
    if (!IsSlave()) {
      CLEAR_SET(tele::GetShareMem(), minute_index, i);
    } else if (IS_SET(tele::GetShareMem(), minute_index, i)) {
      n++;
      continue;
    }
    TeleTask * task = TeleTaskFactory::BuildTask(i, token_str_, fetch_time_str_, TASK_NORMAL);
    if (!task) {
      n++;
      continue;
    }
      
    task->minute_index = minute_index;
    if (PushTask(task) < 0) {
      delete task;
      break;
    } else {
      n++;
    }
  }
  index_ += n;
  pthread_mutex_unlock(&task_lock_);
  if (n > 0) {
    pthread_cond_broadcast(&not_empty_);
  }
  return n;
}

int TeleTaskProducer::PushTask(TeleTask* task) {
  int rc = -1;
  if ( producer_queue_.size() < MAX_TASK_QUEUE_SIZE) {
    producer_queue_.push(task);
    rc = 0;
  }
 // printf("push task %d , %s rc %d\n", task->index, task->time_str.c_str(), rc);
  return rc;
}

TeleTask* TeleTaskProducer::GetTask() {
  TeleTask *task = NULL;
  pthread_mutex_lock(&task_lock_);
  while (producer_queue_.size() == 0) {
    pthread_cond_wait(&not_empty_, &task_lock_);
  }
  if (producer_queue_.size() > 0) {
    task = producer_queue_.front();
    producer_queue_.pop();
  }
  pthread_mutex_unlock(&task_lock_);
  return task;
}
