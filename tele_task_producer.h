#ifndef TELE_TASK_PRODUCER_H
#define TELE_TASK_PRODUCER_H
#include <pthread.h>
#include <queue>
#include <string>
struct tm;
struct TeleTask {
  int index;
  std::string token;
  std::string time_str;
};

class TeleTaskProducer {
 public:
  TeleTaskProducer();
  bool Init(std::string url_fetch_token);
  static void *TeleProducerThread(void *arg);
  TeleTask * GetTask();
  int  PushTaskBatch();
  void CheckandUpdateTask();
  void UpdateTask(const char *time_str);
  static TeleTaskProducer *GetInstance();
  int GetBufferedTaskSize();
  void SetIdentityError(); 
 private:
  int index_;
  std::string token_str_;
  std::string  fetch_time_str_;
  std::string url_fetch_token_;
  bool need_update_token_; 
  static TeleTaskProducer *instance_;
  struct tm *old_tm_;
  bool UpdateToken(); 
  int  PushTask(TeleTask* task);
  void BuildTeleTimeStr(struct tm *current_time, char *buffer); 
  std::queue<TeleTask*> producer_queue_; 
  pthread_mutex_t task_lock_;
  pthread_cond_t  not_empty_;
  bool NeedUpdate();
};

#endif
