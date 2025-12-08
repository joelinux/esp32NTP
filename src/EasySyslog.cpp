#include "EasySyslog.h"
#include <stdarg.h>

EasySyslog::EasySyslog(const char* appName, const char* hostname) {
  strncpy(_appName, appName ? appName : "ESP32App", sizeof(_appName) - 1);
  strncpy(_hostname, hostname ? hostname : "ESP32", sizeof(_hostname) - 1);
}

EasySyslog::~EasySyslog() {
  _udp.stop();
}

bool EasySyslog::begin(const char* server, uint16_t port) {
  IPAddress ip;
  if (!WiFi.hostByName(server, ip)) return false;
  return begin(ip, port);
}

bool EasySyslog::begin(IPAddress server, uint16_t port) {
  _server = server;
  _port = port;
  return _udp.begin(0);  // Local port 0 (any)
}

void EasySyslog::sendLog(SyslogSeverity severity, const char* message) {
  if (!_udp.beginPacket(_server, _port)) return;

  // <PRI> = facility (8 = user) * 8 + severity
  int pri = 8 * 8 + severity;

  // Simple BSD format: <PRI>TAG[PID]: message
  // No timestamp (server will add it on receipt)
  char buf[512];
  int len = snprintf(buf, sizeof(buf), "<%d>%s: %s", pri, _appName, message);

  _udp.write((uint8_t*)buf, len);
  _udp.endPacket();

  if (_echoSerial) {
    Serial.printf("[%s] %s\n", _appName, message);
  }
}

void EasySyslog::log(SyslogSeverity severity, const char* format, ...) {
  char message[256];
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  sendLog(severity, message);
}

// logf is alias
void EasySyslog::logf(SyslogSeverity severity, const char* format, ...) {
  log(severity, format);
}

// Convenience wrappers
void EasySyslog::emerg(const char* format, ...)   { va_list args; va_start(args, format); log(LOG_EMERG, format, args); va_end(args); }
void EasySyslog::alert(const char* format, ...)   { va_list args; va_start(args, format); log(LOG_ALERT, format, args); va_end(args); }
void EasySyslog::crit(const char* format, ...)    { va_list args; va_start(args, format); log(LOG_CRIT, format, args); va_end(args); }
void EasySyslog::err(const char* format, ...)     { va_list args; va_start(args, format); log(LOG_ERR, format, args); va_end(args); }
void EasySyslog::warning(const char* format, ...) { va_list args; va_start(args, format); log(LOG_WARNING, format, args); va_end(args); }
void EasySyslog::notice(const char* format, ...)  { va_list args; va_start(args, format); log(LOG_NOTICE, format, args); va_end(args); }
void EasySyslog::info(const char* format, ...)    { va_list args; va_start(args, format); log(LOG_INFO, format, args); va_end(args); }
void EasySyslog::debug(const char* format, ...)   { va_list args; va_start(args, format); log(LOG_DEBUG, format, args); va_end(args); }

