// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2025 Joe Lesko

#include "ntpServer.h"

extern void toggleDisplay();
extern ESP32Time rtc;
extern int act_total;
extern  bool ppsFlag;
extern bool timeandDateSet;


TaskHandle_t taskHandle0 = NULL; // task handle for setting/refreshing the time

TinyGPSPlus gps;

bool timeisPPS = false;

bool debugIsOn = false; // set to true to see progress results in the serial monitor, set to false to suppress these
SemaphoreHandle_t mutex;         // used to ensure an NTP request results are not impacted by the process that refreshes the time

// ****************************************************************************
// NTP port and packet buffer
// ****************************************************************************
#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
byte packetBuffer[NTP_PACKET_SIZE];

WiFiUDP Udp;

void startNTPServer()
{
  Udp.begin(NTP_PORT);
}


// ****************************************************************************
//
// ****************************************************************************
void setDateAndTimeFromGPS(void *parameter)
{

  if (debugIsOn)
    Serial.println("Start setDateAndTimeFromGPS task");

  while (true)
  {

    // wait for the ppsFlag to be raised at the start of the 1st second
    ppsFlag = false;

    // The lock gets cleared before we can display it
    // was set. So don't use this option
    // Mark lock false until we get another pulse
    // ppsLock = false;

    while (!ppsFlag)
    {
      vTaskDelay(2 / portTICK_PERIOD_MS);
    };

    // Serial.println("Got PPS Flag. Set Time");

    if (gps.date.isValid() && gps.time.isValid()) // make sure the date and time are valid (in that values are populated)
    {

      struct tm wt;
      wt.tm_year = gps.date.year();
      wt.tm_mon = gps.date.month();
      wt.tm_mday = gps.date.day();
      wt.tm_hour = gps.time.hour();
      wt.tm_min = gps.time.minute();
      wt.tm_sec = gps.time.second();
      if ((wt.tm_year > 1900) && (wt.tm_mon > 0) && (wt.tm_mon < 13) && (wt.tm_mday > 0) && (wt.tm_mday < 32) && (wt.tm_hour < 24) && (wt.tm_min < 60) && (wt.tm_sec < 61)) // make sure the values are within reason
      {
        // wait for the PPS flag to be raised (signifying the true start of the candidate time)
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
        {

          // set the date and time

          // unsigned long pegProcessingAdjustmentEndTime = rtc.getMicros();
          int64_t pegProcessingAdjustmentEndTime = esp_timer_get_time();
          if (debugIsOn)
            Serial.println("Candidate date and time " + String(wt.tm_year) + " " + String(wt.tm_mon) + " " + String(wt.tm_mday) + " " + String(wt.tm_hour) + " " + String(wt.tm_min) + " " + String(wt.tm_sec));

          // set the real time clock
          rtc.setTime(wt.tm_sec, wt.tm_min, wt.tm_hour, wt.tm_mday, wt.tm_mon, wt.tm_year);
          timeandDateSet = true;
          timeisPPS = true;

          // Serial.println("Setting time in GPS Update");

          // release the hold
          xSemaphoreGive(mutex);
          // give some time to ensure the PPS pin is reset
        }
      };
      vTaskDelay(2 / portTICK_PERIOD_MS);
    };
  };
};

// ****************************************************************************
//
// ****************************************************************************
void startGPSUpdateTask()
{
  Serial.println("Task created start");
  // create a mutex to be used to ensure an NTP request results are not impacted by the process that refreshes the time
  mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
      setDateAndTimeFromGPS,
      "Set Date and Time from GPS",
      3000,
      NULL,
      20, // task priority must be reasonably high or the queues from which the gps data is drawn will not be adequately replenished
      &taskHandle0,
      0 // use core 1 to split the load with updateTheDisplay
  );
  Serial.println("Task created end");
};

// ****************************************************************************
//
// ****************************************************************************
void startUDPSever()
{
  Udp.begin(NTP_PORT);
}

