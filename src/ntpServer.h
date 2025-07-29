// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#ifndef _NTPSERVER_H
#define _NTPSERVER_H

#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <esp_timer.h>
#include <TimeLib.h>
#include <mbedtls/md.h> // Include for HMAC-SHA256 (install via Arduino Library Manager)
#include "version.h"

void startGPSUpdateTask();
void processNTPRequests();

void ResetSSID();

extern TinyGPSPlus gps;
extern unsigned long ppsLastSignalmicro;
extern String wifi_ssid;
extern String wifi_pass;
extern String admin_pw;
extern int tzoffset;
extern volatile bool ppsLock;
extern int act_cnt;
extern bool enable_display;


#endif
