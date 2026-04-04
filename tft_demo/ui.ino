// ui.ino



void handleUI() {
  if (screenSleeping) {
    if (btnUp.fell || btnDown.fell || btnConfirm.fell || btn4.fell) {
      btnUp.fell = false;
      btnDown.fell = false;
      btnConfirm.fell = false;
      btn4.fell = false;
      // 唤醒
      screenSleeping = false;
      digitalWrite(TFT_BL, HIGH);
      tft.enableSleep(false);
      delay(120);              // ST7789 退出 sleep 需要 ~120ms
      tft.fillScreen(ST77XX_BLACK);
      TimeUpdate = 1;
      weatherUpdate = true;
      RefreshCat();
      todoDirty = true;
      eventDirty = true;
    }
    return;  // 息屏期间跳过所有 UI 更新
  }

  // === 进入息屏：四键同时 或 超时30分钟 ===
  bool allPressed = (digitalRead(BTN_UP)==LOW && digitalRead(BTN_DOWN)==LOW 
                     && digitalRead(BTN_CONFIRM)==LOW && digitalRead(BTN_4)==LOW);
  if (allPressed || (millis() - lastActivity > SLEEP_TIMEOUT)) {
    screenSleeping = true;
    tft.enableSleep(true);
    digitalWrite(TFT_BL, LOW);
    // 清掉所有按键状态
    btnUp.fell = false;
    btnDown.fell = false;
    btnConfirm.fell = false;
    btn4.fell = false;
    return;
  }

  tft.fillRect(0, 170, 320, 5, ST77XX_WHITE);
  tft.fillRect(138, 0, 5, 170, ST77XX_WHITE);
    if (btnConfirm.fell && currentScreen == SCREEN_CLOCK) {
  btnConfirm.fell = false;
  todoSelectMode = !todoSelectMode;
  if (todoSelectMode) {
    todoSelectIndex = 0;
    todoScrollOffset = 0;
  }
  todoDirty = true;
  eventDirty = true;
}
    if (todoSelectMode && currentScreen == SCREEN_CLOCK) {
    if (btnUp.fell) {
      btnUp.fell = false;
      if (todoSelectIndex > 0) todoSelectIndex--;
      todoDirty = true;
      eventDirty = true;
    }
    if (btnDown.fell) {
      btnDown.fell = false;
      if (todoSelectIndex < todoTotalVisible - 1) todoSelectIndex++;
      todoDirty = true;
      eventDirty = true;
    }
    if (btn4.fell) {
      btn4.fell = false;
      toggleSelectedItem();
      todoDirty = true;
      eventDirty = true;
    }
    // 不让这些键触发其他功能
    return;
  }
  if (btnUp.fell) {
    if (currentScreen == SCREEN_CLOCK) {
      currentScreen = SCREEN_TIMER;
      timerDirty = true;
      btnUp.fell = false;
    } else if(currentScreen == SCREEN_TIMER && timerState == TIMER_DISPLAY){
      currentScreen = SCREEN_CLOCK;
      TimeUpdate = 1;
      btnUp.fell = false;
    }

  }
  if (btn4.fell && currentScreen == SCREEN_CLOCK) {
    btn4.fell = false;
    Serial.println("btnConfirm pressed, testing PaperCut...");
    papercutReleaseAll();
  }

  if (btnDown.fell && currentScreen == SCREEN_CLOCK) {
  btnDown.fell = false;
  weatherPage = (weatherPage + 1) % 2;
  weatherUpdate = true;
}

}


void tft_display(int x, int y, int size, char data[]){
  tft.setTextSize(size);
  tft.setCursor(x,y);
  tft.println(data);
}
