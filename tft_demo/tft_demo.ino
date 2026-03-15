#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "icons.h"


#define TFT_MOSI 23  // 必须接 23 (VSPI MOSI)
#define TFT_SCLK 18  // 必须接 18 (VSPI SCLK)
#define TFT_CS   15  
#define TFT_DC   2
#define TFT_RST  4
#define IDLE 0
#define SLEEP 1
#define JUMP 2
#define catX_default 10
#define catY_default 49
static int catX =  catX_default;
static int catY =  catY_default;
const char* ssid = "IfiW 881";
const char* password = "88888888";
TaskHandle_t TaskTimeDisplay;
// 只传 CS, DC 和 RST。库会自动使用引脚 23 和 18 开启高速硬件 SPI
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
static int TimeUpdate=0;
static struct tm t;
volatile float global_temp = -999.0; 
volatile int global_weather_code = -1;  
volatile bool weatherUpdate= false;
static int CatSTATE = IDLE;
int bobX=0;
int bobY=0;
int cnt = 0;
static int AnimCnt;

enum Screen { SCREEN_CLOCK, SCREEN_TIMER };
Screen currentScreen = SCREEN_CLOCK;
enum TimerState { TIMER_DISPLAY, TIMER_SETUP, TIMER_RUNNING, TIMER_BUZZING };
TimerState timerState=TIMER_DISPLAY;

void setup() {
  Serial.begin(9600);
  setupButtons();

  tft.init(240, 320);
  tft.invertDisplay(false);
  tft.setRotation(3);  // 0-3 试到对
  tft.fillScreen(ST77XX_BLACK);
  // 前景色是白，背景色是黑
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connecting...");
  
  WiFi.begin(ssid, password);
  /*
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  tft.println("Connected...");
  */
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 50) { // 增加超时机制
    delay(1000);
    Serial.print(".");
    Serial.print(" Status: ");
    Serial.println(WiFi.status()); // 打印状态码
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
  } else {
    Serial.println("\nConnect Failed. Please check SSID/PW or Hardware.");
  }
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(5, 80);
  tft.setTextSize(4);

  tft.println("Hello World:)");
  // NTP 校时，CST-8 = 中国时区
  configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org");
  xTaskCreatePinnedToCore(
    timeDisplayTask,   // 1. 刚才写的线程函数名
    "TimeDisplay",     // 2. 线程的字符串名字（随便起，调试用）
    4096,              // 3. 分配给这个线程的内存栈大小（TFT 和 sprintf 比较吃内存，给 4096 比较稳妥）
    NULL,              // 4. 传递给线程的参数（不需要就写 NULL）
    1,                 // 5. 线程优先级（0-24，1 就可以了）
    &TaskTimeDisplay,  // 6. 指向前面声明的任务句柄
    1                  // 7. 将这个线程绑定到核心 1 (ESP32 是双核，核心 0 通常处理 WiFi，核心 1 跑业务逻辑)
  );
  xTaskCreatePinnedToCore(weatherTask,
    "WeatherTask",
    4096, NULL,
    1,
    NULL,
    1
  );
  tft.fillScreen(ST77XX_BLACK);


}
static int lastMin = -1;
int lastState = HIGH;
void loop() {
  handleUI();

  switch (currentScreen) {
    case SCREEN_CLOCK:
      UpdateTime();
      break;
    case SCREEN_TIMER:
      updateTimer();
      break;
  }
  UpdateCat();
  UpdateWeather();
  delay(40);
}



