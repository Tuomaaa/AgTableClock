// button.ino

#define BTN_DOWN    16
#define BTN_CONFIRM 5
#define BTN_UP      17

struct Button {
  int pin;
  volatile bool fell;
  unsigned long lastPress;
};

Button btnDown    = {BTN_DOWN,    false, 0};
Button btnConfirm = {BTN_CONFIRM, false, 0};
Button btnUp      = {BTN_UP,      false, 0};

void IRAM_ATTR isrDown()    { if (millis() - btnDown.lastPress > 200)    { btnDown.fell = true;    btnDown.lastPress = millis(); } }
void IRAM_ATTR isrConfirm() { if (millis() - btnConfirm.lastPress > 200) { btnConfirm.fell = true; btnConfirm.lastPress = millis(); } }
void IRAM_ATTR isrUp()      { if (millis() - btnUp.lastPress > 200)      { btnUp.fell = true;      btnUp.lastPress = millis(); } }

void setupButtons() {
  pinMode(BTN_DOWN,    INPUT_PULLUP);
  pinMode(BTN_CONFIRM, INPUT_PULLUP);
  pinMode(BTN_UP,      INPUT_PULLUP);
  attachInterrupt(BTN_DOWN,    isrDown,    FALLING);
  attachInterrupt(BTN_CONFIRM, isrConfirm, FALLING);
  attachInterrupt(BTN_UP,      isrUp,      FALLING);
}