// ****************************************************************************
//
// ****************************************************************************
uint32_t microseconds_to_ntp_fraction(uint64_t microseconds)
{
  // Ensure microseconds is within 0 to 999,999 (fractional part)
  microseconds = microseconds % 1000000;

  // Convert to NTP fraction: (microseconds / 1,000,000) * 2^32
  // 2^32 = 4294967296, so use (microseconds * 4294967296) / 1000000
  // To avoid overflow, compute as (microseconds * (2^32 / 1000000))
  double fraction = (double)microseconds * (4294967296.0 / 1000000.0);

  // Round to nearest integer and ensure 32-bit unsigned
  return (uint32_t)(fraction + 0.5);
}

// ****************************************************************************
//
// ****************************************************************************
uint64_t getCurrentTimeInNTP64BitFormat()
{

  const uint64_t numberOfSecondsBetween1900and1970 = 2208988800;

  uint64_t clockSecondsSinceEpoch = numberOfSecondsBetween1900and1970;
  uint64_t microseconds;
  // unsigned long cur_ms = rtc.getMicros();
  unsigned long cur_ms = esp_timer_get_time();
  unsigned long epoch = rtc.getEpoch();

  if (cur_ms >= ppsLastSignalmicro)
  {
    microseconds = uint64_t(cur_ms - ppsLastSignalmicro);
  }
  else
  {
    microseconds = uint64_t(cur_ms) - uint64_t(ppsLastSignalmicro) + 4294967296ULL;
  }

  uint64_t ntpts = ((uint64_t)(clockSecondsSinceEpoch + epoch + 1) << 32) | (uint64_t)(microseconds_to_ntp_fraction(microseconds));
  /*
  Serial.println("Epoch time    " + String(epoch));
  Serial.println("Microseconds " + String(microseconds));
  Serial.println("Return NTP value " + String(ntpts));
  */
  return ntpts;
}

