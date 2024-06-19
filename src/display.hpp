#pragma once

#include <Arduino.h>
#include <M5Unified.h>

#include "data.hpp"

#define ANIMATION 15

M5Canvas canvas;
uint32_t last_time;

void init_display()
{
  canvas.createSprite(M5.Display.width(), M5.Display.height());

  canvas.setCursor(0, 0);
  canvas.setFont(&Font0);
  text_height = canvas.fontHeight();
  text_width = canvas.fontWidth();
  height = M5.Display.height();
  width = M5.Display.width();
}

int animation(int c, int l, int count, int imin, int imax, int omin, int omax)
{
  double ratio = (double)count / (double)ANIMATION;
  double n = -(cos(M_PI * (ratio - 1)) / 2);
  if (n < 0.0)
    n = 0;
  if (n > 1.0)
    n = 1;
  return ::map(n * (c - l) + l, imin, imax, omin, omax);
}

void drawNormal()
{
  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&Font0);
  char cpu_str[20];
  sprintf(cpu_str, "CPU%3d%%", int(cpu_percent));
  auto sw = canvas.textWidth(cpu_str);
  canvas.drawString(cpu_str, 0, 1);
  canvas.drawRect(sw, 0, width - sw, text_height, TFT_WHITE);
  canvas.fillRect(sw, 0, animation(cpu_percent, last_cpu_percent, count, 0, 100, 0, width - sw), text_height, TFT_WHITE);

  char ram_str[20];
  sprintf(ram_str, "RAM%3d%%", int(ram_percent));
  canvas.drawString(ram_str, 0, text_height + 3);
  canvas.drawRect(sw, text_height + 3, width - sw, text_height, TFT_WHITE);
  canvas.fillRect(sw, text_height + 3, animation(ram_percent, last_ram_percent, count, 0, 100, 0, width - sw), text_height, TFT_WHITE);

  char dsk_str[50];
  sprintf(dsk_str, "DSK %0.0f%% %s/%s", disk_percent, disk_used, disk_total);
  canvas.drawString(dsk_str, 0, (text_height + 3) * 2);

  char net_str[50];
  sprintf(net_str, "NET S:%s R:%s", net_send, net_recv);
  canvas.drawString(net_str, 0, (text_height + 3) * 3);

  auto start_stat = (text_height + 3) * 4;
  auto end_stat = height;
  for (int i = 1; i < width; i++)
  {
    int value, old_value = 0;
    int value1, value2, old_value1, old_value2 = 0;
    switch (mode)
    {
    case 0:
    {
      value = cpu_stats[i];
      old_value = cpu_stats[i - 1];
      auto pos_y = animation(value, old_value, count, 0, 100, height, start_stat);
      canvas.drawFastVLine(i, pos_y, (end_stat - pos_y), TFT_WHITE);
      break;
    }
    case 1:
    {
      value = mem_stats[i];
      old_value = mem_stats[i - 1];
      auto pos_y = animation(value, old_value, count, 0, 100, height, start_stat);
      canvas.drawFastVLine(i, pos_y, (end_stat - pos_y), TFT_WHITE);
      break;
    }
    case 2:
    {
      value1 = send_stats[i];
      old_value1 = send_stats[i - 1];
      value2 = recv_stats[i];
      old_value2 = recv_stats[i - 1];
      auto h = (end_stat - start_stat) / 2;
      auto length_y_1 = animation(value1, old_value1, count, 0, send_max, 0, h);
      auto length_y_2 = animation(value2, old_value2, count, 0, recv_max, 0, h);
      canvas.drawFastVLine(i, start_stat + h, -length_y_1, TFT_WHITE);
      canvas.drawFastVLine(i, start_stat + h, length_y_2, TFT_WHITE);
      break;
    }
    }
  }
}

void drawGauge(int x, int y, float usage, float last_usage, float max_usage, String value, const char *label)
{
  int radius = 20;
  int thickness = 5;
  int startAngle = 90;
  int angleRange = 360;

  canvas.drawCircle(x, y, radius, TFT_WHITE);
  canvas.drawCircle(x, y, radius - thickness, TFT_WHITE);

  float disUsage = animation(usage, last_usage, count, 0, max_usage, 0, 100);
  float endUsageAngle = (float)startAngle + disUsage / 100.0 * (float)angleRange;
  canvas.fillArc(x, y, radius, radius - thickness, startAngle, endUsageAngle, TFT_WHITE);

  canvas.drawCircle(x, y, radius, TFT_WHITE);
  canvas.drawCircle(x, y, radius - thickness, TFT_WHITE);

  canvas.setTextDatum(MC_DATUM);
  canvas.setTextSize(1);
  canvas.drawString(label, x, y, &Font0);

  canvas.setTextSize(1);
  canvas.drawString(value, x, y + radius + 8, &Font0);
}

void display(void *)
{
  while (true)
  {
    if (count > ANIMATION)
    {
      continue;
    }
    if (count < ANIMATION)
    {
      count++;
    }

    canvas.clear();
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
      drawNormal();
      break;
    case 3:
      drawGauge(32, 28, cpu_percent, last_cpu_percent, 100, String((int)(cpu_percent)) + "%", "CPU");
      drawGauge(96, 28, ram_percent, last_ram_percent, 100, String((int)(ram_percent)) + "%", "MEM");
      break;
    case 4:
      drawGauge(32, 28, send_stats[M5.Display.width() - 1], send_stats[M5.Display.width() - 2], send_max, net_send, "SEND");
      drawGauge(96, 28, recv_stats[M5.Display.width() - 1], recv_stats[M5.Display.width() - 2], recv_max, net_recv, "RECV");
      break;
    }
    canvas.pushSprite(&M5.Display, 0, 0);

    delay(30);
  }
}
