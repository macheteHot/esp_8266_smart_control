#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <TaskSchedulerDeclarations.h>
extern PubSubClient client;
extern String macStr;
extern String availabilityTopic;
extern Scheduler runner;
extern String configPageUrl;

void sendLog(const String &msg);

namespace {
constexpr int PRESENCE_PIN = 5;
bool lastPresenceState = false;
String presenceConfigTopic =
    "homeassistant/binary_sensor/presence_" + macStr + "/config";
String presenceTopic =
    "homeassistant/binary_sensor/presence_" + macStr + "/state";
void presenceSensorLoop() {
  bool currentPresence = digitalRead(PRESENCE_PIN);
  if (currentPresence != lastPresenceState) {
    lastPresenceState = currentPresence;
    const char *payload = currentPresence ? "ON" : "OFF";
    client.publish(presenceTopic.c_str(), payload, true);
    Serial.printf("更新 人体存在状态 状态: %s\n", payload);
  }
}

Task sensorTask(200, TASK_FOREVER, presenceSensorLoop);
} // namespace

void publishPresenceDiscoveryConfig() {
  sendLog("初始化设备");
  pinMode(PRESENCE_PIN, INPUT);
  runner.addTask(sensorTask);
  sensorTask.enable();
  sendLog("清除旧的 Home Assistant 人体存在传感器发现配置");
  client.publish(presenceConfigTopic.c_str(), "", true);
  sendLog("注册 Home Assistant 人体存在传感器 发现配置");
  JsonDocument deviceInfo;
  JsonArray ids = deviceInfo["identifiers"].to<JsonArray>();
  ids.add("esp8266_" + macStr + "_presence");
  deviceInfo["name"] = "人体存在传感器";
  deviceInfo["model"] = "mqtt_presence_sensor";
  deviceInfo["serial_number"] = macStr;
  deviceInfo["manufacturer"] = "虎哥科技";
  JsonDocument presenceDoc;
  presenceDoc["name"] = "存在检测";
  presenceDoc["state_topic"] = presenceTopic;
  presenceDoc["device_class"] = "presence";
  deviceInfo["configuration_url"] = configPageUrl;
  presenceDoc["unique_id"] = macStr + "_presence";
  presenceDoc["payload_on"] = "ON";
  presenceDoc["payload_off"] = "OFF";
  presenceDoc["availability_topic"] = availabilityTopic;
  presenceDoc["payload_available"] = "online";
  presenceDoc["payload_not_available"] = "offline";
  presenceDoc["device"] = deviceInfo;
  presenceDoc["retain"] = true;
  String jsonStr;
  serializeJson(presenceDoc, jsonStr);
  size_t msgLen = jsonStr.length();
  if (client.beginPublish(presenceConfigTopic.c_str(), msgLen, true)) {
    client.print(jsonStr);
    if (client.endPublish()) {
      sendLog("发布人体存在传感器成功");
    }
  }
}
