// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

// ****************************************************************************
// esp32NTP Server
// ****************************************************************************

#include "main.h"
#include "webServer.h"
#include "ntpServer.h"

extern void startNTPServer();

// When you need to configure what AP the device will
// connect to, these values are used in AP mode.

String ssid = "NTPS1";        // Edit here AP name
String password = "12345678"; // Edit here AP password

IPAddress softAPIP(192, 168, 5, 1); // Edit here AP website IP

// If WIFI_NTP_RESET is define, you can send a packet to NTP server to reset the WiFi credentials
// This is useful if you want to change the WiFi credentials without having to press reset button
// You can send a packet with the content "RESET WIFI" to the NTP server
// echo "RESET WIFI" | nc -u <ESP32_IP_ADDRESS> 123
// If you do not want this feature, comment out the line below

#define WIFI_NTP_RESET

#include "DisplayManager.h"

#ifdef USE_SSD1306
Adafruit_SSD1306 ssd1306_display(128, 64, &Wire, -1);
SSD1306Display displayImpl(&ssd1306_display);
#endif
#ifdef USE_ST7789
Adafruit_ST7789 st7789_display = Adafruit_ST7789(LCD_CS, LCD_DC, LCD_RST);
ST7789Display displayImpl(&st7789_display);
#endif
#ifdef USE_TFT
TFT_eSPI tft_display = TFT_eSPI();
TFTDisplay displayImpl(&tft_display);
#endif

bool restartServer = false;
bool factoryReset = false;

DisplayManager *display = &displayImpl;

#ifdef SERIAL1
HardwareSerial &gpsSerial = Serial1;
#else
#ifdef SERIAL2
HardwareSerial &gpsSerial = Serial2;
#else
#ifdef SSSERIAL
SoftwareSerial gpsSerial(RXPIN, TXPIN);
#endif
#endif
#endif

extern void ResetSSID();

Preferences prefs;

// ****************************************************************************
// The TinyGPSPlus object
// ****************************************************************************

unsigned long ppsLastSignalmicro = esp_timer_get_time();
// ESP32Time real time clock
ESP32Time rtc(0);

bool enable_display = true;
bool enable_boot_display;

// ****************************************************************************
// GPS Variables
// ****************************************************************************

volatile bool ppsFlag = false; // GPS one-pulse-per-second flag
volatile bool ppsLock = false; // Used to display if we got a PPS lock
int act_total = 0;
int act_cnt = 0;
long running_act_cnt = 0;

const bool displayTimeZone = false; // set to true to display the time zone, set to false to not display the time zone

bool timeandDateSet = false;

// ****************************************************************************
// Time zone
// ****************************************************************************
#ifdef USE_TIMEZONE
// please see https://github.com/khoih-prog/Timezone_Generic#timechangerules-struct for more information
#include <Timezone.h>
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240}; // UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};  // UTC - 5 hours
TimeChangeRule usMST = {"MST", Second, Sun, Mar, 2, -420}; // UTC - 7 hours

// Timezone myTZ(myDST, mySTD);
Timezone myTZ(usMST, usMST);

// TimeZone
TimeChangeRule *tcr;
#else
int tzoffset = -1;
#endif

// ****************************************************************************
// GPS Pins and Speed
// ****************************************************************************

const int PPSPin = PPSPIN;
static const uint32_t GPSBaud = 9600;

String wifi_ssid = "";
String wifi_pass = "";
String admin_id = "admin";
String admin_pw = "";
String hostname = "";

TaskHandle_t taskHandle1 = NULL; // task handle for setting/refreshing the display

// EEPROM address for reset flag
int Reset = 1;

// The serial connection to the GPS device
// SoftwareSerial ss(RXPIN, TXPIN);

// ****************************************************************************
// Temp varables needing global scopt
// ****************************************************************************

// For stats that happen every 5 seconds
// unsigned long last = 0UL;

// int ix = 0;

// ****************************************************************************
// WIFI Functions
// ****************************************************************************

bool displayStatus()
{
  return (enable_display);
}

void toggleDisplay()
{
  enable_display = display->toggle();
}

bool displayBootStatus()
{
  return (enable_boot_display);
}

void toggleBootDisplay()
{
  if (enable_boot_display)
  {
    enable_boot_display = false;
  }
  else
  {
    enable_boot_display = true;
  }
  prefs.putBool(DISPLAYON, enable_boot_display);
}

