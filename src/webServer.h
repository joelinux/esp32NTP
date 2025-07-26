// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko


#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncHTTPUpdateServer.h>

// ****************************************************************************
// WIFI  Variables
// ****************************************************************************

#define WIFIRESET "wifireset"
#define WIFISSID "wifissid"
#define WIFIPASS "wifipass"
#define SECRETKEY "secretkey"

extern void startWebServer();
extern void startOTAServer();

#endif
