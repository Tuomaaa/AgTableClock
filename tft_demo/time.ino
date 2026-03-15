void UpdateTime(){
  if(TimeUpdate==1){
    tft.fillRect(0, 175, 155, 65, ST77XX_BLACK);
    char timeBuf[6];
    char dateBuf[20];
    sprintf(timeBuf, "%02d:%02d", t.tm_hour, t.tm_min);
    sprintf(dateBuf, "%04d.%02d.%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    
    tft_display(15,180,4,timeBuf);
    tft_display(15,220,2,dateBuf);
    tft.fillRect(0, 170, 320, 5, ST77XX_WHITE);
    //tft.fillRect(150, 170, 5, 70, ST77XX_WHITE);
    TimeUpdate=0;
  }
}

void timeDisplayTask(void *pvParameters) {
  static int lastMin = -1;
  
  // RTOS 的线程必须是一个死循环 (for(;;) 或 while(1))
  // 如果线程意外退出了循环，会导致 CPU 崩溃 (Task Watchdog Triggered)
  for (;;) { 

    if (!getLocalTime(&t)) {
      // RTOS 专属延时：出让 CPU 执行权给其他线程
      vTaskDelay(1000 / portTICK_PERIOD_MS); 
      Serial.println("fetch failed");
      continue; 
    }
    if (t.tm_min != lastMin) {
      lastMin = t.tm_min; // 更新记录的分钟数
      TimeUpdate=1;
      // 每次执行完，必须让出 CPU 一段时间（比如 1 秒）
      if((t.tm_hour>=23 || t.tm_hour<=7 )&&CatSTATE==IDLE){

        CatSTATE=SLEEP;
        AnimCnt=0;
      }
      else if(CatSTATE!=JUMP){
        CatSTATE=IDLE;
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}