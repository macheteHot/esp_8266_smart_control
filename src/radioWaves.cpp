#include <PubSubClient.h>
#include <RCSwitch.h> // 433 mhz 接收器
#include <TaskSchedulerDeclarations.h>

extern Scheduler runner;
extern PubSubClient client;
extern String macStr;
extern bool enableRcEvent;

void registerLightToggleCode(String code);
void registerClimateCode(String code);
void registerDoorToggleCode(String code);
void sendLog(const String &msg);

namespace {
constexpr int RC_PIN = 12;
RCSwitch mySwitch = RCSwitch();

void radioWavesLoop() {
  unsigned int bitLength = mySwitch.getReceivedBitlength();
  unsigned int protocol = mySwitch.getReceivedProtocol();
  unsigned long value = mySwitch.getReceivedValue();
  if (value != 0) {
    String valueStr = String(value);
    sendLog(String(millis()) + " 收到遥控信号 bitLen: " + String(bitLength) +
            " protocol: " + String(protocol) + " value: " + valueStr);
    sendLog("======================");
    // 发布事件
    if (enableRcEvent) {
      String topic = "rc_433/" + macStr + "/events";
      client.publish(topic.c_str(), valueStr.c_str(), false);
      sendLog("发布 433 事件: " + valueStr);
    }
    // 回调所有监听
    registerLightToggleCode(valueStr);
    registerDoorToggleCode(valueStr);
    registerClimateCode(valueStr);
  }
  mySwitch.resetAvailable();
}

Task radioWavesTask(0, TASK_FOREVER, &radioWavesLoop);
} // namespace

void initRadioReceiver() {
  mySwitch.enableReceive(RC_PIN);
  // 注册循环
  runner.addTask(radioWavesTask);
  radioWavesTask.enable();
}
