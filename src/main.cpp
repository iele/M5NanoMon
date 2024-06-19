#include <M5UnitGLASS2.h>
#include <M5UnitLCD.h>
#include <M5Unified.h>

#include <ArduinoJson.h>

#include "esp_heap_caps.h"
#include "freertos/task.h"

#include "data.hpp"
#include "display.hpp"
#include "serial.hpp"

M5UnitLCD lcd(GPIO_NUM_2, GPIO_NUM_1, 400000);

void debug(void *)
{
  while (true)
  {
    lcd.clear();
    lcd.setFont(&Font2);
    lcd.setCursor(0, 0);
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    lcd.println(info.total_free_bytes);
    lcd.println(info.total_allocated_bytes);
    lcd.println(info.largest_free_block);
    lcd.println(info.minimum_free_bytes);
    lcd.println(info.allocated_blocks);
    lcd.println(info.free_blocks);
    lcd.println(info.total_blocks);
    lcd.display();
    delay(1000);
  }
}

void setup()
{
  Serial.setRxBufferSize(2048);
  Serial.setTxBufferSize(2048);

  auto cfg = M5.config();
  cfg.fallback_board = m5::board_t::board_M5UnitGLASS2;
  cfg.external_display.unit_glass2 = true;
  cfg.unit_glass2.pin_sda = GPIO_NUM_2;
  cfg.unit_glass2.pin_scl = GPIO_NUM_1;
  cfg.unit_glass2.i2c_addr = 0x3c;
  cfg.unit_glass2.i2c_freq = 400000;
  cfg.unit_glass2.i2c_port = I2C_NUM_0;

  M5.begin(cfg);

  pinMode(M5NANO_C6_RGB_LED_PWR_PIN, OUTPUT);
  digitalWrite(M5NANO_C6_RGB_LED_PWR_PIN, HIGH);
  strip.begin();
  strip.show();

  M5.Display.begin();
  M5.Display.setColorDepth(1);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.setFont(&DejaVu18);
  M5.Display.setTextDatum(MC_DATUM);
  M5.Display.setTextColor(TFT_BLACK,TFT_WHITE);
  M5.Display.drawString("M5NanoMon", M5.Display.width() / 2, M5.Display.height() / 2);
  delay(3000);

  M5.addDisplay(lcd);
  lcd.begin();

  init_display();
  cpu_stats = (int *)calloc(width, sizeof(int));
  mem_stats = (int *)calloc(width, sizeof(int));
  send_stats = (int *)calloc(width, sizeof(int));
  recv_stats = (int *)calloc(width, sizeof(int));
  mode = 0;

  line.reserve(2048);

  xTaskCreatePinnedToCore(debug, "debug", 2048, NULL, 21, NULL, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(display, "display", 10240, NULL, 21, NULL, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(serial, "serial", 2048, NULL, 21, NULL, PRO_CPU_NUM);
}

void loop()
{
  M5.update();

  if (M5.BtnA.wasPressed())
  {
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      mode++;
      break;
    case 4:
      mode = 0;
      break;
    }
  }

  if (M5.BtnA.wasHold())
  {
    esp_restart();
  }
  delay(50);
}