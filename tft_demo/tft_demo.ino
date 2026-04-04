#include <WiFiClientSecure.h>
#include <base64.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "icons.h"
//#include <esp_wpa2.h>
#include <esp_eap_client.h>
#include <PubSubClient.h>
#define weatherCanvas (*_weatherCanvas)
#define catCanvas (*_catCanvas)

#define WEATHER_X 160
#define WEATHER_Y 175
#define WEATHER_W 160
#define WEATHER_H 65
#define CAT_CANVAS_W 138  // 128 + 左右各6px余量
#define CAT_CANVAS_H 170
#define CAT_AREA_X 0     // canvas 固定在屏幕的位置
#define CAT_AREA_Y 0

#define TFT_MOSI 23  // 必须接 23 (VSPI MOSI)
#define TFT_SCLK 18  // 必须接 18 (VSPI SCLK)
#define TFT_CS   27
#define TFT_DC   26
#define TFT_RST  25
#define TFT_BL   15
#define SLEEP_TIMEOUT 1800000UL  // 30分钟

bool screenSleeping = false;
#define BTN_UP      33
#define BTN_DOWN    32
#define BTN_CONFIRM 14
#define BTN_4       13
#define IDLE 0
#define SLEEP 1
#define JUMP 2
#define catX_default 10
#define catY_default 49
SET_LOOP_TASK_STACK_SIZE(16384);  
//#define USE_EDUROAM

#ifdef USE_EDUROAM
  const char* ssid     = "eduroam";
  const char* eap_id   = "panh3@rose-hulman.edu";
  const char* eap_pass = "Tuoma060831!";
#else
  const char* ssid     = "IfiW 881";
  const char* password = "88888888";
#endif
GFXcanvas16* _catCanvas = nullptr;
GFXcanvas16* _weatherCanvas = nullptr;
int todoTotalVisible = 0;  // drawTodo 每次更新
static int catX =  catX_default;
static int catY =  catY_default;
bool todoSelectMode = false;
int todoSelectIndex = 0;
int todoScrollOffset = 0;
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
int weatherPage = 0;        // 0=当前天气, 1=histogram, 2=室内
float hourlyTemp[24];
int hourlyPrecip[24];
bool hourlyDataReady = false;

enum Screen { SCREEN_CLOCK, SCREEN_TIMER };
Screen currentScreen = SCREEN_CLOCK;
enum TimerState { TIMER_DISPLAY, TIMER_SETUP, TIMER_RUNNING, TIMER_BUZZING };
TimerState timerState=TIMER_DISPLAY;

// MQTT
const char* mqtt_broker = "69b873fc3df54b608947187aa47548bf.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "Tuostmm";
const char* mqtt_pass = "Tuoma114514";
const char* mqtt_topic_todo = "tomacat/todo";
const char* mqtt_topic_events = "tomacat/events";

WiFiClientSecure mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

// Todo 数据
#define MAX_TODOS 8
struct TodoItem {
  char text[40];
  bool done;
};
TodoItem todoItems[MAX_TODOS];
int todoCount = 0;
bool todoDirty = true;
#define MAX_EVENTS 12
struct EventItem {
  char text[31];
  char course[21];
  uint32_t timestamp;
  char source;       // 'g' or 'm'
  bool checked;
};
EventItem eventItems[MAX_EVENTS];
int eventCount = 0;
bool eventDirty = true;

void setup() {
  Serial.begin(9600);
  pinMode(TFT_BL, OUTPUT);
digitalWrite(TFT_BL, HIGH);  // 开背光
  setupButtons();

  tft.init(240, 320);
  tft.invertDisplay(false);
  tft.setRotation(3);  // 0-3 试到对
  tft.fillScreen(ST77XX_BLACK);
  // 前景色是白，背景色是黑
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connecting to eduroam...");
  eduroamSetUp();
// DNS 测试
  IPAddress ip;
  if (WiFi.hostByName(mqtt_broker, ip)) {
    Serial.print("DNS resolved: ");
    Serial.println(ip);
  } else {
    Serial.println("DNS FAILED");
  }

  WiFiClient rawClient;
  Serial.print("Raw TCP 8883... ");
  if (rawClient.connect(mqtt_broker, 8883, 5000)) {
    Serial.println("OK");
    rawClient.stop();
  } else {
    Serial.println("BLOCKED");
  }

 mqttSetup();
 
  if (WiFi.status() == WL_CONNECTED) {
    String cid = "tomacat-esp-" + String(esp_random() % 0xFFFF, HEX);
    Serial.printf("Pre-canvas heap: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    if (mqttClient.connect(cid.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected!");
      mqttClient.subscribe(mqtt_topic_todo, 1);
      mqttClient.subscribe(mqtt_topic_events, 1); // 增加对 events 的监听
    } else {
      Serial.printf("MQTT failed rc=%d\n", mqttClient.state());
    }
  }

  // TLS 握手完成后再分配 canvas
  _catCanvas = new GFXcanvas16(CAT_CANVAS_W, CAT_CANVAS_H);
  _weatherCanvas = new GFXcanvas16(WEATHER_W, WEATHER_H);
  Serial.printf("Post-canvas heap: %d\n", ESP.getFreeHeap());

  // FreeRTOS tasks...
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(5, 80);
  tft.setTextSize(4);

  tft.println("Hello World:)");
  // NTP 校时，CST-8 = 中国时区
  configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org");
  xTaskCreatePinnedToCore(
    timeDisplayTask,   // 1. 刚才写的线程函数名
    "TimeDisplay",     // 2. 线程的字符串名字（随便起，调试用）
    8192,              // 3. 分配给这个线程的内存栈大小（TFT 和 sprintf 比较吃内存，给 4096 比较稳妥）
    NULL,              // 4. 传递给线程的参数（不需要就写 NULL）
    1,                 // 5. 线程优先级（0-24，1 就可以了）
    &TaskTimeDisplay,  // 6. 指向前面声明的任务句柄
    1                  // 7. 将这个线程绑定到核心 1 (ESP32 是双核，核心 0 通常处理 WiFi，核心 1 跑业务逻辑)
  );
  xTaskCreatePinnedToCore(weatherTask,
    "WeatherTask",
    8192, NULL,
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
  static unsigned long lastHeapPrint = 0;
  if (millis() - lastHeapPrint > 10000) {
    lastHeapPrint = millis();
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  }
  
  mqttReconnect();
mqttClient.loop();
drawTodo();
  delay(40);
}

void eduroamSetUp(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  #ifdef USE_EDUROAM
    esp_eap_client_set_identity((uint8_t *)eap_id, strlen(eap_id));
    esp_eap_client_set_username((uint8_t *)eap_id, strlen(eap_id));
    esp_eap_client_set_password((uint8_t *)eap_pass, strlen(eap_pass));
    esp_eap_client_set_disable_time_check(true);
    esp_wifi_sta_enterprise_enable();
    WiFi.begin(ssid);
  #else
    WiFi.begin(ssid, password);
  #endif
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 120) { // 增加超时机制
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
}



