// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko


#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <TinyGPSPlus.h>
#include <ESP32Time.h>
#include <TimeLib.h>
#include "version.h"

// ****************************************************************************
// WIFI  Variables
// ****************************************************************************

#define WIFIRESET "wifireset"
#define WIFISSID "wifissid"
#define WIFIPASS "wifipass"
#define SECRETKEY "secretkey"
#define ADMINID	 "adminid"
#define ADMINPW	"adminpw"
#define HOSTNAME "hostname"

extern void startWebServer();
extern void startMgtServer();
extern TinyGPSPlus gps;

#endif
