#pragma once

#include <Arduino.h>

static int *cpu_stats;
static int *mem_stats;
static int *send_stats;
static int *recv_stats;
static int send_max;
static int recv_max;

static float cpu_percent;
static float last_cpu_percent;
static float ram_percent;
static float last_ram_percent;
static String ram_used;
static String ram_total;
static float disk_percent;
static String disk_used;
static String disk_total;
static String net_send;
static String net_recv;
static int net_send_orig;
static int net_recv_orig;

int count;
int mode;

int32_t text_height, text_width;
int32_t height, width;
