#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <TaskSchedulerDeclarations.h>
extern PubSubClient client;
extern String macStr;
extern Scheduler runner;
extern String availabilityTopic;
extern String configPageUrl;

void sendLog(const String &msg);
void splitCommaSeparated(const String &payload, std::vector<String> &array);

namespace {
constexpr int RELAY_PIN = 14;
String lightConfigTopic = "homeassistant/light/" + macStr + "/config";
String lightCommandTopic = "homeassistant/light/" + macStr + "/on/switch";
String lightStateTopic = "homeassistant/light/" + macStr + "/on/status";
String lightControlTextConfigTopic =
    "homeassistant/text/light_control_" + macStr + "/config";
String lightControlCommandTopic = "config/ha/light/control_" + macStr + "/set";
String lightControlStateTopic = "config/ha/light/control_" + macStr + "/state";
std::vector<String> ToggleCodesArray;
bool lastLightState = false;

Task lightSyncTask(200, TASK_ONCE, []() {
  client.publish(lightStateTopic.c_str(), lastLightState ? "true" : "false",
                 true);
  sendLog("MQTT 灯 发布状态: " + String(lastLightState ? "ON" : "OFF"));
});

void changeLightState(bool state) {
  lastLightState = state;
  sendLog("触发继电器: " + String(state ? "on" : "off"));
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  if (lightSyncTask.isEnabled()) {
    lightSyncTask.disable();
  }

  // 重新启动延迟 200ms 的任务
  lightSyncTask.restartDelayed();
}

} // namespace

void publishLightDiscoveryConfig() {
  sendLog("初始化设备");
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // 初始化继电器为低电平 (关闭)
  runner.addTask(lightSyncTask);
  lightSyncTask.enable();
  sendLog("清除旧的 Home Assistant 灯发现配置");
  client.publish(lightConfigTopic.c_str(), "", true);
  client.publish(lightControlTextConfigTopic.c_str(), "", true);
  // 通用的设备信息对象
  JsonDocument deviceInfo;
  JsonArray ids = deviceInfo["identifiers"].to<JsonArray>();
  ids.add("esp8266_" + macStr + "_light");
  deviceInfo["name"] = "灯";
  deviceInfo["model"] = "mqtt_light";
  deviceInfo["serial_number"] = macStr;
  deviceInfo["manufacturer"] = "虎哥科技";
  deviceInfo["configuration_url"] = configPageUrl;

  JsonDocument lightDoc;
  lightDoc["name"] = "灯";
  lightDoc["unique_id"] = macStr + "_light";
  lightDoc["command_topic"] = lightCommandTopic;
  lightDoc["state_topic"] = lightStateTopic;
  lightDoc["availability_topic"] = availabilityTopic;
  lightDoc["payload_available"] = "online";
  lightDoc["payload_not_available"] = "offline";
  lightDoc["payload_on"] = "true";
  lightDoc["payload_off"] = "false";
  lightDoc["qos"] = 0;
  lightDoc["retain"] = true;
  lightDoc["device"] = deviceInfo;
  sendLog("注册 Home Assistant 灯 发现配置");

  String lightJsonStr;
  serializeJson(lightDoc, lightJsonStr);
  // 发布自动发现
  if (client.beginPublish(lightConfigTopic.c_str(), lightJsonStr.length(),
                          true)) {
    client.print(lightJsonStr);
    if (client.endPublish()) {
      sendLog("发布灯成功");
    }
  }
  // 配置无线开关
  JsonDocument lightToggleTextJson;
  lightToggleTextJson["name"] = "① 开关码（多个用,分割）";
  lightToggleTextJson["unique_id"] = macStr + "_light_control_text";
  lightToggleTextJson["command_topic"] = lightControlCommandTopic;
  lightToggleTextJson["state_topic"] = lightControlStateTopic;
  lightToggleTextJson["device"] = deviceInfo;
  lightToggleTextJson["retain"] = true;
  String lightToggleTextJsonStr;
  serializeJson(lightToggleTextJson, lightToggleTextJsonStr);
  if (client.beginPublish(lightControlTextConfigTopic.c_str(),
                          lightToggleTextJsonStr.length(), true)) {
    client.print(lightToggleTextJsonStr);
    if (client.endPublish()) {
      sendLog("发布开关码配置成功");
    }
  }
}

void subscribeLightTopics() {
  sendLog("订阅灯控制主题");
  client.subscribe(lightCommandTopic.c_str());        // 订阅灯控制主题
  client.subscribe(lightControlCommandTopic.c_str()); // 订阅开关码变化
}

void handleLightCommands(String topic, String payload) {
  if (topic != lightCommandTopic &&     // 灯控制主题
      topic != lightControlCommandTopic // 开关码变化主题
  ) {
    // 忽略不相关
    return;
  }
  if (topic == lightCommandTopic) {
    sendLog("处理灯控制命令");
    changeLightState(payload == "true");
  }
  if (topic == lightControlCommandTopic) {
    sendLog("同步开关码");
    // 处理开关码
    splitCommaSeparated(payload, ToggleCodesArray);
    // 打印验证
    sendLog("更新后的 ToggleCodesArray:");
    for (const auto &codeStr : ToggleCodesArray) {
      sendLog("> " + codeStr);
    }
    // 发布到 state_topic 供 HA 显示
    client.publish(lightControlStateTopic.c_str(), payload.c_str(), true);
    sendLog("同步开关码状态");
  }
}

void registerLightToggleCode(String code) {
  // 静态变量保存上次触发信息
  static unsigned long lastTime = 0;
  constexpr unsigned long CODE_INTERVAL_MS = 200; // 中断阈值

  unsigned long now = millis();
  for (const auto &codeStr : ToggleCodesArray) {
    if (code == codeStr) {
      if (now - lastTime > CODE_INTERVAL_MS) {
        sendLog(String(now) + " 收到匹配的开关码，切换灯状态");
        changeLightState(!lastLightState);
      } else {
        sendLog(String(now) + " 连续的操作信号忽略: " + code);
      }
      lastTime = now;
      break; // 找到就退出
    }
  }
}
