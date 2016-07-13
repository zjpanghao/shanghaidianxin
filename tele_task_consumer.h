#ifndef TELE_TASK_CONSUMER_H
#define TELE_TASK_CONSUMER_H
#include <string>
struct TeleTask;
class TeleTaskConsumer {
 public:
  TeleTaskConsumer();
  int Init(int nthread);
  static void *TeleConsumerThread(void *arg);
  std::string BuildUrlFromTask(TeleTask *task);
  static TeleTaskConsumer* GetInstance();
 private:
  static TeleTaskConsumer* instance_;
  std::string url_pre_;
  std::string url_table_;
  std::string url_flag_;
  std::string  DeBase64(std::string value);
  int Store(const char *value, int len);
  std::string GetJsonValue(std::string value);

};
#endif
