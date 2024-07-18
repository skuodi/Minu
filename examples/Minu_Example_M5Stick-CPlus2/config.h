/**
 * @file config.h
 * @brief Contains compile-time known definitions that override the functionality of the device
 */

#ifndef _MINU_M5CP2_CONF_H_
#define _MINU_M5CP2_CONF_H_

#include <Arduino.h>
#include "TZ.h"         // Contains timezone definitions Source: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h

// Uncomment the following line to enable verbose debug logging
// #define ENABLE_VERBOSE_DEBUG_LOG
#ifdef ENABLE_VERBOSE_DEBUG_LOG
#define UI_DEBUG_LOG
#define WEBSERVER_DEBUG_LOG
#endif

#define MINU_FOB_NAME           "MinuFob"

#define WIFI_SSID_DEFAULT       "ssid"
#define WIFI_PASSWORD_DEFAULT   "password"

#define WIFI_AP_SSID_DEFAULT       "MinuFob"
#define WIFI_AP_PASSWORD_DEFAULT   "Hello#mINU!"

// Uncomment the following line to enable beeping on every button press
// #define UI_BEEP

/// @brief Port used for the device's local HTTP server
#define HTTP_PORT               80

/// @brief Define NTP servers for time sync
#define NTP_SERVER1             "pool.ntp.org"
#define NTP_SERVER2             "time.nist.gov"

/// @brief Offset from GMT time in seconds
#define GMT_OFFSET_SEC          (3 * 3600)  

/// @brief Daylight saving time
#define DAYLIGHT_OFFSET_SEC     0     

/// @brief Timezone definition
#define TIMEZONE                TZ_America_Chicago


/// @brief Define the default text size for the menu system
///        This also defines the default length of the mennu item main and auxiliary text
#define TEXT_SIZE_DEFAULT 2

#if (TEXT_SIZE_DEFAULT == 2)
#define MINU_MAIN_TEXT_LEN 15
#define MINU_AUX_TEXT_LEN 5
#define MINU_ITEM_MAX_COUNT 6
#elif (TEXT_SIZE_DEFAULT == 3)
#define MINU_MAIN_TEXT_LEN 9
#define MINU_AUX_TEXT_LEN 3
#define MINU_ITEM_MAX_COUNT 3
#else
#error "TEXT_SIZE_DEFAULT must be defined with a value of either 2 or 3"
#endif

#define LONG_PRESS_THRESHOLD_MS     300

#endif