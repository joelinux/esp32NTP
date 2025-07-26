// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

// main.h 

#ifndef _MAIN_H_
#define _MAIN_H_

#include <SoftwareSerial.h>
#include <Wire.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include <TimeLib.h>
#include <Preferences.h>
#include <mbedtls/md.h> // Include for HMAC-SHA256 (install via Arduino Library Manager)
#include <Update.h>
#include <esp_partition.h>


// The following 3 defines are defined in platformio.ini
// #define USE_SSD1306 // Uncomment to use SSD1306 OLED display
// #define USE_ST7789 // Uncomment to use ST7789 TFT display
// #define USE_TFT      // Uncomment to use TFT_eSPI display

// #define USE_TIMEZONE // Uncomment if you edit local timezone below

#ifdef USE_SSD1306
#define OLED_ADDR 0x3C
#endif

#ifdef ESP32_DEV_BOARD
#define SCL_PIN 22
#define SDA_PIN 21
#define RXPIN   16
#define TXPIN   17
#define PPSPIN  36
#define P_RESET 13 // Reset Wifi PIN
#define SERIAL2
#endif

#ifdef BOARD_ESP32S2
// Define I2C pins for ESP32-S2
#define SDA_PIN 8
#define SCL_PIN 9
#define RXPIN   16
#define TXPIN   18
#define PPSPIN  36
#define P_RESET 13 // Reset Wifi PIN
#define SSSERIAL
#endif

#ifdef USE_TFT
#define RXPIN   1
#define TXPIN   3
#define PPSPIN  35
// #define P_RESET THERE IS NO RESET PIN on the CYD
// #define P_RESET 4
#define SSSERIAL
#endif

#ifdef BOARD_ESP32_D1_MINI
#define SCL_PIN 22
#define SDA_PIN 21
#define RXPIN   10
#define TXPIN   9
#define PPSPIN  5
#define P_RESET 4 // Reset Wifi PIN IO4
#define SERIAL2
#endif

#endif // _MAIN_H_
