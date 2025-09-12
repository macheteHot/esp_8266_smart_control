#include <ESP8266WiFi.h>
#include <LittleFS.h>
void sendLog(const String &msg);

constexpr int LED_PIN = 2;

extern char mqtt_server[128];
extern char mqtt_port[6];
extern char mqtt_user[64];
extern char mqtt_password[64];

bool enablePresence, enableDoorSensor, enableLight, enableAc, enableRcEvent;
const String MQTT_CONFIG_FILE_NAME = "/mqtt_config";
const String COMPONENT_CONFIG_FILE_NAME = "/component_config";

String getMacStr() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac;
}

String macStr = getMacStr();

String getLocalUrl() {
  IPAddress ip = WiFi.localIP();
  return "http://" + ip.toString();
};

void initLED() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    if (i < times - 1) {
      delay(100);
    }
  }
}

void saveComponentsConfig(bool presence, bool door, bool light, bool ac,
                          bool rc) {
  enablePresence = presence;
  enableDoorSensor = door;
  enableLight = light;
  enableAc = ac;
  enableRcEvent = rc;
  File f = LittleFS.open(COMPONENT_CONFIG_FILE_NAME, "w");
  if (f) {
    uint8_t configByte = 0;
    configByte |= (rc ? 0x10 : 0);
    configByte |= (presence ? 0x08 : 0);
    configByte |= (door ? 0x04 : 0);
    configByte |= (light ? 0x02 : 0);
    configByte |= (ac ? 0x01 : 0);
    f.write(configByte);
    f.close();
    sendLog("使能配置存储成功");
  } else {
    sendLog("无法写入存储!");
  }
}

void loadComponentsConfig() {
  File f = LittleFS.open(COMPONENT_CONFIG_FILE_NAME, "r");
  if (f) {
    if (f.available()) {
      uint8_t configByte = f.read();
      enableRcEvent = (configByte & 0x10) != 0;
      enablePresence = (configByte & 0x08) != 0;
      enableDoorSensor = (configByte & 0x04) != 0;
      enableLight = (configByte & 0x02) != 0;
      enableAc = (configByte & 0x01) != 0;
    } else {
      enableRcEvent = false;
      enablePresence = false;
      enableDoorSensor = false;
      enableLight = true;
      enableAc = false;
    }
    sendLog("获取使能配置成功");
    sendLog("433事件: " + String(enableRcEvent ? "开启" : "关闭"));
    sendLog("空调: " + String(enableAc ? "开启" : "关闭"));
    sendLog("人体存在传感器: " + String(enablePresence ? "开启" : "关闭"));
    sendLog("门磁传感器: " + String(enableDoorSensor ? "开启" : "关闭"));
    sendLog("灯: " + String(enableLight ? "开启" : "关闭"));
    f.close();
    sendLog("使能配置加载成功");
  } else {
    sendLog("无法读取存储!");
  }
}

void saveMqttConfig(char *server, char *port, char *user, char *password) {
  File f = LittleFS.open(MQTT_CONFIG_FILE_NAME, "w");
  if (f) {
    f.printf("%s\n%s\n%s\n%s\n", server, port, user, password);
    f.close();
    sendLog("MQTT 配置存储成功:");
    sendLog("Server: " + String(server) + " port: " + String(port) +
            " user: " + String(user) + " password: " + String(password));
  } else {
    sendLog("无法写入存储!");
  }
}

void loadMqttConfig(char *server, char *port, char *user, char *password) {
  File f = LittleFS.open(MQTT_CONFIG_FILE_NAME, "r");
  if (f) {
    String line;
    line = f.readStringUntil('\n');
    line.trim();
    line.toCharArray(server, 128);
    line = f.readStringUntil('\n');
    line.trim();
    line.toCharArray(port, 6);
    line = f.readStringUntil('\n');
    line.trim();
    line.toCharArray(user, 64);
    line = f.readStringUntil('\n');
    line.trim();
    line.toCharArray(password, 64);
    f.close();
  } else {
    sendLog("没有发现mqtt 配置!");
  }
}

void loadAllConfig() {
  LittleFS.begin();
  loadComponentsConfig();
  loadMqttConfig(mqtt_server, mqtt_port, mqtt_user, mqtt_password);
}

void splitCommaSeparated(const String &payload, std::vector<String> &array) {
  array.clear();
  size_t start = 0;
  int end;
  while ((end = payload.indexOf(',', start)) != -1) {
    array.push_back(payload.substring(start, end));
    start = end + 1;
  }
  if (start < payload.length()) {
    array.push_back(payload.substring(start));
  }
}
