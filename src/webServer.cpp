// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#include "webServer.h"

extern String wifi_ssid;
extern String wifi_pass;
extern String secretKey;
extern Preferences prefs;
extern void ResetSSID();
extern void toggleDisplay();

const char *html = R"(
<!DOCTYPE HTML>
<html>
<head>
<meta charset="UTF-8">
<style>
        .form-container {
            max-width: 400px;
            margin: 20px 0 20px 20px;
            padding: 20px;
            font-family: Arial, sans-serif;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: inline-block;
            width: 100px;
            text-align: right;
            margin-right: 10px;
        }
        input {
            padding: 5px;
            width: 200px;
        }
        button {
            margin-left: 110px;
            padding: 8px 20px;
        }
    </style>
</head>
  <body>
    <h2>NTP Server Configuration </h2>
      <div class="form-container">
        <form id="myForm" action="/submit" method="POST">
            <div class="form-group">
                <label for="input1">SSID WiFi:</label>
                <input type="text" id="input1" name="input1" required>
            </div>
            <div class="form-group">
                <label for="value2">Password:</label>
                <input type="password" id="input2" name="input2" required>
            </div>
            <div class="form-group">
                <label for="input3">Admin Secret:</label>
                <input type="text" id="input3" name="input3">
            </div>
            <button type="submit">Submit</button>
        </form>
    </div>
  </body>
</html>
)";

const char *updatehtml = R"(
<!DOCTYPE HTML>
<html>
<head>
<meta charset="UTF-8">
</head>
  <body>
    <h2>NTP Server Running </h2>
      <li><a href=/toggle>Toggle Display On/Off</a>
      <li><a href=/update>Update Firmware</a>
      <li><a href=/reboot>Reboot System</a>
      <p>
      <p>
      <p>
      <li><a href=/reset>RESET EVERYTHING (Careful)</a>
  </body>
</html>
)";

ESPAsyncHTTPUpdateServer updateServer;
AsyncWebServer server(80);

void startWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      request->send(200, "text/html", html);
      Serial.println("Web Client Request"); });

    server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      String input1 = "";
      String input2 = "";
      String input3 = "";

      Serial.println("Check input 1");
      if (request->hasParam("input1", true)) {
        Serial.println("input1 is true");
        input1 = request->getParam("input1", true)->value();
      }
      if (request->hasParam("input2", true)) {
        input2 = request->getParam("input2", true)->value();
      }
      if (request->hasParam("input3", true)) {
        input3 = request->getParam("input3", true)->value();
      }
      wifi_ssid = input1;
      wifi_pass = input2;
      secretKey = input3;
      Serial.print("SSID: ");
      Serial.println(input1);
      Serial.print("Password: ");
      Serial.println(input2);
      request->send(200, "text/plain", "Data received successfully! Restarting...");
      prefs.putString(WIFISSID, wifi_ssid);
      prefs.putString(WIFIPASS, wifi_pass);
      prefs.putString(SECRETKEY, secretKey);
      prefs.putBool(WIFIRESET, false);
      vTaskDelay(2000);
      Serial.println("Restarting..");
      ESP.restart(); });

    // Start up Web Server
    Serial.println("SETUP Web Server Done");
    server.begin();
    Serial.println("STARTED Web Server Done");
}

void startOTAServer()
{
    Serial.println("Starting Update Server");
    updateServer.setup(&server, "admin", secretKey.c_str());
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml);
                Serial.println("Web Client Request"); });
    server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml);
                Serial.println("Web Client Toggle Display");
                toggleDisplay();
              });
    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml);
                Serial.println("Web Client REBOOT");
                vTaskDelay(1000);
                ESP.restart(); });
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml);
                Serial.println("Web Client RESET");
                vTaskDelay(1000);
                ResetSSID();
                vTaskDelay(2000); // Simple debounce
                ESP.restart(); });

    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(401); });

    server.begin();
}
