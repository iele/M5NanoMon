#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>

#include "data.hpp"

#define M5NANO_C6_BLUE_LED_PIN 7
#define M5NANO_C6_BTN_PIN 9
#define M5NANO_C6_IR_TX_PIN 3
#define M5NANO_C6_RGB_LED_PWR_PIN 19
#define M5NANO_C6_RGB_LED_DATA_PIN 20

#define NUM_LEDS 1

HWCDC uart = HWCDCSerial;

String line;
JsonDocument doc;

Adafruit_NeoPixel strip(NUM_LEDS, M5NANO_C6_RGB_LED_DATA_PIN,
                        NEO_GRB + NEO_KHZ800);

void serial(void *)
{
  while (true)
  {
    if (uart.available() > 0)
    {
      line.clear();

      delay(100);
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.show();

      while (true)
      {
        char c = (char)uart.read();
        line += c;
        if (c == '\n')
        {
          break;
        }
      }

      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();

      if (auto error = deserializeJson(doc, line))
      {
        uart.print(line.c_str());
        uart.print("\r");
        uart.print(error.c_str());
        uart.print("\n");
        continue;
      }

      last_cpu_percent = cpu_percent;
      last_ram_percent = ram_percent;

      cpu_percent = doc["cpu"]["percent"];

      JsonObject ram = doc["ram"];
      ram_percent = ram["percent"];
      String _ram_used = ram["used"];
      String _ram_total = ram["total"];
      ram_used = _ram_used;
      ram_total = _ram_total;

      JsonObject disk = doc["disk"];
      disk_percent = disk["percent"];
      String _disk_used = disk["used"];
      String _disk_total = disk["total"];
      disk_used = _disk_used;
      disk_total = _disk_total;

      JsonObject net = doc["net"];
      String _net_send = net["send"];
      String _net_recv = net["recv"];
      net_send_orig = net["send_orig"];
      net_recv_orig = net["recv_orig"];
      net_send = _net_send;
      net_recv = _net_recv;

      send_max = recv_max = 0;
      for (int i = 1; i < width; i++)
      {
        cpu_stats[i - 1] = cpu_stats[i];
        mem_stats[i - 1] = mem_stats[i];
        send_stats[i - 1] = send_stats[i];
        recv_stats[i - 1] = recv_stats[i];
        if (send_max < send_stats[i - 1])
          send_max = send_stats[i - 1];
        if (recv_max < recv_stats[i - 1])
          recv_max = recv_stats[i - 1];
      }
      cpu_stats[width - 1] = cpu_percent;
      mem_stats[width - 1] = ram_percent;
      send_stats[width - 1] = net_send_orig;
      recv_stats[width - 1] = net_recv_orig;
      if (send_max < net_send_orig)
        send_max = net_send_orig;
      if (recv_max < net_send_orig)
        recv_max = net_recv_orig;

      uart.print("\n");
      count = 0;
    }
    else
    {
      strip.setPixelColor(0, strip.Color(255, 0, 0));
      strip.show();
    }
  }
}