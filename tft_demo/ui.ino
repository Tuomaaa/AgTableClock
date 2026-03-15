// ui.ino



void handleUI() {
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
}


void tft_display(int x, int y, int size, char data[]){
  tft.setTextSize(size);
  tft.setCursor(x,y);
  tft.println(data);
}
