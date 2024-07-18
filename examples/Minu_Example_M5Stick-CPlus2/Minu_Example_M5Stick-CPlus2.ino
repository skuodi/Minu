#include "M5StickCPlus2.h"
#include "WiFi.h"
#include "time.h"
#include "esp_sntp.h"
#include "ESP32Ping.h"

#include "config.h"
#include "utils.h"

#include "../../minu.hpp"
#include "ui.h"
#include <vector>

static IPAddress primaryDNS(8, 8, 8, 8);   // optional
static IPAddress secondaryDNS(8, 8, 4, 4); // optional

static PingTarget dns = {.displayHostname="DNS server", .useIP=true, .pingIP=primaryDNS, .pinged=false, .pingOK=false};
static PingTarget google = {.displayHostname="Google.com", .useIP=false, .fqn="www.google.com", .pinged=false, .pingOK=false};

std::vector<PingTarget> pingTargets;

#define MIN(x, y) (x < y) ? x : y

void printText(const char *msg, uint8_t len, uint16_t fore = 0xFFFF, uint16_t back = 0x0000)
{
  // Check that the message and message length are valid
  if (!msg || !len)
    return;
  
  // Set printing colours
  M5.Lcd.setTextColor(fore, back);

  // If printing the entire string, no modification is is necessary
  if (strlen(msg) == len)
  {
    M5.Lcd.print(msg);
    return;
  }

  // Otherwise, print the string.
  // If the string is too short, pad it with spaces and if too long, truncate it
  char buff[len + 1];
  memset(buff, ' ', sizeof(buff));
  memcpy(buff, msg, MIN(len, strlen(msg)));
  buff[len] = 0;

  M5.Lcd.print(buff);
}

void printTextInverted(const char *msg, uint8_t len, uint16_t fore = 0xFFFF, uint16_t back = 0x0000)
{
  // Check that the message and message length are valid
  if (!msg || !len)
    return;

  // Set printing colours
  M5.Lcd.setTextColor(back, fore);

  // If printing the entire string, no modification is is necessary
  if (strlen(msg) == len)
  {
    M5.Lcd.print(msg);
    return;
  }

  // Otherwise, print the string.
  // If the string is too short, pad it with spaces and if too long, truncate it
  char buff[len + 1];
  memset(buff, ' ', sizeof(buff));
  memcpy(buff, msg, MIN(len, strlen(msg)));
  buff[len] = 0;

  M5.Lcd.print(buff);
}

/// @brief Create the menu, defining the length of the main and auxiliary texe sections
Minu menu(printText, printTextInverted, MINU_MAIN_TEXT_LEN, MINU_AUX_TEXT_LEN);

void setup()
{
  // Setup the serial terminal 
  Serial.begin(115200);

  // Initialize the global M5 API object for the M5StickCPlus2
  auto cfg = M5.config();
  StickCP2.begin(cfg);

  // Set the orientation of the screen
  M5.Lcd.setRotation(1);
  // Set Lcd brightness. Doesn't seem to actually work
  M5.Lcd.setBrightness(1);
  
  Serial.println("Starting");
  showSplashScreen();

  // If in STA mode but still unconnected, busy wait for connection or Wi-Fi mode change 
  WiFi.begin(WIFI_SSID_DEFAULT, WIFI_PASSWORD_DEFAULT);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  // After showing splash screen, reset text settings to defaults before initializing the menu
  M5.Lcd.setTextColor(MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
  M5.Lcd.setTextSize(TEXT_SIZE_DEFAULT);
  M5.Lcd.setTextWrap(false, false);

  // Populate the list of network ping targets
  pingTargets.push_back(dns);
  pingTargets.push_back(google);

  // Set Wi-Fi hostname. This name shows up, for example, on the list of connected devices on the router settings page
  WiFi.setHostname(MINU_FOB_NAME);

  // Initialize the menu system.
  uiMenuInit();

  // Set the function to be called when network time is successfully synced
  sntp_set_time_sync_notification_cb([](struct timeval *t)
                                     {
  Serial.println("Got time adjustment from NTP!");
  syncNtpToRtc(TIMEZONE);
  });

  esp_sntp_servermode_dhcp(1);
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
}

void loop()
{
  
}

void showSplashScreen()
{
  Serial.println("SplashScreen");
  Serial.println("GOBOX PRO");
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextSize(5);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.println();
  M5.Lcd.println(" MINU");
  M5.Lcd.println(" EXAMPLE  ");
  delay(6000);
  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println("\n Waiting for WiFi...\n");
  M5.Lcd.println(" SSID    :" WIFI_SSID_DEFAULT);
  M5.Lcd.println(" Password:" WIFI_PASSWORD_DEFAULT);
}