// ****************************************************************************
//
// ****************************************************************************
void ResetSSID()
{
  int ix;
  static bool did_reset = false;
  if (!did_reset)
  {
    did_reset = true;
    prefs.clear();
    prefs.putBool(WIFIRESET, true);
    /*
    prefs.putString(WIFISSID, "");
    prefs.putString(WIFIPASS, "");
    prefs.putString(SECRETKEY, "");
    prefs.putString(ADMINID, "");
    prefs.putString(ADMINPW, "");
    prefs.putString(HOSTNAME, "");
    */
    prefs.end();
    Serial.println("Reset");
  }
}

// ****************************************************************************
//
// ****************************************************************************
void WiFiSetup()
{
  static bool ran_once = false;
  static int ix = 0;

  Serial.println("Starting WiFiSetup..");
  Reset = prefs.getBool(WIFIRESET, true);
  wifi_ssid = prefs.getString(WIFISSID, ssid);
  wifi_pass = prefs.getString(WIFIPASS, password);
  admin_id = prefs.getString(ADMINID, "admin");
  admin_pw = prefs.getString(ADMINPW);
  hostname = prefs.getString(HOSTNAME);
  String secretKey = prefs.getString(SECRETKEY);

  if (admin_pw.isEmpty() && !secretKey.isEmpty())
  {
    admin_pw = secretKey;
  }

  /*
  Serial.println("Admin is: " + admin_id);
  Serial.println("Admin pw is: " + admin_pw);
  */
  if (Reset && (ran_once == true))
  {
    Serial.println("Ran WiFiSetup once. Skipping..");
    return;
  }

  if (Reset)
  {
    ran_once = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(softAPIP, IPAddress(0, 0, 0, 0), IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid, password);
    Serial.println("> Configuration mode <");
    Serial.println("Access Point Started!");
    Serial.print("AP SSID: ");
    Serial.print(ssid);
    Serial.print(" Password: ");
    Serial.println(password);
    Serial.println("Configuration page: http://192.168.5.1");

    Serial.println("STARTING Web Server");

    startWebServer();
  }
  else
  {

    // WiFi.disconnect();
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_STA);
    // Get the MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Extract the last 3 bytes and format as hex
    char thostname[15]; // 6 chars + null terminator
    snprintf(thostname, sizeof(thostname), "time-%02X%02X%02X", mac[3], mac[4], mac[5]);
    if (hostname.isEmpty())
    {
      hostname = String(thostname);
    }

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    vTaskDelay(500);

    WiFi.setHostname(hostname.c_str());
    Serial.println("Wifi Hostname is: " + String(hostname));

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // This is a MUST!
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    Serial.println("Wifi hostname reported as " + String(WiFi.getHostname()));
    // Serial.println("SID=" + wifi_ssid + " PW=" + wifi_pass);
    ix = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      vTaskDelay(1000);
      ix++;
      Serial.printf("Connection in progress... %d\r\n", ix);
      if (ix > 7)
      {
        return;
      }
    }
    Serial.println("Connected to the Wi-Fi network.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Disable Wi-Fi power-saving mode
    esp_err_t ret = esp_wifi_set_ps(WIFI_PS_NONE);

    if (ret == ESP_OK)
    {
      Serial.println("Power-saving mode disabled successfully");
    }
    else
    {
      Serial.printf("Failed to disable power-saving mode: %d\n", ret);
    }
    startMgtServer();
  }
}

// ****************************************************************************
//
// ****************************************************************************
void ppsHandlerRising()
{
  // PPS interrupt handler
  // raise the flag that signals the start of the next second
  ppsFlag = true;
  ppsLock = true;
  stratum = 2;

  ppsLastSignalmicro = esp_timer_get_time();
  // Serial.println("PPS PULSE RISING");
}

