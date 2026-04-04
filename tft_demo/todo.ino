// todo.ino

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.print("MQTT JSON error: ");
    Serial.println(err.c_str());
    return;
  }

  if (strcmp(topic, mqtt_topic_todo) == 0) {
    JsonArray arr = doc.as<JsonArray>();
    todoCount = 0;
    for (JsonObject item : arr) {
      if (todoCount >= MAX_TODOS) break;
      const char* txt = item["text"] | "";
      strncpy(todoItems[todoCount].text, txt, 39);
      todoItems[todoCount].text[39] = '\0';
      todoItems[todoCount].done = item["done"] | false;
      todoCount++;
    }
    todoDirty = true;
    Serial.printf("MQTT: %d todos\n", todoCount);
  }
  else if (strcmp(topic, mqtt_topic_events) == 0) {
    JsonArray arr = doc.as<JsonArray>();
    eventCount = 0;
    for (JsonObject item : arr) {
      if (eventCount >= MAX_EVENTS) break;
      const char* t = item["t"] | "";
      const char* c = item["c"] | "";
      strncpy(eventItems[eventCount].text, t, 30);
      eventItems[eventCount].text[30] = '\0';
      strncpy(eventItems[eventCount].course, c, 20);
      eventItems[eventCount].course[20] = '\0';
      eventItems[eventCount].timestamp = item["d"] | (uint32_t)0;
      const char* s = item["s"] | "m";
      eventItems[eventCount].source = s[0];
      eventItems[eventCount].checked = (item["x"] | 0) == 1;
      eventCount++;
    }
    eventDirty = true;
    Serial.printf("MQTT: %d events\n", eventCount);
  }
}

void mqttSetup() {
  mqttWifiClient.setInsecure();
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
}

void mqttReconnect() {
  if (mqttClient.connected()) return;
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastAttempt = 0;
  if (millis() - lastAttempt < 5000) return;
  lastAttempt = millis();

  String clientId = "tomacat-esp-" + String(esp_random() % 0xFFFF, HEX);
  Serial.print("MQTT connecting... ");

  if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("connected!");
    mqttClient.subscribe(mqtt_topic_todo, 1);
    mqttClient.subscribe(mqtt_topic_events, 1);
  } else {
    Serial.printf("failed, rc=%d\n", mqttClient.state());
  }
}

