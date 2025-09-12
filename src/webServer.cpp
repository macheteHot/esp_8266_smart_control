#include "index_html.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <TaskSchedulerDeclarations.h>
#include <WebSocketsServer.h>
#include <deque>

extern Scheduler runner;
extern char mqtt_server[128];
extern char mqtt_port[6];
extern char mqtt_user[64];
extern char mqtt_password[64];
extern String macStr;
extern bool enablePresence, enableDoorSensor, enableLight, enableAc,
    enableRcEvent;
void saveMqttConfig(char *server, char *port, char *user, char *password);
void saveComponentsConfig(bool presence, bool door, bool light, bool ac,
                          bool rc);

namespace {
// ----------------- WebSocket 日志 -----------------
constexpr int MAX_LOG_BUFFER = 200;
constexpr int MAX_LOG_BYTES = 4096; // 最大日志长度
std::deque<String> logBuffer;
size_t logBytes = 0;            // 当前缓存总字节数
WebSocketsServer webSocket(81); // WebSocket 服务端口
} // namespace

// 发送日志给所有 WebSocket 客户端
void sendLog(const String &msg) {
  Serial.println(msg);
  String tmp = msg; // 非 const
  logBuffer.push_back(tmp);
  logBytes += tmp.length();
  while (logBytes > MAX_LOG_BYTES && !logBuffer.empty()) {
    logBytes -= logBuffer.front().length();
    logBuffer.pop_front();
  }
  while (logBuffer.size() > MAX_LOG_BUFFER) {
    logBuffer.pop_front();
  }

  for (uint8_t i = 0; i < webSocket.connectedClients(); i++) {
    webSocket.sendTXT(i, tmp);
  }
}

namespace {

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// WebSocket 事件处理（可选）
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                      size_t length) {
  if (type == WStype_CONNECTED) {
    sendLog("WebSocket客户端已连接 " + String(num + 1));
    for (auto &msg : logBuffer) {
      webSocket.sendTXT(num, msg);
    }
  }
}

// ----------------- Web 处理 -----------------
void handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Content-Type", "text/html; charset=utf-8");
  server.send_P(200, "text/html", (PGM_P)index_html, index_html_len);
}

void handleGetMqttConfig() {
  JsonDocument mqttConfigDoc;
  mqttConfigDoc["mqtt_server"] = mqtt_server;
  mqttConfigDoc["mqtt_port"] = mqtt_port;
  mqttConfigDoc["mqtt_username"] = mqtt_user;
  mqttConfigDoc["mqtt_password"] = mqtt_password;
  String json;
  serializeJson(mqttConfigDoc, json);
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", json);
}

void handleGetSystemConfig() {
  JsonDocument configDoc;
  configDoc["presence"] = enablePresence;
  configDoc["ac"] = enableAc;
  configDoc["door"] = enableDoorSensor;
  configDoc["light"] = enableLight;
  configDoc["rcEvent"] = enableRcEvent;
  configDoc["mac"] = macStr;
  String json;
  serializeJson(configDoc, json);
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", json);
}

void handleUpgrade() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    sendLog("开始更新: " + String(upload.filename));

    size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) {
      Update.printError(Serial);
      server.send(500, "application/json", "{\"message\":\"开始写入失败\"}");
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
      server.send(500, "application/json", "{\"message\":\"写入失败\"}");
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      sendLog("更新成功: " + String(upload.totalSize) + " 字节, 重启");
      server.send(200, "application/json", "{\"message\":\"上传成功\"}");
      delay(500);
      ESP.restart();
    } else {
      Update.printError(Serial);
      server.send(500, "application/json", "{\"message\":\"更新失败\"}");
    }
  }
}

void handleSetMqttUpdate() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"message\":\"仅支持 POST\"}");
    return;
  }

  // 获取表单字段
  String mqtt_server = server.arg("mqtt_server");
  String mqtt_port_str = server.arg("mqtt_port");
  String mqtt_username = server.arg("mqtt_username");
  String mqtt_password = server.arg("mqtt_password");

  if (mqtt_server.length() == 0) {
    server.send(400, "application/json",
                "{\"message\":\"mqtt_server 不能为空\"}");
    return;
  }
  if (mqtt_port_str.length() == 0) {
    server.send(400, "application/json",
                "{\"message\":\"mqtt_port 不能为空\"}");
    return;
  }

  saveMqttConfig((char *)mqtt_server.c_str(), (char *)mqtt_port_str.c_str(),
                 (char *)mqtt_username.c_str(), (char *)mqtt_password.c_str());

  server.send(200, "application/json", "{\"message\":\"MQTT 配置更新成功\"}");
}

void handleSetSystemConfig() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"message\":\"仅支持 POST\"}");
    return;
  }
  // 获取表单字段
  bool ac = server.arg("ac") == "1";
  bool presence = server.arg("presence") == "1";
  bool door = server.arg("door") == "1";
  bool light = server.arg("light") == "1";
  bool rc = server.arg("rcEvent") == "1";
  saveComponentsConfig(presence, door, light, ac, rc);
  server.send(200, "application/json",
              "{\"message\":\"配置使能成功,请手动重启以生效,"
              "请在home assistant中请手动删除关闭的设备\"}");
}
void webLoop() {
  server.handleClient();
  webSocket.loop();
}

Task webControlTask(0, TASK_FOREVER, &webLoop);

} // namespace

void initWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upgrade", HTTP_POST, []() {}, handleUpgrade);
  server.on("/reboot", HTTP_GET, []() { ESP.restart(); });
  server.on("/systemUpdate", HTTP_POST, handleSetSystemConfig);
  server.on("/getSystemConfig", HTTP_GET, handleGetSystemConfig);
  server.on("/mqttUpdate", HTTP_POST, handleSetMqttUpdate);
  server.on("/getMqttConfig", HTTP_GET, handleGetMqttConfig);
  // WebSocket 初始化
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  server.begin();
  runner.addTask(webControlTask);
  webControlTask.enable();
}