// ****************************************************************************
//
// ****************************************************************************
void updateTheDisplay(void *parameter)
{
  char buf[100];
  static int wifi_cnt = 0;
  static int set_rtc = 30;
  static int last_sec = -1;
  static int last_hour = -1;
  static int last_pps_sec = -1;
  static int first_set = 0;
  static bool needs_clear = true;
  static int last_sat = -1;
  static struct tm *tt;
  int sats;

  display->init();

  if (!enable_boot_display)
  {
    toggleDisplay();
  }

  while (true)
  {
    if (gps.satellites.value() < 4)
    {
      ppsLock = false;
      stratum = 10;
    }
    else if ( ppsLock) {
      stratum = 2;
    } 
    if (gps.time.isUpdated())
    {
      if ((first_set == 0) && gps.date.isValid() && gps.time.isValid()) // make sure the date and time are valid (in that values are populated)
      {
        struct tm wt;

        wt.tm_year = gps.date.year();
        wt.tm_mon = gps.date.month();
        wt.tm_mday = gps.date.day();
        wt.tm_hour = gps.time.hour();
        wt.tm_min = gps.time.minute();
        wt.tm_sec = gps.time.second();
        // set the real time clock
        if ((wt.tm_year > 1900) && (wt.tm_mon > 0) && (wt.tm_mon < 13) && (wt.tm_mday > 0) && (wt.tm_mday < 32) && (wt.tm_hour < 24) && (wt.tm_min < 60) && (wt.tm_sec < 61)) // make sure the values are within reason
        {
          Serial.println("Setting date and time in Display");
          rtc.setTime(wt.tm_sec, wt.tm_min, wt.tm_hour, wt.tm_mday, wt.tm_mon, wt.tm_year);
          timeandDateSet = true;
          first_set = 1;
        }
      }
      if (last_sec != gps.time.second())
      {
        if (last_hour != gps.time.hour())
        {
          act_cnt = 0;
          last_hour = gps.time.hour();
        }
        act_cnt += act_total;
        running_act_cnt += act_total;
        act_total = 0;
        // Serial.printf("Wifi Status %d : %d\n", WiFi.status(), WL_CONNECTED);
        if (WiFi.status() != WL_CONNECTED)
        {
          wifi_cnt++;
          if (wifi_cnt > 10)
          {
            if (enable_display)
            {
              display->begin();
              display->clear();
#ifdef USE_SSD1306
              display->setTextSize(1);
#else
              display->setTextSize(2);
#endif
              display->setCursor(0, 0);
              if (wifi_ssid == ssid || Reset)
              {
                display->println("SETUP WIFI");
                display->println("SID: " + ssid);
                display->println("PW: " + password);
                display->println(WiFi.softAPIP().toString());
              }
              else
              {
                display->println("LOST WIFI");
                display->println("Wrong PW ?");
                display->println("SID: " + wifi_ssid);
              }
              display->display();
              needs_clear = true;
            }
            // ResetSSID();
            WiFiSetup();
            vTaskDelay(5000);
            // Serial.println("Delaying for wifi");
            wifi_cnt = 0;
          }
          else
          {
            // WiFi.disconnect();
          }
        }
        // Wifi didn't reconnect after 10 attemps.
        if ((WiFi.status() != WL_CONNECTED) && (wifi_cnt > 99))
        {
          ESP.restart();
        }
        if (enable_display)
        {
          char tmps[20];
          display->begin();
          if (needs_clear)
          {
            display->clear();
            needs_clear = false;
          }
          display->setTextSize(2);

#ifdef USE_TFT
          display->setCursor(0, 20);
          /*
          display->greenBtn();
          */
#endif
#ifdef USE_ST7789
          display->setCursor(0, 0);
          display->setTextColor(WHITE);
#endif
#ifdef USE_TIMEZONE
          time_t now_Local_Time = myTZ.toLocal(rtc.getEpoch(), &tcr);
#else
          // Add 1 seconds. Need to figure out why
          time_t now_Local_Time = rtc.getEpoch() + tzoffset + 1;
          tt = gmtime(&now_Local_Time);
#endif

          // sprintf(buf, "%02d:%02d:%02d Z", gps.time.hour(), gps.time.minute(), gps.time.second());
#ifdef USE_TIMEZONE
          sprintf(buf, "%02d:%02d:%02d", hour(now_Local_Time), minute(now_Local_Time), second(now_Local_Time) tcr->abbrev);
#else
          // sprintf(buf, "%02d:%02d:%02d", hour(now_Local_Time), minute(now_Local_Time), second(now_Local_Time));
          sprintf(buf, "%02d:%02d:%02d", tt->tm_hour, tt->tm_min, tt->tm_sec);
#endif
          display->println(buf);
          // sprintf(buf, "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
          // sprintf(buf, "%04d-%02d-%02d", year(now_Local_Time), month(now_Local_Time), day(now_Local_Time));
          sprintf(buf, "%04d-%02d-%02d", tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday);
          display->println(buf);
#ifdef USE_SSD1306
          display->setTextSize(1);
#endif
          display->setTextColor(ORANGE);
          // display->setTextColor(BLUE);
          // display->setTextSize(1);
          display->print("LOCK: ");
          if (ppsLock)
          {
            display->print("YES ");
            // ppsLock=false;
          }
          else
          {
            display->print("NO  ");
          }
          sprintf(tmps, "act: %-6d\n", act_cnt);
          display->print(tmps);
          sats = gps.satellites.value();
          if (sats != last_sat)
          {
            if (sats < last_sat)
            {
              ppsLock = false;
            }
            last_sat = sats;
          }
          // ppsLock = false;
          // display->setTextColor(YELLOW);
          sprintf(tmps, "%2d\n", sats);
          display->print("SATS: " + String(tmps));
          display->setTextColor(GREEN);
          display->print("IP:");
          display->println(WiFi.localIP().toString());
          // display->setTextColor(RED);
          display->print("WiFi: ");
          if (WiFi.SSID() != "")
          {
            display->println(WiFi.SSID());
          }
          else
          {
            display->println("DO SETUP");
          }
          display->display();
        }
      }
    }
    // block for 1 sec
    if (gps.time.second() != last_pps_sec)
    {
      last_pps_sec = gps.time.second();
      ppsFlag = false;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// ****************************************************************************
//
// ****************************************************************************
void startDisplayTask()
{

  xTaskCreatePinnedToCore(
      updateTheDisplay,     // Function that should be called
      "Update the display", // Name of the task (for debugging)
      3000,                 // Stack size (bytes)
      NULL,                 // Parameter to pass
      10,                   // Task priority
      &taskHandle1,         // Task handle
      1                     // use core 0 to split the load with setDateAndTimeFromGPS
  );
}

void checkOTAPartitionSpace()
{
  const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  if (partition != NULL)
  {
    Serial.print("OTA Partition Size: ");
    Serial.print(partition->size);
    Serial.println(" bytes");

    // Note: Free space depends on whether the partition is empty or contains an old image
    // You may need to check if an image is present
  }
  else
  {
    Serial.println("No OTA partition found");
  }
}

// ****************************************************************************
//
// ****************************************************************************
void setup()
{
  // Create a config namespace to store variables for reboot
  prefs.begin("config", false);

  enable_boot_display = prefs.getBool(DISPLAYON, true);

  Serial.begin(115200);

  // Decide on serial type for a board
#ifdef SSSERIAL
  gpsSerial.begin(GPSBaud);
#else
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPIN, TXPIN);
#endif

#ifdef P_RESET
  pinMode(P_RESET, INPUT_PULLUP);
#endif

  Serial.println("Start CPU Frequency: " + String(getCpuFrequencyMhz()) + " MHz");
  // Set fixed CPU frequency (e.g., 240 MHz)

  Serial.println("CPU Frequency: " + String(getCpuFrequencyMhz()) + " MHz");

  Serial.println();

  if (enable_display)
  {
#if defined(BOARD_ESP32S2) || defined(BOARD_ESP32C3)
    Wire.begin(SDA_PIN, SCL_PIN); // SDA, SCL for ESP32S2
#endif
    display->init();
    display->begin();
    display->clear();
    display->setTextColor(WHITE);
    display->setTextSize(2);
    display->setCursor(0, 0);
    display->println("Starting");
    display->println("System");
    display->display();
  }
  WiFiSetup();

  // setup for the use of the pulse-per-second pin
  pinMode(PPSPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PPSPin), ppsHandlerRising, RISING);

  startGPSUpdateTask();

  Serial.println("Got control back");
  // Start NTP Server
  startNTPServer();

  if (enable_display)
  {
    startDisplayTask();
  }

  checkOTAPartitionSpace();
}

// ****************************************************************************
//
// ****************************************************************************
void loop()
{
  double lng;
#ifdef P_RESET
  if (digitalRead(P_RESET) == LOW)
  {
    factoryReset = true;
    vTaskDelay(2000); // Simple debounce
  }
#endif

  if (factoryReset)
  {
    Serial.println("Reset Pressed!");
    vTaskDelay(500);
    display->clear();
    display->setTextColor(WHITE);
    display->setTextSize(2);
    display->setCursor(0, 0);
    display->println("RESET");
    display->println("WIFI");
    display->display();
    ResetSSID();
    ESP.restart();
  }

  while (gpsSerial.available() > 0)
    gps.encode(gpsSerial.read());

  if (gps.charsProcessed() < 10)
  {
    vTaskDelay(1000);
    display->clear();
    display->setTextColor(WHITE);
    display->setTextSize(2);
    display->setCursor(0, 0);
    display->println("NO GPS");
    display->println("ChecK");
    display->println("Wiring");
    display->display();
    Serial.println(F("WARNING: No GPS data.  Check wiring."));
    vTaskDelay(5000);
  }
  processNTPRequests();
  if (tzoffset == -1)
  {
    if (gps.location.isValid())
    {
      lng = gps.location.lng();
      tzoffset = round(lng / 15.0) * 3600;
    }
  }
  if (restartServer)
  {
    vTaskDelay(500);
    ESP.restart();
  }
}
