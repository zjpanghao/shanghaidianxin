#ifndef TELE_TASK_PRODUCER_H
#define TELE_TASK_PRODUCER_H
#include <pthread.h>
#include <queue>
#include <string>
#include <curl/curl.h>
#define MAX_FETCH_INDEX 10000
enum TaskType {
TASK_NORMAL = 0,
TASK_DELAY = 1

};
struct tm;
struct TeleTask {
  int index;
  int minute_index;
  std::string token;
  std::string time_str;
  int retry_count;
  TaskType type;
};

class TeleTaskFactory {
 public:
  static TeleTask* BuildTask(int index, std::string token, std::string time_str, TaskType type) {
    TeleTask *ta = new TeleTask;
    ta->index = index;
    ta->time_str = time_str;
    ta->token = token;
    ta->type = type;
    ta->minute_index = 0;
    ta->retry_count = 0;
    return ta;
  }
};
class TeleTaskProducer {
 public:
  TeleTaskProducer();
  bool Init(std::string url_fetch_token, int delay_min);
  static void *TeleProducerThread(void *arg);
  TeleTask * GetTask();
  int PushTaskBatch();
  void CheckandUpdateTask();
  void UpdateTask(const char *time_str);
  static TeleTaskProducer *GetInstance();
  int GetBufferedTaskSize();
  void SetIdentityError();
 private:
  int index_;
  int delay_min_;
  std::string token_str_;
  std::string fetch_time_str_;
  std::string url_fetch_token_;
  bool need_update_token_;
  int token_update_minutes_;
  static TeleTaskProducer *instance_;
  struct tm *old_tm_;
  bool UpdateToken();
  int PushTask(TeleTask* task);
  void BuildTeleTimeStr(struct tm *current_time, char *buffer);
  void CheckTokenTimeout();
  std::queue<TeleTask*> producer_queue_;
  std::queue<TeleTask*> producer_delay_queue_;
  pthread_mutex_t task_lock_;
  pthread_cond_t not_empty_;
  bool NeedUpdate();
  CURL *cu_;
};

#endif
