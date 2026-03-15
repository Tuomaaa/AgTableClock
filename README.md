# ESP32 Desktop Companion 🐱

A pixel-art desktop gadget built on ESP32 — featuring a live clock, animated pixel cat, weather display, indoor sensor readings, countdown timer, and todo list. Designed as a gift.

# Credits:
This project is 100% Vibe Documented and 30% Vibe Coded
## Hardware

| Component | Model | Interface |
|-----------|-------|-----------|
| MCU | ESP32 WROOM-32 | — |
| Display | 2.8" TFT 240×320 (ST7789V) | SPI |
| Sensor | AHT20 (temp/humidity) + BMP280 (pressure) | I2C |
| Buttons | 3× tactile push buttons | GPIO (INPUT_PULLUP) |
| Buzzer | Passive buzzer (optional) | GPIO |
| Power | USB-C 5V | — |
| Enclosure | 3D printed (PLA) | — |

## Pin Map

### SPI — Display

| TFT Pin | ESP32 GPIO |
|---------|------------|
| MOSI (SDA) | 23 |
| SCLK (SCK) | 18 |
| CS | 15 |
| DC | 2 |
| RST | 4 |
| VCC | 3.3V |
| GND | GND |

### I2C — Sensors

| Sensor Pin | ESP32 GPIO |
|------------|------------|
| SDA | 8 |
| SCL | 9 |

AHT20 address: `0x38`, BMP280 address: `0x77` (some modules `0x76`)

### Buttons

| Function | GPIO | Action |
|----------|------|--------|
| MODE | 16 | Switch between Clock ↔ Timer screen |
| UP | 17 | Timer: increase minutes |
| DOWN / CONFIRM | 5 | Timer: decrease minutes / confirm & start / stop |

All buttons use `INPUT_PULLUP` with ISR on `FALLING` edge (active low, one leg to GPIO, other to GND).

## Features

### Clock
- NTP time sync via WiFi (`pool.ntp.org`)
- Timezone: `EST5EDT` (configurable)
- Displays hours, minutes, and date
- Updates every minute

### Pixel Cat
- 32×32 sprite, 4× scaled, rendered via `GFXcanvas16` (zero flicker)
- States: **IDLE** (random eye movement), **SLEEP** (nighttime, animated Zzz), **JUMP** (button triggered)
- Auto-switches to SLEEP between 23:00–07:00

### Weather
- Fetches from [Open-Meteo API](https://open-meteo.com/) (free, no API key needed)
- Displays temperature and pixel-art weather icon
- 7 icon types: sun, cloud, fog, drizzle, rain, snow, storm
- Updates every 15 minutes

### Indoor Sensors
- AHT20: temperature + relative humidity
- BMP280: barometric pressure (hPa)

### Countdown Timer
States: `DISPLAY → SETUP → RUNNING → BUZZING`

| State | MODE (GPIO 16) | UP (GPIO 17) | CONFIRM (GPIO 5) |
|-------|---------------|--------------|-------------------|
| DISPLAY | Back to Clock | — | Enter Setup |
| SETUP | Decrease min | Increase min | Start timer |
| RUNNING | — | — | Stop → Clock |
| BUZZING | — | — | Dismiss → Clock |

- Range: 1–99 minutes
- Shows percentage during countdown
- Text turns red when time's up
- Buzzer sounds for 5 seconds (optional)

### Todo List (WIP)
- Managed via ESP32 WebServer on port 80
- QR code displayed on screen for quick phone access
- Add / toggle / delete items from phone browser

## File Structure

```
tft_demo/
├── tft_demo.ino    # Main: setup(), loop(), globals, WiFi
├── ui.h            # Screen/TimerState enums, externs
├── ui.ino          # Screen switching, button→state routing
├── button.ino      # Button structs, ISRs, setupButtons()
├── timer.ino       # Timer logic + drawTimerScreen()
├── time.ino        # NTP clock display + timeDisplayTask (RTOS)
├── weather.ino     # Weather API fetch + display + icons
├── cat.ino         # Pixel cat rendering + animation states
├── todo.ino        # WebServer handlers + drawTodos()
└── icons.h         # Sprite data: cat frames, weather icons, color palettes
```

## Dependencies

Install via Arduino IDE Library Manager:

- `Adafruit GFX Library`
- `Adafruit ST7735 and ST7789`
- `Adafruit AHTX0`
- `Adafruit BMP280`
- `ArduinoJson`
- `QRCode` (by Richard Moore)

## Setup

1. Install libraries listed above
2. Open `tft_demo/tft_demo.ino` in Arduino IDE
3. Set board: **ESP32 Dev Module**
4. Edit WiFi credentials in `tft_demo.ino`:
   ```cpp
   const char* ssid = "your_ssid";
   const char* password = "your_password";
   ```
5. Upload and open Serial Monitor at 9600 baud
6. Connect to same WiFi on phone, scan QR code for todo list

## Architecture

- **Dual-core RTOS**: time sync and weather fetch run on Core 1 as FreeRTOS tasks, UI runs on Core 0 in `loop()`
- **Dirty flag pattern**: screens only redraw when data changes (`TimeUpdate`, `weatherUpdate`, `timerDirty`)
- **Canvas rendering**: pixel cat uses `GFXcanvas16` for flicker-free sprite animation
- **Hardware SPI**: display uses hardware VSPI for fast refresh (no visible scan line)
- **ISR + debounce**: buttons use hardware interrupts with 200ms debounce

## License

Personal project — not licensed for redistribution.
