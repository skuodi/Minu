/**
 * @file  utils.h
 * @brief Contains common user definitions and convenience fucntions
 */

#include "M5StickCPlus2.h"
#include "config.h"
#include "utils.h"

void setTimezone(const char* tzone)
{
  Serial.printf("Setting Timezone to %s\n",tzone);
  setenv("TZ",tzone,1);
  tzset();
}

bool syncNtpToRtc(const char* tzone)
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain NTP time");
    return false;
  }
  setTimezone(tzone);
  time_t syncT = time(nullptr) + 1;   // Advance one second
  while (syncT > time(nullptr));      // and wait for the actual time to catch up
  M5.Rtc.setDateTime(gmtime(&syncT)); // Set the new date and time to RTC
  Serial.println(&timeinfo, "Local time: %I:%M:%S%p %A, %B-%d-%Y ");
  return true;
}
