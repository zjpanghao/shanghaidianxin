#include "tele_task_producer.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "json_data.h"
#include "base64.h"
#define BATCHNUMS 100
#define MAX_TASK_QUEUE_SIZE 300 
#define MAX_FETCH_INDEX 10000
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
  token_str_ = get_toKen_by_url(url_fetch_token_);
  printf("The token %s\n", token_str_.c_str());
  need_update_token_ = false;
  return token_str_ == "" ? false : true;
}
bool TeleTaskProducer::Init(std::string url_fetch_token) {
  url_fetch_token_ = url_fetch_token;
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
  if (need_update_token_) {
    UpdateToken();
  }
  pthread_mutex_unlock(&task_lock_);

  printf("Update time to %s clear queue %d\n", time_str, size);
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
  if (old_tm_->tm_sec < 30 && current_tm.tm_sec >=30 ) {
    char time_str[13];
    BuildTeleTimeStr(&current_tm, time_str);
    UpdateTask(time_str); 
  }
  *old_tm_ = current_tm;
}
TeleTaskProducer::TeleTaskProducer():fetch_time_str_("") {
  pthread_mutex_init(&task_lock_, NULL);
  pthread_cond_init(&not_empty_, NULL);
  old_tm_ = new struct tm;
  memset(old_tm_, 0, sizeof(struct tm));
}

int TeleTaskProducer::PushTaskBatch() {
  int i;
  int n = 0;
  pthread_mutex_lock(&task_lock_); 
    if (fetch_time_str_.length() != 12) {
    printf("please wait for the fetch time str\n");
    pthread_mutex_unlock(&task_lock_);
    return 0;
  }
  for (i = index_; i < index_ + BATCHNUMS && i < MAX_FETCH_INDEX; i++) {
    TeleTask * task = new TeleTask;
    task->index = i;
    task->token = token_str_;
    task->time_str = fetch_time_str_;
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
