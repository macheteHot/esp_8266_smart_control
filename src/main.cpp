#include <Arduino.h>
#include <TaskScheduler.h>

Scheduler runner; // 主调度器

void sendLog(const String &msg);
void loadAllConfig();
void syncTimeFromNtpList();
void autoConnectWifi();
void initLED();
void initMqtt();
void initRadioReceiver();
void blinkLED(int times);
void initWebServer();
String getLocalUrl();

String configPageUrl = "";

uint32_t initialHeap = 0;

void sendFreeHeap() {
  uint32_t freeHeap = ESP.getFreeHeap();
  float usedPercent = (float)(initialHeap - freeHeap) * 100.0 / initialHeap;
  sendLog("Free Heap: " + String(freeHeap) +
          " bytes, Used: " + String(usedPercent, 1) + "%");
}

Task logFreeHeapTask(5000, TASK_FOREVER, &sendFreeHeap);

void setup() {
  Serial.begin(115200);
  initialHeap = ESP.getFreeHeap(); // 记录最大内存
  loadAllConfig();                 // 获取所有配置 最早初始化
  initLED();
  autoConnectWifi();             // 最早先连接wifi
  configPageUrl = getLocalUrl(); // 获取本机地址
  blinkLED(3);                   // wifi 连接成功 闪烁 LED
  syncTimeFromNtpList();         // 时间同步
  initRadioReceiver();           // 初始化433 无线电
  initMqtt();                    // 初始化mqtt通信
  initWebServer();               // 开启web 服务
  runner.addTask(logFreeHeapTask);
  logFreeHeapTask.enable();
}

// ==== 主循环 ====
void loop() { runner.execute(); }