// ****************************************************************************
// send NTP reply
// ****************************************************************************
void sendNTPpacket(IPAddress remoteIP, int remotePort)
{

  // set the receive time to the current time
  uint64_t receiveTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  // Initialize values needed to form NTP request

  // LI: 0, Version: 4, Mode: 4 (server)
  // packetBuffer[0] = 0b00100100;
  // LI: 0, Version: 3, Mode: 4 (server)
  packetBuffer[0] = 0b00011100;

  if (timeisPPS)
  {
    // Stratum, or type of clock
    packetBuffer[1] = 0b00000010;
  }
  else
  {
    // Stratum, or type of clock
    packetBuffer[1] = 0b00001010;
  }
  // Polling Interval
  packetBuffer[2] = 4;

  // Peer Clock Precision
  // log2(sec)
  // 0xF6 <--> -10 <--> 0.0009765625 s
  // 0xF7 <--> -9 <--> 0.001953125 s
  // 0xF8 <--> -8 <--> 0.00390625 s
  // 0xF9 <--> -7 <--> 0.0078125 s
  // 0xFA <--> -6 <--> 0.0156250 s
  // 0xFB <--> -5 <--> 0.0312500 s
  packetBuffer[3] = 0xF7;

  // 8 bytes for Root Delay & Root Dispersion
  // root delay
  packetBuffer[4] = 0;
  packetBuffer[5] = 0;
  packetBuffer[6] = 0;
  packetBuffer[7] = 0;

  // root dispersion
  packetBuffer[8] = 0;
  packetBuffer[9] = 0;
  packetBuffer[10] = 0;
  packetBuffer[11] = 0x50;

  // time source (namestring)
  packetBuffer[12] = 71; // G
  packetBuffer[13] = 80; // P
  packetBuffer[14] = 83; // S
  packetBuffer[15] = 0;

  // get the current time and write it out as the reference time to bytes 16 to 23 of the response packet
  uint64_t referenceTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  packetBuffer[16] = (int)((referenceTime_uint64_t >> 56) & 0xFF);
  packetBuffer[17] = (int)((referenceTime_uint64_t >> 48) & 0xFF);
  packetBuffer[18] = (int)((referenceTime_uint64_t >> 40) & 0xFF);
  packetBuffer[19] = (int)((referenceTime_uint64_t >> 32) & 0xFF);
  packetBuffer[20] = (int)((referenceTime_uint64_t >> 24) & 0xFF);
  packetBuffer[21] = (int)((referenceTime_uint64_t >> 16) & 0xFF);
  packetBuffer[22] = (int)((referenceTime_uint64_t >> 8) & 0xFF);
  packetBuffer[23] = (int)(referenceTime_uint64_t & 0xFF);

  // copy transmit time from the NTP original request to bytes 24 to 31 of the response packet
  packetBuffer[24] = packetBuffer[40];
  packetBuffer[25] = packetBuffer[41];
  packetBuffer[26] = packetBuffer[42];
  packetBuffer[27] = packetBuffer[43];
  packetBuffer[28] = packetBuffer[44];
  packetBuffer[29] = packetBuffer[45];
  packetBuffer[30] = packetBuffer[46];
  packetBuffer[31] = packetBuffer[47];

  // write out the receive time (it was set above) to bytes 32 to 39 of the response packet
  packetBuffer[32] = (int)((receiveTime_uint64_t >> 56) & 0xFF);
  packetBuffer[33] = (int)((receiveTime_uint64_t >> 48) & 0xFF);
  packetBuffer[34] = (int)((receiveTime_uint64_t >> 40) & 0xFF);
  packetBuffer[35] = (int)((receiveTime_uint64_t >> 32) & 0xFF);
  packetBuffer[36] = (int)((receiveTime_uint64_t >> 24) & 0xFF);
  packetBuffer[37] = (int)((receiveTime_uint64_t >> 16) & 0xFF);
  packetBuffer[38] = (int)((receiveTime_uint64_t >> 8) & 0xFF);
  packetBuffer[39] = (int)(receiveTime_uint64_t & 0xFF);

  // get the current time and write it out as the transmit time to bytes 40 to 47 of the response packet
  uint64_t transmitTime_uint64_t = getCurrentTimeInNTP64BitFormat();

  packetBuffer[40] = (int)((transmitTime_uint64_t >> 56) & 0xFF);
  packetBuffer[41] = (int)((transmitTime_uint64_t >> 48) & 0xFF);
  packetBuffer[42] = (int)((transmitTime_uint64_t >> 40) & 0xFF);
  packetBuffer[43] = (int)((transmitTime_uint64_t >> 32) & 0xFF);
  packetBuffer[44] = (int)((transmitTime_uint64_t >> 24) & 0xFF);
  packetBuffer[45] = (int)((transmitTime_uint64_t >> 16) & 0xFF);
  packetBuffer[46] = (int)((transmitTime_uint64_t >> 8) & 0xFF);
  packetBuffer[47] = (int)(transmitTime_uint64_t & 0xFF);

  // send the reply
  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

int bytelen(byte *ba)
{
  int ix = 0;
  while (ba[ix] != 0)
  {
    ix++;
  }
  return (ix);
}

void computeHmac(const char *message, char *hmacResult)
{
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  const size_t keyLen = admin_pw.length();
  const size_t msgLen = strlen(message);
  unsigned char hmacOutput[32]; // SHA256 produces 32 bytes

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)admin_pw.c_str(), keyLen);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)message, msgLen);
  mbedtls_md_hmac_finish(&ctx, hmacOutput);
  mbedtls_md_free(&ctx);

  // Convert to hex string
  for (int i = 0; i < 32; i++)
  {
    sprintf(hmacResult + (i * 2), "%02x", hmacOutput[i]);
  }
}

bool check_hmac(const char *cmd, String &packet)
{
  int len = strlen(cmd);
  bool ret = false;
  char computedHmac[65];

  if (packet.startsWith(cmd))
  {
    String receivedHmac = packet.substring(len + 1);
    computeHmac(cmd, computedHmac);
    if (receivedHmac == String(computedHmac))
    {
      ret = true;
    }
    Serial.printf("Got CMD %s\r\n", cmd);
  }
  return (ret);
}

void sendUDPResponse(const char *buffer)
{
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write((uint8_t *)buffer, strlen(buffer));
  Udp.endPacket();
}

