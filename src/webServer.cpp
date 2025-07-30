// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#include <Update.h>
#include "webServer.h"
#include "esp32_home_page.h" // CAUSED THE ESP32 TO CRASH

extern String wifi_ssid;
extern String wifi_pass;
extern String admin_id;
extern String admin_pw;
extern String hostname;
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
                <label for="fssid">SSID WiFi:</label>
                <input type="text" id="fssid" name="fssid" required>
            </div>
            <div class="form-group">
                <label for="fssid_pw">Wifi Password:</label>
                <input type="password" id="fssid_pw" name="fssid_pw" required>
            </div>
            <div class="form-group">
                <label for="adminid">Admin ID:</label>
                <input type="text" value="admin" id="adminid" name="adminid">
            </div>
            <div class="form-group">
                <label for="adminpw1">Admin Password:</label>
                <input type="password" id="adminpw1" name="adminpw1">
            </div>
            <div class="form-group">
                <label for="adminpw2">Repeat Admin Password:</label>
                <input type="password" id="adminpw2" name="adminpw2">
            </div>
            <div class="form-group">
                <label for="fhostname">Hostname, ok to leave blank:</label>
                <input type="text" id="fhostname" name="fhostname">
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
              String fssid = "";
              String fssid_pw = "";
              String fadmin_id = "";
              String fadmin_pw1 = "";
              String fadmin_pw2 = "";
              String fhostname = "";

              Serial.println("Check ssid");
              if (request->hasParam("fssid", true))
              {
                Serial.println("fssid is true");
                fssid = request->getParam("fssid", true)->value();
              }
              if (request->hasParam("fssid_pw", true))
              {
                fssid_pw = request->getParam("fssid_pw", true)->value();
              }
              if (request->hasParam("adminid", true))
              {
                fadmin_id = request->getParam("adminid", true)->value();
              }
              if (request->hasParam("adminpw1", true))
              {
                fadmin_pw1 = request->getParam("adminpw1", true)->value();
              }
              if (request->hasParam("adminpw2", true))
              {
                fadmin_pw2 = request->getParam("adminpw2", true)->value();
              }
              wifi_ssid = fssid;
              wifi_pass = fssid_pw;
              if (!fadmin_pw1.isEmpty() && fadmin_pw1 == fadmin_pw2)
              {
                admin_pw = fadmin_pw1;
              }
              else
              {
                request->send(200, "text/plain", "Passwords didn't match. Restarting...");
                Serial.println("Admin passwords don't match");
                restartServer = true;
                return;
              }
              if (request->hasParam("fhostname", true))
              {
                fhostname = request->getParam("fhostname", true)->value();
              }
              Serial.print("SSID: ");
              Serial.println(fssid);
              /*
              Serial.print("Password: ");
              Serial.println(fssid_pw);
              */
              request->send(200, "text/plain", "Data received successfully! Restarting...");
              prefs.putString(WIFISSID, wifi_ssid);
              prefs.putString(WIFIPASS, wifi_pass);
              prefs.putString(ADMINID, fadmin_id);
              prefs.putString(ADMINPW, fadmin_pw1);
              if (!fhostname.isEmpty())
              {
                prefs.putString(HOSTNAME, fhostname);
              }

              prefs.putBool(WIFIRESET, false);
              vTaskDelay(2000);
              Serial.println("Restarting..");
              restartServer = true;
              // ESP.restart();
            });

  // Start up Web Server
  Serial.println("SETUP Web Server Done");
  server.begin();
  Serial.println("STARTED Web Server Done");
}

void startMgtServer()
{
  Serial.println("Starting Update Server");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
                   return request->requestAuthentication();
                request->send(200, "text/html", updatehtml );
                Serial.println("Web Client Request Main"); });
  server.on("/api/toggle", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
                   return request->requestAuthentication();
                request->redirect("/");  // Implicitly uses 302 Found
                Serial.println("Web Client Toggle Display");
                toggleDisplay(); });
  server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
      return request->requestAuthentication();
    Serial.println("Web Client REBOOT");
                    request->send(200, "text/html", "Restarting server...");
                    restartServer = true; });

  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if(!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
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

              json += "\"hostname\":\"" + hostname +"\"";
              json += ",\"firmware\":\"" + String(ESP32NTP_VER) +"\"";
              json += ",\"board\":\"" + String(NTP_BOARD_VERSION) +"\"";
              json += ",\"uptime\":\"" + String(millis() / 1000) +"\"";
              json += ",\"freeHeap\":\"" + String(ESP.getFreeHeap()) +"\"";
              json += ",\"temperature\":\"" + String(temperatureRead(),1) +"°C\"";

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
              if (!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
              {
                return request->requestAuthentication();
              }
              request->send(200, "text/html", "Fireware updating. Restarting");
              Serial.println("Web Client Request");
              // restartTimer.once_ms(1000,[]{ ESP.restart(); });
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
      if (!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
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
  server.on("/api/admin-config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
    if (!request->authenticate(admin_id.c_str(), admin_pw.c_str()))
    {
      return request->requestAuthentication();
    }

    String body = String((char *)data).substring(0, len);
    Serial.println("Admin config update: " + body);

    // Parse JSON manually (simple approach)
    int adminIdStart = body.indexOf("\"adminId\":\"") + 11;
    int adminIdEnd = body.indexOf("\"", adminIdStart);
    String newAdminId = body.substring(adminIdStart, adminIdEnd);

    int adminPasswordStart = body.indexOf("\"adminPassword\":\"") + 17;
    int adminPasswordEnd = body.indexOf("\"", adminPasswordStart);
    String newAdminPassword = body.substring(adminPasswordStart, adminPasswordEnd);

    int hostnameStart = body.indexOf("\"hostname\":\"") + 12;
    int hostnameEnd = body.indexOf("\"", hostnameStart);
    String newHostname = body.substring(hostnameStart, hostnameEnd);

    // Validate inputs
    if (newAdminId.length() >= 3 && newAdminId.length() <= 32)
    {
      admin_id = newAdminId;
      prefs.putString(ADMINID, admin_id);
      restartServer = true;
    }
    if (newAdminPassword.length() >= 6 && newAdminPassword.length() <= 64)
    {
      admin_pw = newAdminPassword;
      prefs.putString(ADMINPW, admin_pw);
      restartServer = true;
    }
    if (newHostname.length() >= 3 && newHostname.length() <= 32)
    {

      // Update global variables
      hostname = newHostname;

      // Save to preferences
      prefs.putString(HOSTNAME, hostname);
      restartServer = true;
    }

    if ( restartServer ) {
      prefs.end();
      request->send(200, "application/json", "{\"status\":\"success\"}");
      Serial.println("Admin settings updated successfully");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid input parameters\"}");
    } });

  server.on("/api/logout", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(401); });

  server.begin();
}
