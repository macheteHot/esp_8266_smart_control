#include <WiFiManager.h>

void saveMqttConfig(char *server, char *port, char *user, char *password);
void loadMqttConfig(char *server, char *port, char *user, char *password);
extern String macStr;

char mqtt_server[128];
char mqtt_port[6] = "1883";
char mqtt_user[64];
char mqtt_password[64];
bool shouldSaveConfig = false;

namespace {
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "MQTT服务器地址", mqtt_server,
                                        128);
WiFiManagerParameter custom_mqtt_port("port", "MQTT 服务器端口", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("user", "MQTT 用户名(可为空)", mqtt_user,
                                      64);
WiFiManagerParameter custom_mqtt_password("password", "MQTT 密码(可为空)",
                                          mqtt_password, 64);
void saveConfigCallback() {
  shouldSaveConfig = true; // 用户修改了配置
}

} // namespace

void autoConnectWifi() {
  wm.setConnectTimeout(15);
  wm.setTitle("智能配网");
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setHostname("smart_control_" + macStr);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_password);
  if (!wm.autoConnect("虎哥科技-智能控制器")) {
    Serial.println("无法连接或超时");
    ESP.restart();
    delay(1000);
  }
  if (shouldSaveConfig) {
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    saveMqttConfig(mqtt_server, mqtt_port, mqtt_user, mqtt_password);
  }
}