// ****************************************************************************
//
// ****************************************************************************
void processNTPRequests()
{

  unsigned long replyStartTime = rtc.getMicros();
  static int last_sec = -1;

  int packetSize = Udp.parsePacket();

  if (packetSize == NTP_PACKET_SIZE) // an NTP request has arrived
  {

    act_total++;
    // store sender ip for later use
    IPAddress remoteIP = Udp.remoteIP();

    // read the data from the packet into the buffer for later use
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    if (timeandDateSet)
    {
      // Serial.println("timeandDateSet is true. Sending NTP date");
      // hold here if and while the date and time are being refreshed
      // when ok to proceed place a hold on using the mutex to prevent the date and time from being refreshed while the reply packet is being built
      // sendNTPpacket(remoteIP, Udp.remotePort());
      if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
      {
        // send NTP reply
        sendNTPpacket(remoteIP, Udp.remotePort());
        xSemaphoreGive(mutex);
      };
    }

    // report query in serial monitor
    // note: unlike other serial monitor writes in this sketch, this particular write is on the critical path for processing NTP requests.
    // while it does not delay the response to an initial NTP request, if subsequent NTP requests are queued up to run directly afterward
    // this serial monitor write will delay responding to the queued request by approximately 1 milli second.
    if (debugIsOn)
    {
      String updatemessage = "Query from " + remoteIP.toString() + " at  " + String(rtc.getEpoch());
      Serial.println(updatemessage);
    };
    // Serial.println("cur_act = " + String(cur_act) );
  }
  else
  {
    char buffer[1500];
    if (packetSize > 0)
    {
      int len = Udp.read(buffer, 1500);
      if (len > 0)
      {
        buffer[len] = '\0';
      }
      Udp.flush(); // clean up any other bytes left
      String packet = String(buffer);
      // Serial.printf("Got Packet [%.7s]\r\n", packet.c_str());

      // First check commands that need no password

      if (packet.startsWith("stats"))
      {
        static struct tm *tt;

        time_t now_Local_Time = rtc.getEpoch() + tzoffset + 1;
        tt = gmtime(&now_Local_Time);

        sprintf(buffer, "\r\nVersion: %s\r\nFirmware: %s\r\n"
                        "%02d:%02d:%02d\r\n%04d-%02d-%02d\r\nLOCK: %s\r\n"
                        "SATS: %d\r\nWIFI: %s\r\nIP: %s\r\nActivity: %d\r\nWifi RSSI: %d dBm\r\n"
                        "Free Heap Memory: %d\r\n"
                        "ESP32 Chip Model: %s\r\n"
                        "ESP32 Flash Chip Size: %d\r\n",
                ESP32NTP_VER,
                NTP_BOARD_VERSION,
                tt->tm_hour, tt->tm_min, tt->tm_sec,
                tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday,
                ppsLock == true ? "YES" : "NO",
                gps.satellites.value(),
                WiFi.SSID(),
                WiFi.localIP().toString().c_str(),
                act_cnt,
                WiFi.RSSI(),
                ESP.getFreeHeap(),
                ESP.getChipModel(),
                ESP.getFlashChipSize());
        // sprintf(tmps, "  act: %-4d\n", act_cnt);

        // Send back some stats
        // send the reply
        sendUDPResponse(buffer);
      }
      if (packet.startsWith("display"))
      {
	toggleDisplay();
        sprintf(buffer,"Display toggled %s", enable_display ? "On " : "off");
        sendUDPResponse(buffer);
      }

      // Check commands that Need a password
      if (!admin_pw.isEmpty())
      {

        if (check_hmac("reboot", packet))
        {
          sendUDPResponse("Rebooting");
          vTaskDelay(500);
          Serial.println("Valid REBOOT");
          ESP.restart();
        }
        if (check_hmac("reset wifi", packet))
        {
          sendUDPResponse("Restarting");
          vTaskDelay(500);
          ResetSSID();
          ESP.restart();
        }
        sendUDPResponse("Failed Admin Password or invalid command");
        return;
      }

      // coming packet is, but it is not an ntp request so get rid of it
      if (debugIsOn)
        Serial.println("Invalid request received on port " + String(NTP_PORT) + ", length =" + String(packetSize));
    }
  }
}

