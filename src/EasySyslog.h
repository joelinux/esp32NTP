#ifndef EASYSYSLOG_H
#define EASYSYSLOG_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>

#define SYSLOG_PORT 514

// Syslog severity levels (facility is fixed to LOG_USER = 8)
enum SyslogSeverity {
  LOG_EMERG   = 0,  // Emergency
  LOG_ALERT   = 1,
  LOG_CRIT    = 2,  // Critical
  LOG_ERR     = 3,  // Error
  LOG_WARNING = 4,
  LOG_NOTICE  = 5,
  LOG_INFO    = 6,
  LOG_DEBUG   = 7
};

class EasySyslog {
public:
  EasySyslog();
  ~EasySyslog();

  void init(const char *appName, const char *hostname);
  bool begin(const char* server, uint16_t port = SYSLOG_PORT);
  bool begin(IPAddress server, uint16_t port = SYSLOG_PORT);

  void log(SyslogSeverity severity, const char* format, ...);
  void logf(SyslogSeverity severity, const char* format, ...);  // Same as log
  void vlog(SyslogSeverity severity, const char* format, va_list args);

  // Convenience methods
  void emerg(const char* format, ...);
  void alert(const char* format, ...);
  void crit(const char* format, ...);
  void err(const char* format, ...);
  void warning(const char* format, ...);
  void notice(const char* format, ...);
  void info(const char* format, ...);
  void debug(const char* format, ...);

  void setEchoSerial(bool echo) { _echoSerial = echo; }

private:
  void sendLog(SyslogSeverity severity, const char* message);

  WiFiUDP _udp;
  IPAddress _server;
  uint16_t _port;
  bool _init;
  char _appName[64];
  char _hostname[64];
  bool _echoSerial = true;
};

#endif
