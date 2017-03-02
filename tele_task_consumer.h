#ifndef TELE_TASK_CONSUMER_H
#define TELE_TASK_CONSUMER_H
#include <pthread.h>
#include <string>
#include <list>
#include "curl/curl.h"
struct TeleTask;
namespace tele {
struct TeleMem;
}
class TeleTaskConsumer {
 public:
  TeleTaskConsumer();
  int Init(int nthread);
  static void *TeleConsumerThread(void *arg);
  static void *TeleConsumerRefetchThread(void *arg);
  std::string BuildUrlFromTask(TeleTask *task);
  static TeleTaskConsumer* GetInstance();
  void ShowState();
  int total_refetch_success_;
  int total_fail_first_;
 private:
  static TeleTaskConsumer* instance_;
  std::string url_pre_;
  std::string url_table_;
  std::string url_flag_;
  std::string  DeBase64(std::string value);
  int Store(const char *value, int len);
  std::string GetJsonValue(std::string value);
  TeleTask *GetFailTask();
  bool SaveFailTask(TeleTask *task);
  std::list<TeleTask*> fail_task_list_;
  pthread_mutex_t fail_task_mutex_;

};
#endif
