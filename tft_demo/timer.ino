// timer.ino
  int timer_sec =0;
  int timer_min = 5;
  bool timerDirty = true;
  unsigned long startTime = 0;
  unsigned long timerDuration = 0;
  int secLeft = 0;
void updateTimer() {


  switch(timerState){
    case TIMER_DISPLAY:
      if(btnConfirm.fell){
        btnConfirm.fell = false;
        timerState = TIMER_SETUP;
        btnDown.fell = false;
        btnUp.fell = false;
        timerDirty = true;
      }

      break;
    case TIMER_SETUP:
      if (btnDown.fell) {
        btnDown.fell = false;
        if (timer_min > 1) timer_min--;
          timerDirty = true;
      }
      if (btnUp.fell) {
        btnUp.fell = false;
        if (timer_min < 99) timer_min++;
          timerDirty = true;
      }
      if (btnConfirm.fell) {
        btnConfirm.fell = false;
        // Calculate duration and snapshot the start time exactly when started
        timerDuration = (60UL * timer_min + timer_sec) * 1000UL;
        startTime = millis(); 
        timerState = TIMER_RUNNING;
        timerDirty = true;
      }  
      break;
    case TIMER_RUNNING:{
      static unsigned long lastSec = 0;
      if (millis() - lastSec >= 1000) {
        lastSec = millis();
        timerDirty = true;
      }
      if (millis() - startTime >= timerDuration) {
        timerState = TIMER_BUZZING;
        timerDirty = true;
      }
      if (btnConfirm.fell) {
        btnConfirm.fell = false;
        timerState = TIMER_DISPLAY;
        currentScreen = SCREEN_CLOCK;
        TimeUpdate = 1;
        catY = catY_default;
        RefreshCat();
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

      }
      break;
    }
    case TIMER_BUZZING:
      if (btnConfirm.fell) {
        btnConfirm.fell = false;
        timerState = TIMER_DISPLAY;
        currentScreen = SCREEN_CLOCK;
        TimeUpdate = 1;
        catY = catY_default;
        RefreshCat();

        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      }
      break;
  }
  if (timerDirty) {
    timerDirty = false;
    drawTimerScreen();
  }
}


void drawTimerScreen() {
  tft.fillRect(0, 175, 155, 65, ST77XX_BLACK);
  static char timeBuf[6];
  static char dataBuf[30];
    int pct = 0;

  if (timerState == TIMER_RUNNING) {
    unsigned long elapsed = millis() - startTime;
    if (elapsed >= timerDuration) {
      secLeft = 0;
      pct = 100;
    } else {
      secLeft = (timerDuration - elapsed) / 1000;
      pct = elapsed * 100 / timerDuration;
    }
  } else if (timerState == TIMER_BUZZING) {
    secLeft = 0;
    pct = 100;

  } else {
    secLeft = timer_min * 60 + timer_sec;
    pct = 0;
  }
  if(timerState!=TIMER_DISPLAY){
    catY = catY_default-100+pct;
    RefreshCat();
    }
  if (timerState == TIMER_BUZZING) {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  } 
  else if(timerState == TIMER_RUNNING || timerState == TIMER_SETUP ){
    tft.setTextColor(tft.color565(255, 215, 0), ST77XX_BLACK);

  }
  else {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  switch (timerState) {
    case TIMER_DISPLAY: sprintf(dataBuf,"[OK] Setup\n\n   [MODE] Back"); break;
    case TIMER_SETUP:   sprintf(dataBuf,"[UP/DN] Min\n\n  [OK] Start"); break;
    case TIMER_RUNNING: sprintf(dataBuf,"[OK] Stop"); break;
    case TIMER_BUZZING: sprintf(dataBuf,"[OK] Dismiss"); break;
  }
  sprintf(timeBuf, "%02d:%02d", secLeft / 60, secLeft % 60);
  tft_display(15, 180, 4, timeBuf);
    //sprintf(dateBuf, "%s, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  tft_display(15,215,1,dataBuf);
}



