void UpdateTime(){

  if(TimeUpdate==1){
    struct tm now;
    if (!getLocalTime(&now)) return;  // 拿不到就跳过

    tft.fillRect(0, 175, 155, 65, ST77XX_BLACK);
    char timeBuf[6];
    char dateBuf[20];
    sprintf(timeBuf, "%02d:%02d", now.tm_hour, now.tm_min);
    sprintf(dateBuf, "%04d.%02d.%02d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

    tft_display(15,180,4,timeBuf);
    tft_display(15,220,2,dateBuf);
    tft.fillRect(0, 170, 320, 5, ST77XX_WHITE);
    TimeUpdate=0;
  }
}

void timeDisplayTask(void *pvParameters) {
  static int lastMin = -1;
  for (;;) { 
    struct tm now;
    if (!getLocalTime(&now)) {
      vTaskDelay(1000 / portTICK_PERIOD_MS); 
      Serial.println("fetch failed");
      continue; 
    }
    if (now.tm_min != lastMin) {
      lastMin = now.tm_min;
      TimeUpdate = 1;

      if ((now.tm_hour >= 23 || now.tm_hour <= 7) && CatSTATE == IDLE) {
        CatSTATE = SLEEP;
        AnimCnt = 0;
      } else if (CatSTATE != JUMP) {
        CatSTATE = IDLE;
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 这行之前漏了！
  }
}