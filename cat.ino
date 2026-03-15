const uint8_t* cat_pose;

#define CAT_CANVAS_W 138  // 128 + 左右各6px余量
#define CAT_CANVAS_H 170
#define CAT_AREA_X 0     // canvas 固定在屏幕的位置
#define CAT_AREA_Y 0

GFXcanvas16 catCanvas(CAT_CANVAS_W, CAT_CANVAS_H);

void drawCatwithEyes(int cx, int cy, int scale) {
  catCanvas.fillScreen(ST77XX_BLACK);
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t px = cat_pose[y * 32 + x];
      if (px > 0) {
        catCanvas.fillRect(cx + x * scale,cy + y * scale-8, scale, scale, catColors[px]);
      }
    }
  }
  catCanvas.fillRect(catX+bobX + 4 * 4, catY+bobY+ 20 * 4, 4, 4, catColors[2]);
  catCanvas.fillRect(catX+bobX + 7 * 4, catY+bobY+ 21 * 4, 4, 4, catColors[2]);
  catCanvas.fillRect(catX+bobX + 10 * 4, catY+bobY+ 20 * 4, 4, 4, catColors[2]);
  tft.drawRGBBitmap(CAT_AREA_X, CAT_AREA_Y, catCanvas.getBuffer(), CAT_CANVAS_W, CAT_CANVAS_H);
}

void drawCat(int cx, int cy, int scale) {
  catCanvas.fillScreen(ST77XX_BLACK);
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t px = cat_pose[y * 32 + x];
      if (px > 0) {
        catCanvas.fillRect(cx + x * scale,cy + y * scale-8, scale, scale, catColors[px]);
      }
    }
  }
  tft.drawRGBBitmap(CAT_AREA_X, CAT_AREA_Y, catCanvas.getBuffer(), CAT_CANVAS_W, CAT_CANVAS_H);
}
void RefreshCat(){
  
  
  cat_pose = cat_idle;

  drawCatwithEyes(catX, catY, 4);  // 4x 放大, 居中


}
void UpdateCat(){
  switch(CatSTATE){
    case IDLE:
      if(cnt == 100){
        bobX = (esp_random() % 6)-3;  // 简单上下跳动
        bobY = (esp_random() % 4)-2;  // 简单上下跳动
        cnt = 0;
        
        cat_pose = cat_idle;
        drawCat(catX, catY, 4);  // 4x 放大, 居中
      
        tft.fillRect(catX+bobX + 4 * 4, catY+bobY+ 20 * 4, 4, 4, catColors[2]);
        tft.fillRect(catX+bobX + 7 * 4, catY+bobY+ 21 * 4, 4, 4, catColors[2]);
        tft.fillRect(catX+bobX + 10 * 4, catY+bobY+ 20 * 4, 4, 4, catColors[2]);
      }
      cnt++;
      break;
    case SLEEP:
      cat_pose = cat_sleep;
      
      if(AnimCnt % 30 == 0){
        catX = catX_default;
        catY = catY_default;
        drawCat(catX, catY, 4);
        switch(AnimCnt/30){
          case 5:
            AnimCnt=0;
            tft.fillRect(catX+ 30 * 4, catY+ 11 * 4, 4, 4, catColors[4]);
            tft.fillRect(catX+ 31 * 4, catY+ 10 * 4, 4, 4, catColors[4]);
          case 4:
            tft.fillRect(catX+ 27 * 4, catY+ 13 * 4, 4, 4, catColors[4]);
            tft.fillRect(catX+ 28 * 4, catY+ 12 * 4, 4, 4, catColors[4]);
          case 3: 
            tft.fillRect(catX+ 23 * 4, catY+ 15 * 4, 4, 4, catColors[4]);
            tft.fillRect(catX+ 24 * 4, catY+ 14 * 4, 4, 4, catColors[4]);
            break;
          default:
            tft.fillRect(catX+ 30 * 4, catY+ 11 * 4, 4, 4, catColors[6]);
            tft.fillRect(catX+ 31 * 4, catY+ 10 * 4, 4, 4, catColors[6]);
            tft.fillRect(catX+ 27 * 4, catY+ 13 * 4, 4, 4, catColors[6]);
            tft.fillRect(catX+ 28 * 4, catY+ 12 * 4, 4, 4, catColors[6]);
            tft.fillRect(catX+ 23 * 4, catY+ 15 * 4, 4, 4, catColors[6]);
            tft.fillRect(catX+ 24 * 4, catY+ 14 * 4, 4, 4, catColors[6]);
        }

      }
      AnimCnt++;
      
      break;
    case JUMP:
      
          CatSTATE=IDLE;

      break;
  }

}
