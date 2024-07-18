/**
 * @file  utils.h
 * @brief Contains common user definitions
 */

#ifndef __MINU_M5CP2_UTILS_H_
#define __MINU_M5CP2_UTILS_H_

#include <stdint.h>

#include <WiFi.h>
#include "config.h"

#define TFT_GREY 0x5AEB  // New colour

struct PingTarget {
  String displayHostname; // User-friendly name of the target
  bool useIP;             // ping by IP if true, by FQN if false
  IPAddress pingIP;       // IP to use when pinging
  String fqn;             // Fully-qualified name to use when pinging
  bool pinged;            // Whether or not a ping has been sent to this host
  bool pingOK;            // result of ping
};

/// @brief List of targets to be pinged
extern std::vector<PingTarget> pingTargets;

/// @brief Modify the timezone of network-synced time
void setTimezone(const char* tzone);

/// @brief Synchronize local RTC time to network time while setting the default timezone
bool syncNtpToRtc(const char* tzone);

#endif