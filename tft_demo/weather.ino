
void UpdateWeather(){
  if(weatherUpdate){
    weatherUpdate=false;
    tft.fillRect(160, 175, 160, 65, ST77XX_BLACK);
    char TempBuf[20];
    char WeatherBuf[20];
    sprintf(TempBuf, "%.1f", global_temp);
    sprintf(WeatherBuf ,"%s",getWeatherDescription(global_weather_code));
    
    tft_display(195,195,3,TempBuf);
    tft_display(285,195,1,"O");
    tft_display(293,195,3,"C");
    //tft_display(180,210,3,WeatherBuf);
    displayIcon(160, 195, 2);
  }

}
String getWeatherDescription(int code) {

  switch (code) {
    
    case 0: 
      return "Clear";       // 晴天
    case 1: case 2: case 3: 
      return "Cloudy";      // 多云/阴天
    case 45: case 48: 
      return "Fog";         // 雾
    case 51: case 53: case 55: 
      return "Drizzle";     // 毛毛雨
    case 61: case 63: case 65: 
      return "Rain";        // 下雨
    case 71: case 73: case 75: 
      return "Snow";        // 下雪
    case 95: case 96: case 99: 
      return "Thunder";     // 雷暴
    default: 
      return "Unknown";     // 未知天气
  }
}

void displayIcon(int cx, int cy, int scale){
  const uint8_t *current_icon = icon_unknown; // ✨ 默认指向问号
    if (global_weather_code == 0) {
      current_icon = icon_sun;            // 0: 晴空万里
    } 
    else if (global_weather_code >= 1 && global_weather_code <= 3) {
      current_icon = icon_cloud;          // 1,2,3: 各种多云和阴天
    } 
    else if (global_weather_code == 45 || global_weather_code == 48) {
      current_icon = icon_fog;            // 45,48: 雾霾
    } 
    else if (global_weather_code >= 51 && global_weather_code <= 55) {
      current_icon = icon_drizzle;        // 51,53,55: 毛毛细雨
    } 
    else if (global_weather_code >= 61 && global_weather_code <= 65) {
      current_icon = icon_rain;           // 61,63,65: 各种中大雨
    } 
    else if (global_weather_code >= 71 && global_weather_code <= 77) {
      current_icon = icon_snow;           // 71-77: 各种降雪
    } 
    else if (global_weather_code >= 95 && global_weather_code <= 99) {
      current_icon = icon_storm;          // 95-99: 雷暴天气
    }
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      uint8_t px = current_icon[y * 16 + x];
        if(px>0){
          uint16_t c = weatherColors[px];
          tft.fillRect(cx + x * scale, cy + y * scale, scale, scale, c);
        }
    }
  }

}


void weatherTask(void *pvParameters) {
  
  for (;;) {
    /*
    // 确保 WiFi 连着才去抓数据
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Open-Meteo API，已经配置好特雷霍特 (Terre Haute) 的经纬度，并请求当前天气
      String url = "https://api.open-meteo.com/v1/forecast?latitude=39.4667&longitude=-87.4139&current_weather=true";      
      http.begin(url);
      int httpCode = http.GET(); // 发起 GET 请求

      if (httpCode > 0) {
        // 如果成功抓到数据
        String payload = http.getString();
        
        // 使用 ArduinoJson 提取数据
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          // current_weather=true 时，数据都在 "current_weather" 下面
          float new_temp = doc["current_weather"]["temperature"];
          int new_code   = doc["current_weather"]["weathercode"]; // 注意这里是 weathercode
          
          if (new_temp != global_temp || new_code != global_weather_code) {
            global_temp = new_temp;
            global_weather_code = new_code;
            weatherUpdate = true; // 触发重绘
          }
        }
      }
      http.end(); // 释放资源
    }

    // 极其重要：天气不需要每秒抓！
    // 免费 API 抓太快会被封 IP。每 15 分钟 (900000 毫秒) 抓一次就足够了。
    
    vTaskDelay(900000 / portTICK_PERIOD_MS); */
  
  
  global_temp = -99;
  global_weather_code = esp_random() % 100;
              weatherUpdate = true; // 触发重绘

  vTaskDelay(900000 / portTICK_PERIOD_MS); 
  }
}