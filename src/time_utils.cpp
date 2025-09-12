#include <Arduino.h>
void sendLog(const String &msg);

namespace {

// ==== NTP服务器列表 ====
const char *ntpServers[] = {
    "ntp.aliyun.com",          "ntp1.aliyun.com",
    "ntp2.aliyun.com",         "ntp3.aliyun.com",
    "time.windows.com",        "time1.cloud.tencent.com",
    "time2.cloud.tencent.com", "time3.cloud.tencent.com",
    "time4.cloud.tencent.com", "time5.cloud.tencent.com"};

const int ntpServerCount = sizeof(ntpServers) / sizeof(ntpServers[0]);
} // namespace

// ==== 初始化并同步时间 ====
bool syncTimeFromNtpList() {
  for (int i = 0; i < ntpServerCount; i++) {
    sendLog("尝试 NTP 服务器:" + String(ntpServers[i]));
    configTime(8 * 3600, 0, ntpServers[i]); // 中国时区 +8

    int retry = 0;
    while (time(nullptr) < 100000 && retry < 10) {
      delay(100);
      Serial.print(".");
      retry++;
    }

    if (time(nullptr) > 100000) {
      sendLog("时间同步成功！");
      return true;
    }

    sendLog("当前ntp服务器不可用，尝试下一个");
  }

  sendLog("所有 NTP 服务器同步失败！");
  return false;
}

// ==== 提供时间查询函数 ====
int getCurrentYear() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  return t->tm_year + 1900;
}

int getCurrentMonth() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  return t->tm_mon + 1;
}

int getCurrentDay() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  return t->tm_mday;
}
