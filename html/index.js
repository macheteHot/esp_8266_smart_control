import "./index.css";
import "./switch.css";

const replaceMacEls = document.querySelectorAll(".mac");
const responseEl = document.getElementById("mqttResponse");
const systemResponseEl = document.getElementById("systemResponse");
const logArea = document.getElementById("logArea");
const tabs = document.querySelectorAll(".tab");
const contents = document.querySelectorAll(".tab-content");
const mqttBtn = document.getElementById("mqttBtn");
const otaBtn = document.getElementById("otaBtn");
const sysBtn = document.getElementById("sysBtn");
const rebootBtn = document.getElementById("rebootBtn");

const serverInputEl = document.getElementById("mqtt_server");
const portInputEl = document.getElementById("mqtt_port");
const usernameInputEl = document.getElementById("mqtt_username");
const passwordInputEl = document.getElementById("mqtt_password");

const presenceInputEl = document.getElementById("presence");
const acInputEl = document.getElementById("ac");
const doorInputEl = document.getElementById("door");
const lightInputEl = document.getElementById("light");
const rcEventInputEl = document.getElementById("rcEvent");
const versionEl = document.querySelector(".version");
versionEl.textContent = "__BUILD_TIME__"; // replace by rollup

let autoScroll = true;

tabs.forEach((tab) => {
  tab.addEventListener("click", () => {
    tabs.forEach((t) => t.classList.remove("active"));
    contents.forEach((c) => c.classList.remove("active"));
    tab.classList.add("active");
    document.getElementById(tab.dataset.tab).classList.add("active");
  });
});

// MQTT 更新函数
async function updateMQTT() {
  const server = serverInputEl.value.trim();
  const port = portInputEl.value.trim();
  const username = usernameInputEl.value.trim();
  const password = passwordInputEl.value.trim();

  if (!server) {
    responseEl.textContent = "MQTT 服务器不能为空";
    return;
  }
  const portNum = Number(port);
  if (!port || isNaN(portNum) || portNum < 1 || portNum > 65535) {
    responseEl.textContent = "MQTT 端口必须在1-65535";
    return;
  }
  const formData = new FormData();
  formData.append("mqtt_server", server);
  formData.append("mqtt_port", portNum);
  formData.append("mqtt_username", username);
  formData.append("mqtt_password", password);

  try {
    const res = await fetch("/mqttUpdate", {
      method: "POST",
      body: formData,
    });
    const result = await res.json();
    responseEl.textContent = result.message || "更新成功";
  } catch (err) {
    responseEl.textContent = "更新失败: " + err;
  }
}

// OTA 更新函数
async function updateOTA() {
  const fileInput = document.getElementById("firmware");
  const responseEl = document.getElementById("otaResponse");
  if (!fileInput.files.length) {
    responseEl.textContent = "请选择固件文件";
    return;
  }
  const file = fileInput.files[0];
  if (!file.name.endsWith(".bin")) {
    responseEl.textContent = "固件文件必须为.bin";
    return;
  }
  if (file.size === 0) {
    responseEl.textContent = "固件文件不能为空";
    return;
  }

  const formData = new FormData();
  formData.append("firmware", file);

  try {
    const res = await fetch("/upgrade", {
      method: "POST",
      body: formData,
    });
    const result = await res.json();
    responseEl.textContent = result.message || "更新成功 系统将在5s后刷新";
    setTimeout(() => window.location.reload(), 5000);
  } catch (err) {
    responseEl.textContent = "更新失败: " + err;
  }
}

function getMqttConfig() {
  fetch("/getMqttConfig")
    .then((res) => res.json())
    .then((data) => {
      serverInputEl.value = data.mqtt_server;
      portInputEl.value = data.mqtt_port;
      usernameInputEl.value = data.mqtt_username;
      passwordInputEl.value = data.mqtt_password;
    })
    .catch((err) => {
      responseEl.textContent = "获取mqtt 信息失败";
    });
}

function getSystemConfig() {
  fetch("/getSystemConfig")
    .then((res) => res.json())
    .then((data) => {
      presenceInputEl.checked = data.presence;
      acInputEl.checked = data.ac;
      doorInputEl.checked = data.door;
      lightInputEl.checked = data.light;
      rcEventInputEl.checked = data.rcEvent;

      replaceMacEls.forEach((el) => {
        el.textContent = data.mac;
      });
    })
    .catch((err) => {
      systemResponseEl.textContent = "获取系统信息失败!";
    });
}

async function updateSystem() {
  const presence = presenceInputEl.checked ? "1" : "0";
  const ac = acInputEl.checked ? "1" : "0";
  const door = doorInputEl.checked ? "1" : "0";
  const light = lightInputEl.checked ? "1" : "0";
  const rcEvent = rcEventInputEl.checked ? "1" : "0";
  const formData = new FormData();
  formData.append("ac", ac);
  formData.append("door", door);
  formData.append("light", light);
  formData.append("presence", presence);
  formData.append("rcEvent", rcEvent);
  try {
    const res = await fetch("/systemUpdate", {
      method: "POST",
      body: formData,
    });
    const result = await res.json();
    systemResponseEl.textContent = result.message || "更新成功";
  } catch (err) {
    systemResponseEl.textContent = "更新失败: " + err;
  }
}

function rebootSystem() {
  fetch("/reboot");
  systemResponseEl.textContent = "系统重启中...";
  setTimeout(window.location.reload, 5000);
}

function addLogScrollEvent() {
  logArea.addEventListener("scroll", () => {
    const threshold = 5; // 容差，避免小数误差
    const atBottom =
      logArea.scrollHeight - logArea.scrollTop - logArea.clientHeight <=
      threshold;

    if (atBottom) {
      autoScroll = true; // 在底部 → 开启自动滚动
    } else {
      autoScroll = false; // 离开底部 → 停止自动滚动
    }
  });
}

function addLog(str) {
  const p = document.createElement("p");
  const now = new Date();
  const hours = String(now.getHours()).padStart(2, "0");
  const minutes = String(now.getMinutes()).padStart(2, "0");
  const seconds = String(now.getSeconds()).padStart(2, "0");
  const timeTag = `[${hours}:${minutes}:${seconds}]`;
  p.textContent = `${timeTag} ${str}`;
  logArea.appendChild(p);
  if (autoScroll) {
    logArea.scrollTop = logArea.scrollHeight;
  }
}

function showLogs() {
  const ws = new WebSocket("ws://" + window.location.hostname + ":81"); // ESP8266 WebSocket 端口
  ws.onopen = () => {
    addLog("[日志连接已建立]");
  };

  // 接收消息
  ws.onmessage = (e) => {
    addLog(e.data);
  };

  // 断开处理
  ws.onclose = () => {
    addLog("[日志连接断开]");
  };

  ws.onerror = (err) => {
    addLog("[WebSocket错误]");
  };
}
function main() {
  getMqttConfig();
  getSystemConfig();
  mqttBtn.addEventListener("click", updateMQTT);
  otaBtn.addEventListener("click", updateOTA);
  sysBtn.addEventListener("click", updateSystem);
  rebootBtn.addEventListener("click", rebootSystem);
  addLogScrollEvent();
  showLogs();
}

main();
