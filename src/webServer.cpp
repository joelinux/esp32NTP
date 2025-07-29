// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#include <Update.h>
#include "webServer.h"
#include "esp32_home_page.h" // CAUSED THE ESP32 TO CRASH

extern String wifi_ssid;
extern String wifi_pass;
extern String secretKey;
extern ESP32Time rtc;
extern Preferences prefs;
extern int tzoffset;
extern int act_cnt;
extern bool ppsLock;
extern bool restartServer;
extern bool factoryReset;

extern void ResetSSID();
extern void toggleDisplay();
extern bool displayStatus();

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml );
                Serial.println("Web Client Request Main"); });
  server.on("/api/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->redirect("/");  // Implicitly uses 302 Found
                Serial.println("Web Client Toggle Display");
                toggleDisplay(); });
  server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!request->authenticate("admin", secretKey.c_str()))
      return request->requestAuthentication();
    Serial.println("Web Client REBOOT");
                    request->send(200, "text/html", "Restarting server...");
                    restartServer = true; });

  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate("admin", secretKey.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", "Restarting server...");
                Serial.println("Web Client RESET");
               factoryReset = true; });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String json = "{";
              struct tm *tt;
              time_t now_Local_Time;
              char buf[30];

              json += "\"firmware\":\"" + String(ESP32NTP_VER) +"\"";
              json += ",\"board\":\"" + String(NTP_BOARD_VERSION) +"\"";
              json += ",\"uptime\":\"" + String(millis() / 1000) +"\"";
              json += ",\"freeHeap\":\"" + String(ESP.getFreeHeap()) +"\"";
              json += ",\"temperature\":\"" + String(temperatureRead(),1) +"\"";

              json += ",\"ntp\": {";
              json += "\"requestsPerHour\":\"" + String(act_cnt) +"\"";
              json += "}"; // close ntp
              json += ",\"display\": {";
              json += "\"main\":" + String( displayStatus() ? "true" : "false");
              json += "}"; // close display

              json += ",\"gps\": {";
              json += "\"lockStatus\":\"" + String(ppsLock ? "3D Fix" : "No") +"\"";
              json += ",\"hasLock\":" + String(ppsLock ? "true" : "false");
              json += ",\"satellitesInView\":\"" + String(gps.satellites.value()) +"\"";
              json += ",\"datetime\":\"";
              now_Local_Time = rtc.getEpoch() + tzoffset;
              tt = gmtime(&now_Local_Time);
              sprintf(buf, "%04d-%02d-%02d ", tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday);
              json += buf;
              sprintf(buf, "%02d:%02d:%02d\"", tt->tm_hour, tt->tm_min, tt->tm_sec);
              json += buf;
              json += "}"; // close gps

              json += "}";
              request->send(200, "application/json", json);
                Serial.println("Web Client Request status");
              json=String(); });
  server.on("/api/firmware", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (!request->authenticate("admin", secretKey.c_str()))
              {
                return request->requestAuthentication();
              }
              request->send(200, "text/html", "Fireware updating. Restarting");
              Serial.println("Web Client Request");
              // restartTimer.once_ms(1000,[]{ ESP.restart(); });
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
      if (!request->authenticate("admin", secretKey.c_str()))
      {
        return;
      }
      if (!index)
      {
        Serial.printf("Update: %s\n", filename.c_str());
        if (!Update.begin())
        {
          Update.printError(Serial);
        }
      }
      if (len)
      {
        if (Update.write(data, len) != len)
        {
          Update.printError(Serial);
        }
      }
      if (final)
      {
        if (Update.end(true))
        {
          Serial.printf("Update Success: %u bytes\n", index + len);
          Serial.println("Reboot Server");
          restartServer = true;
        }
      }
      /*
      else
      {
        Update.printError(Serial);
      }
        */ });
  server.on("/api/logout", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(401); });

  server.begin();
}
