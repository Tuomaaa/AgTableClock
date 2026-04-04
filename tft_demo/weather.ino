// weather.ino



int hourlyCode[24];


void UpdateWeather() {
  if (weatherUpdate) {
    weatherUpdate = false;
    weatherCanvas.fillScreen(ST77XX_BLACK);

    switch (weatherPage) {
      case 0: drawWeatherCurrent(); break;
      case 1: drawHistogram();      break;
      //case 2: drawIndoor();         break;
    }

    tft.drawRGBBitmap(WEATHER_X, WEATHER_Y, weatherCanvas.getBuffer(), WEATHER_W, WEATHER_H);
  }
}

void drawWeatherCurrent() {
  char TempBuf[20];
  sprintf(TempBuf, "%.1f", global_temp);

  weatherCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  weatherCanvas.setTextSize(3);
  weatherCanvas.setCursor(35, 20);
  weatherCanvas.print(TempBuf);
  weatherCanvas.setTextSize(1);
  weatherCanvas.setCursor(125, 20);
  weatherCanvas.print("O");
  weatherCanvas.setTextSize(3);
  weatherCanvas.setCursor(133, 20);
  weatherCanvas.print("C");

  displayIconCanvas(0, 20, 2);
}

void drawHistogram() {
  if (!hourlyDataReady) {
    weatherCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    weatherCanvas.setTextSize(1);
    weatherCanvas.setCursor(10, 25);
    weatherCanvas.print("No data");
    return;
  }

  float tMin = hourlyTemp[0], tMax = hourlyTemp[0];
  for (int i = 1; i < 24; i++) {
    if (hourlyTemp[i] < tMin) tMin = hourlyTemp[i];
    if (hourlyTemp[i] > tMax) tMax = hourlyTemp[i];
  }
  if (tMax - tMin < 1.0) tMax = tMin + 1.0;

  int barW = 5;
  int gap = 1;
  int x0 = 4;
  int yBot = 53;
  int maxH = 50;

  for (int i = 0; i < 24; i++) {
    int barH = (int)((hourlyTemp[i] - tMin) / (tMax - tMin) * maxH);
    if (barH < 2) barH = 2;

  int p = hourlyPrecip[i];
  uint16_t barColor;

  if (hourlyCode[i] >= 95 && hourlyCode[i] <= 99) {
    // 雷暴 → 黄色
    barColor = tft.color565(255, 215, 0);
  } else {
    // 白→蓝：r和g从255降到0，b保持255
    uint8_t r = 255 - p * 255 / 100;
    uint8_t g = 255 - p * 255 / 100;
    uint8_t b = 255;
    barColor = tft.color565(r, g, b);

  }

  int bx = x0 + i * (barW + gap);
  weatherCanvas.fillRect(bx, yBot - barH, barW, barH, barColor);

  }
  char hlBuf[20];
  sprintf(hlBuf, "H:%.0f L:%.0f", tMax, tMin);
  weatherCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  weatherCanvas.setTextSize(1);
  weatherCanvas.setCursor(x0, 0);
  weatherCanvas.print(hlBuf);
  weatherCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  weatherCanvas.setTextSize(1);
  weatherCanvas.setCursor(x0,       56); weatherCanvas.print("0h");
  weatherCanvas.setCursor(x0 + 36,  56); weatherCanvas.print("6h");
  weatherCanvas.setCursor(x0 + 72,  56); weatherCanvas.print("12h");
  weatherCanvas.setCursor(x0 + 108, 56); weatherCanvas.print("18h");
}
 /*   
void drawIndoor() {
  weatherCanvas.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  weatherCanvas.setTextSize(1);
  weatherCanvas.setCursor(10, 15); weatherCanvas.print("Indoor");
  weatherCanvas.setCursor(10, 30); weatherCanvas.print("T: --.-C");
  weatherCanvas.setCursor(10, 45); weatherCanvas.print("H: --.-%");
}
*/
void displayIconCanvas(int cx, int cy, int scale) {
  const uint8_t *current_icon = icon_unknown;
  if (global_weather_code == 0)                                        current_icon = icon_sun;
  else if (global_weather_code >= 1  && global_weather_code <= 3)      current_icon = icon_cloud;
  else if (global_weather_code == 45 || global_weather_code == 48)     current_icon = icon_fog;
  else if (global_weather_code >= 51 && global_weather_code <= 55)     current_icon = icon_drizzle;
  else if (global_weather_code >= 61 && global_weather_code <= 65)     current_icon = icon_rain;
  else if (global_weather_code >= 71 && global_weather_code <= 77)     current_icon = icon_snow;
  else if (global_weather_code >= 95 && global_weather_code <= 99)     current_icon = icon_storm;

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      uint8_t px = current_icon[y * 16 + x];
      if (px > 0) {
        weatherCanvas.fillRect(cx + x * scale, cy + y * scale, scale, scale, weatherColors[px]);
      }
    }
  }
}

String getWeatherDescription(int code) {
  switch (code) {
    case 0:                    return "Clear";
    case 1: case 2: case 3:   return "Cloudy";
    case 45: case 48:          return "Fog";
    case 51: case 53: case 55: return "Drizzle";
    case 61: case 63: case 65: return "Rain";
    case 71: case 73: case 75: return "Snow";
    case 95: case 96: case 99: return "Thunder";
    default:                   return "Unknown";
  }
}
void weatherTask(void *pvParameters) {
  for (;;) {
    while (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://api.open-meteo.com/v1/forecast?"
                 "latitude=39.4667&longitude=-87.4139"
                 "&current_weather=true"
                 "&hourly=temperature_2m,precipitation_probability,weathercode"
                 "&timezone=America%2FIndiana%2FIndianapolis"
                 "&forecast_days=2";
                 
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      // 既然不用 HTTPS 救回了内存，我们直接简单粗暴拿 String，避免 Stream timeout 的坑
      String payload = http.getString();
      
      JsonDocument doc; 
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float new_temp = doc["current_weather"]["temperature"];
        int new_code   = doc["current_weather"]["weathercode"];
        global_temp = new_temp;
        global_weather_code = new_code;

        struct tm now;
        getLocalTime(&now);
        int startHour = now.tm_hour;

        JsonArray temps = doc["hourly"]["temperature_2m"];
        JsonArray precs = doc["hourly"]["precipitation_probability"];
        JsonArray codes = doc["hourly"]["weathercode"];

        for (int i = 0; i < 24 && (startHour + i) < (int)temps.size(); i++) {
          hourlyTemp[i]   = temps[startHour + i].as<float>();
          hourlyPrecip[i] = precs[startHour + i].as<int>();
          hourlyCode[i]   = codes[startHour + i].as<int>();
        }
        
        hourlyDataReady = true;
        weatherUpdate = true;
        Serial.println("Weather fetched successfully!");
      } else {
        Serial.print("JSON Parse failed: ");
        Serial.println(error.c_str());
        Serial.println("====== Raw Payload ======");
        Serial.println(payload); // 把罪魁祸首打印出来
        Serial.println("=========================");
      }
    } else {
      Serial.printf("Weather fetch failed, HTTP code: %d\n", httpCode);
    }
    
    http.end();
    vTaskDelay(900000 / portTICK_PERIOD_MS);
  }
}