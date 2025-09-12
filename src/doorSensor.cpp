#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
extern PubSubClient client;
extern String macStr;
extern String availabilityTopic;
extern String configPageUrl;

void sendLog(const String &msg);

namespace {
String doorConfigTopic =
    "homeassistant/binary_sensor/door_" + macStr + "/config";
String doorStateTopic = "door/" + macStr;
String doorOpenConfigTopic =
    "homeassistant/text/door_open_" + macStr + "/config";
String doorCloseConfigTopic =
    "homeassistant/text/door_close_" + macStr + "/config";
String doorOpenCommandTopic = "config/ha/door/open_" + macStr + "/set";
String doorOpenStateTopic = "config/ha/door/open_" + macStr + "/state";
String doorCloseCommandTopic = "config/ha/door/close_" + macStr + "/set";
String doorCloseStateTopic = "config/ha/door/close_" + macStr + "/state";
String doorOpenCode = "";
String doorCloseCode = "";

void changeDoorState(bool open) {
  const char *payload = open ? "ON" : "OFF";
  client.publish(doorStateTopic.c_str(), payload, true);
  Serial.printf("更新 门窗状态 状态: %s\n", payload);
}

} // namespace

void publishDoorDiscoveryConfig() {
  sendLog("清除旧的 Home Assistant 门窗发现配置");
  client.publish(doorConfigTopic.c_str(), "", true);
  client.publish(doorOpenConfigTopic.c_str(), "", true);
  client.publish(doorCloseConfigTopic.c_str(), "", true);

  sendLog("注册 Home Assistant 门窗传感器 发现配置");
  JsonDocument deviceInfo;
  JsonArray ids = deviceInfo["identifiers"].to<JsonArray>();
  ids.add("esp8266_" + macStr + "_door");
  deviceInfo["name"] = "门窗传感器";
  deviceInfo["model"] = "mqtt_door";
  deviceInfo["serial_number"] = macStr;
  deviceInfo["manufacturer"] = "虎哥科技";
  deviceInfo["configuration_url"] = configPageUrl;
  // 计算消息长度
  JsonDocument doorDoc;
  doorDoc["name"] = "门窗传感器";
  doorDoc["state_topic"] = doorStateTopic;
  doorDoc["device_class"] = "door";
  doorDoc["unique_id"] = macStr + "_door";
  doorDoc["payload_on"] = "ON";
  doorDoc["payload_off"] = "OFF";
  doorDoc["availability_topic"] = availabilityTopic;
  doorDoc["payload_available"] = "online";
  doorDoc["payload_not_available"] = "offline";
  doorDoc["device"] = deviceInfo;
  doorDoc["retain"] = true;
  String doorDocStr;
  serializeJson(doorDoc, doorDocStr);
  size_t msgLen = doorDocStr.length();

  // 发布自动发现配置
  if (client.beginPublish(doorConfigTopic.c_str(), msgLen, true)) {
    client.print(doorDocStr);
    if (client.endPublish()) {
      sendLog("发布门窗传感器成功");
    }
  }

  JsonDocument doorOpenTextDoc;
  doorOpenTextDoc["name"] = "① 开门码";
  doorOpenTextDoc["unique_id"] = macStr + "_door_open_text";
  doorOpenTextDoc["command_topic"] = doorOpenCommandTopic;
  doorOpenTextDoc["state_topic"] = doorOpenStateTopic;
  doorOpenTextDoc["device"] = deviceInfo;
  doorOpenTextDoc["retain"] = true;
  String doorOpenTextJsonStr;
  serializeJson(doorOpenTextDoc, doorOpenTextJsonStr);
  size_t openLen = doorOpenTextJsonStr.length();

  // 使用 Home Assistant 的自动发现主题
  if (client.beginPublish(doorOpenConfigTopic.c_str(), openLen, true)) {
    client.print(doorOpenTextJsonStr);
    if (client.endPublish()) {
      sendLog("发布开门码配置成功");
    }
  }
  JsonDocument doorCloseTextDoc;
  doorCloseTextDoc["name"] = "② 关门码";
  doorCloseTextDoc["unique_id"] = macStr + "_door_close_text";
  doorCloseTextDoc["command_topic"] = doorCloseCommandTopic;
  doorCloseTextDoc["state_topic"] = doorCloseStateTopic;
  doorCloseTextDoc["device"] = deviceInfo;
  doorCloseTextDoc["retain"] = true;
  String doorCloseTextJsonStr;
  serializeJson(doorCloseTextDoc, doorCloseTextJsonStr);
  size_t closeLen = doorCloseTextJsonStr.length();

  // 使用 Home Assistant 的自动发现主题
  if (client.beginPublish(doorCloseConfigTopic.c_str(), closeLen, true)) {
    client.print(doorCloseTextJsonStr);
    if (client.endPublish()) {
      sendLog("发布关门码配置成功");
    }
  }
}

void subscribeDoorTopics() {
  sendLog("订阅门窗控制主题");
  client.subscribe(doorOpenCommandTopic.c_str());  // 订阅门打开命令主题
  client.subscribe(doorCloseCommandTopic.c_str()); // 订阅门关闭命令主题
}

void registerDoorToggleCode(String code) {
  if (doorOpenCode == code) {
    sendLog("门已开: " + code);
    changeDoorState(true);
  } else if (doorCloseCode == code) {
    sendLog("门已关: " + code);
    changeDoorState(false);
  }
}

void handleDoorCommands(String topic, String payload) {
  if (topic != doorOpenCommandTopic &&  // 门打开命令主题
      topic != doorCloseCommandTopic && // 门关闭命令主题
      payload.length() == 0             // 值错误
  ) {
    return;
  }
  if (topic == doorOpenCommandTopic) {
    doorOpenCode = payload;
    sendLog("同步开门码: " + doorOpenCode);
    client.publish(doorOpenStateTopic.c_str(), payload.c_str(), true);
  } else if (topic == doorCloseCommandTopic) {
    doorCloseCode = payload;
    sendLog("同步关门码: " + doorCloseCode);
    client.publish(doorCloseStateTopic.c_str(), payload.c_str(), true);
  }
}