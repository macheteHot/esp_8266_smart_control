#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TaskSchedulerDeclarations.h>

extern Scheduler runner;
extern String macStr;
extern bool enablePresence, enableDoorSensor, enableLight, enableAc;
extern char mqtt_server[128];
extern char mqtt_port[6];
extern char mqtt_user[64];
extern char mqtt_password[64];
void sendLog(const String &msg);

String availabilityTopic = "huge/device/" + macStr + "/availability";

WiFiClient espClient;
PubSubClient client(espClient);
// 空调
void publishClimateDiscoveryConfig();
void subscribeClimateTopics();
void handleClimateCommands(String topic, String payload);
// 门窗传感器
void publishDoorDiscoveryConfig();
void subscribeDoorTopics();
void handleDoorCommands(String topic, String payload);
// 灯
void publishLightDiscoveryConfig();
void subscribeLightTopics();
void handleLightCommands(String topic, String payload);
// 人体存在传感器
void publishPresenceDiscoveryConfig();

namespace {
String clientId = "ESP8266Client-" + macStr;
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char msgBuf[length + 1];
  memcpy(msgBuf, payload, length);
  msgBuf[length] = '\0';

  String msg = String(msgBuf);
  const String topicStr = String(topic);
  sendLog("收到消息: " + topicStr);
  sendLog("消息内容: " + msg);
  handleClimateCommands(topicStr, msg);
  handleLightCommands(topicStr, msg);
  handleDoorCommands(topicStr, msg);
}

static void connectMqtt() {
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_password,
                     availabilityTopic.c_str(), 1, true, "offline", true)) {
    sendLog("连接mqtt 成功");
    if (enableDoorSensor) {
      publishDoorDiscoveryConfig();
      subscribeDoorTopics();
    }
    if (enablePresence) {
      publishPresenceDiscoveryConfig();
    }
    if (enableAc) {
      publishClimateDiscoveryConfig();
      subscribeClimateTopics();
    }
    if (enableLight) {
      publishLightDiscoveryConfig();
      subscribeLightTopics();
    }
    client.publish(availabilityTopic.c_str(), "online",
                   true); // 发布上线
  } else {
    sendLog("连接mqtt失败 错误代码: " + String(client.state()));
  }
}

static void mqttLoop() {
  static unsigned long lastReconnectAttempt = 0;
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      connectMqtt();
    }
  }
  client.loop();
}
Task mqttTask(0, TASK_FOREVER, &mqttLoop);
} // namespace

void initMqtt() {
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(mqttCallback);
  connectMqtt();
  // 注册循环
  runner.addTask(mqttTask);
  mqttTask.enable();
}
