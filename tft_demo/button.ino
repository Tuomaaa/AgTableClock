// button.ino



struct Button {
  int pin;
  volatile bool fell;
  unsigned long lastPress;
};

Button btnDown    = {BTN_DOWN,    false, 0};
Button btnConfirm = {BTN_CONFIRM, false, 0};
Button btnUp      = {BTN_UP,      false, 0};
Button btn4       = {BTN_4,       false, 0}; // 新增 Button 4
unsigned long lastActivity = 0;

void IRAM_ATTR isrDown()    { if (millis() - btnDown.lastPress > 200)    { btnDown.fell = true;    btnDown.lastPress = millis();     lastActivity = millis();
} }
void IRAM_ATTR isrConfirm() { if (millis() - btnConfirm.lastPress > 200) { btnConfirm.fell = true; btnConfirm.lastPress = millis();     lastActivity = millis();
} }
void IRAM_ATTR isrUp()      { if (millis() - btnUp.lastPress > 200)      { btnUp.fell = true;      btnUp.lastPress = millis();     lastActivity = millis();
} }
void IRAM_ATTR isrBtn4()    { if (millis() - btn4.lastPress > 200)       {btn4.fell = true;        btn4.lastPress = millis();     lastActivity = millis();
} }
void setupButtons() {
  pinMode(BTN_DOWN,    INPUT_PULLUP);
  pinMode(BTN_CONFIRM, INPUT_PULLUP);
  pinMode(BTN_UP,      INPUT_PULLUP);
  pinMode(BTN_4,       INPUT_PULLUP); // 新增 Pin Mode
  attachInterrupt(BTN_DOWN,    isrDown,    FALLING);
  attachInterrupt(BTN_CONFIRM, isrConfirm, FALLING);
  attachInterrupt(BTN_UP,      isrUp,      FALLING);
  attachInterrupt(BTN_4,       isrBtn4,    FALLING); // 新增 Interrupt
    lastActivity = millis();

}