// 格式化 unix timestamp 为 "Mon 3" 或 "Apr 3 11PM"
void formatDue(uint32_t ts, char* buf, int bufLen) {
  if (ts == 0) {
    strncpy(buf, "No date", bufLen);
    return;
  }
  time_t t = (time_t)ts;
  struct tm tm;
  localtime_r(&t, &tm);

  // 判断是不是今天
  struct tm now;
  getLocalTime(&now);

  if (tm.tm_yday == now.tm_yday && tm.tm_year == now.tm_year) {
    snprintf(buf, bufLen, "Today %d%s", 
      tm.tm_hour > 12 ? tm.tm_hour - 12 : (tm.tm_hour == 0 ? 12 : tm.tm_hour),
      tm.tm_hour >= 12 ? "PM" : "AM");
  } else {
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                            "Jul","Aug","Sep","Oct","Nov","Dec"};
    snprintf(buf, bufLen, "%s %d %d%s",
      months[tm.tm_mon], tm.tm_mday,
      tm.tm_hour > 12 ? tm.tm_hour - 12 : (tm.tm_hour == 0 ? 12 : tm.tm_hour),
      tm.tm_hour >= 12 ? "PM" : "AM");
  }
}
void toggleSelectedItem() {
  // 遍历和 drawTodo 一样的顺序，找到第 todoSelectIndex 个
  int idx = 0;

  // 未完成 todo
  for (int i = 0; i < todoCount; i++) {
    if (todoItems[i].done) continue;
    if (idx == todoSelectIndex) {
      todoItems[i].done = !todoItems[i].done;
      return;
    }
    idx++;
  }

  // 未完成 event
  for (int i = 0; i < eventCount; i++) {
    if (eventItems[i].checked) continue;
    if (idx == todoSelectIndex) {
      eventItems[i].checked = !eventItems[i].checked;
      return;
    }
    idx++;
  }

  // 已完成 todo
  for (int i = 0; i < todoCount; i++) {
    if (!todoItems[i].done) continue;
    if (idx == todoSelectIndex) {
      todoItems[i].done = !todoItems[i].done;
      return;
    }
    idx++;
  }

  // 已完成 event
  for (int i = 0; i < eventCount; i++) {
    if (!eventItems[i].checked) continue;
    if (idx == todoSelectIndex) {
      eventItems[i].checked = !eventItems[i].checked;
      return;
    }
    idx++;
  }
}
void drawTodo() {
  if (!todoDirty && !eventDirty) return;
  todoDirty = false;
  eventDirty = false;

  int x0 = 147;
  int y0 = 4;
  int lineH = 11;
  int y = y0;
  int drawIdx = 0;

  tft.fillRect(143, 0, 177, 170, ST77XX_BLACK);

  if (todoSelectMode) {
    tft.drawRect(143, 0, 177, 170, tft.color565(255, 215, 0));
    tft.drawRect(144, 1, 175, 168, tft.color565(255, 215, 0));
  }

  tft.setTextSize(1);
  uint16_t yellow = tft.color565(255, 215, 0);

  // === Todos (未完成) ===
  for (int i = 0; i < todoCount && y + lineH < 166; i++) {
    if (todoItems[i].done) continue;
    bool sel = todoSelectMode && drawIdx == todoSelectIndex;
    tft.setTextColor(sel ? yellow : ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(x0, y);
    tft.print("- ");
    char buf[25];
    strncpy(buf, todoItems[i].text, 24);
    buf[24] = '\0';
    tft.print(buf);
    y += lineH;
    drawIdx++;
  }

  // === Events (未勾选) ===
  for (int i = 0; i < eventCount && y + lineH * 2 < 166; i++) {
    if (eventItems[i].checked) continue;
    bool sel = todoSelectMode && drawIdx == todoSelectIndex;

    uint16_t color = sel ? yellow
      : (eventItems[i].source == 'g')
        ? tft.color565(30, 144, 255)
        : tft.color565(255, 107, 107);

    tft.setTextColor(color, ST77XX_BLACK);
    tft.setCursor(x0, y);
    char nameBuf[25];
    strncpy(nameBuf, eventItems[i].text, 24);
    nameBuf[24] = '\0';
    tft.print(nameBuf);
    y += lineH;

    if (y + lineH < 166) {
      tft.setTextColor(sel ? yellow : tft.color565(100, 100, 120), ST77XX_BLACK);
      tft.setCursor(x0 + 6, y);
      char dueBuf[20];
      formatDue(eventItems[i].timestamp, dueBuf, sizeof(dueBuf));
      if (eventItems[i].course[0]) {
        char lineBuf[40];
        snprintf(lineBuf, sizeof(lineBuf), "%.10s %s", eventItems[i].course, dueBuf);
        tft.print(lineBuf);
      } else {
        tft.print(dueBuf);
      }
      y += lineH;
    }
    drawIdx++;
  }

  // === 分隔线 ===
  bool hasChecked = false;
  for (int i = 0; i < todoCount; i++) if (todoItems[i].done) hasChecked = true;
  for (int i = 0; i < eventCount; i++) if (eventItems[i].checked) hasChecked = true;

  if (hasChecked && y + lineH < 166) {
    tft.drawFastHLine(x0, y + 2, 168, tft.color565(60, 60, 80));
    y += 6;
  }

  // === Todos (已完成) ===
  uint16_t grayColor = tft.color565(70, 70, 90);
  for (int i = 0; i < todoCount && y + lineH < 166; i++) {
    if (!todoItems[i].done) continue;
    bool sel = todoSelectMode && drawIdx == todoSelectIndex;
    tft.setTextColor(sel ? yellow : grayColor, ST77XX_BLACK);
    tft.setCursor(x0, y);
    tft.print("x ");
    char buf[25];
    strncpy(buf, todoItems[i].text, 24);
    buf[24] = '\0';
    tft.print(buf);
    y += lineH;
    drawIdx++;
  }

  // === Events (已勾选) ===
  for (int i = 0; i < eventCount && y + lineH < 166; i++) {
    if (!eventItems[i].checked) continue;
    bool sel = todoSelectMode && drawIdx == todoSelectIndex;
    tft.setTextColor(sel ? yellow : grayColor, ST77XX_BLACK);
    tft.setCursor(x0, y);
    char buf[25];
    strncpy(buf, eventItems[i].text, 24);
    buf[24] = '\0';
    tft.print(buf);
    y += lineH;
    drawIdx++;
  }

  // 更新总可见数
  todoTotalVisible = drawIdx;

  // 空状态
  if (todoCount == 0 && eventCount == 0) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setCursor(x0 + 20, y0 + 70);
    tft.print("No todos");
  }
}