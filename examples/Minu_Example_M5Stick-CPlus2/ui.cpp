#include "M5StickCPlus2.h"
#include <WiFi.h>
#include "ESP32Ping.h"
#include <ArduinoUniqueID.h>

#include "../../minu.hpp"
#include "ui.h"
#include "utils.h"
#include "config.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

/// @brief Defines the data to be fetched periodically
typedef enum
{
  UI_UPDATE_TYPE_TIME = 1,
  UI_UPDATE_TYPE_FOB_INFO,
  UI_UPDATE_TYPE_PING,
} UiUpdateType;

/// @brief Information about a Wi-Fi network found during a Wi-Fi scan
typedef struct
{
  String ssid;
  int32_t rssi;
  uint8_t encType;
  uint8_t mac[10];
  int channel;
} UiWiFiScannedNetwork;

TaskHandle_t screenWatchTaskHandle = NULL;
TaskHandle_t screenUpdateTaskHandle = NULL;
TaskHandle_t buttonWatchTaskHandle = NULL;
TaskHandle_t dataUpdateTaskHandle = NULL;

size_t homePageId;
size_t wifiPageId;
size_t scanResultPageId;
size_t pingTargetsPageId;
size_t timePageId;
size_t fobInfoPageId;

static size_t wifiStatusItem;
static ssize_t homepageWifiItem;

long lastButtonADownTime = 0;
long lastButtonBDownTime = 0;
long lastButtonCDownTime = 0;
long currentTime = 0;
bool longPressA;
bool longPressB;
bool longPressC;
bool shortPressA;
bool shortPressB;

void buttonWatchTask(void *arg);
void screenWatchTask(void *arg);
void screenUpdateTask(void *arg);
void dataUpdateTask(void *arg);

void goToFobInfoPage(void *arg = NULL)
{
  menu.goToPage(fobInfoPageId);
}

void goToHomePage(void *arg = NULL)
{
  menu.goToPage(homePageId);
}

void goToWiFiPage(void *arg = NULL)
{
  menu.goToPage(wifiPageId);
}

void goToTimePage(void *arg = NULL)
{
  menu.goToPage(timePageId);
}

void goToScanResultPage(void *arg = NULL)
{
  menu.goToPage(scanResultPageId);
}

void goToPingTargetsPage(void *arg = NULL)
{
  menu.goToPage(pingTargetsPageId);
}

/// @brief Generic function to be called just after the current active page changes
/// @param arg A pointer to the currently active page is passed
void pageOpenedCallback(void *arg)
{
  if (!arg)
    return;
  MinuPage *thisPage = (MinuPage *)arg;
  thisPage->highlightItem(0);
  Serial.printf("Opened page: %s\n", thisPage->title());
}

/// @brief Generic function to be called just before the current active page changes
/// @param arg A pointer to the old active page is passed
void pageClosedCallback(void *arg)
{
  if (!arg)
    return;
  MinuPage *thisPage = (MinuPage *)arg;

  Serial.printf("Closed page: %s\n", thisPage->title());
}

/// @brief Generic function called when the current active page is rendered
/// @param arg A pointer to the currently active page is passed
void pageRenderedCallback(void *arg)
{
  if (!arg)
    return;
  MinuPage *thisPage = (MinuPage *)arg;

  Serial.printf("Rendered page: %s\n", thisPage->title());
}

/// @brief Changes the colour of the Wi-Fi status indicator on the homepage
/// AP  (active) = Green
/// STA (not connected) = Red
/// STA (connected) = Green
/// Wi-Fi not initialized = Grey
void updateWiFiItem(void *arg)
{
  if (!arg)
    return;

  MinuPageItem *thisItem = (MinuPageItem *)arg;

  if (WiFi.getMode() == WIFI_MODE_AP)
    thisItem->setAuxTextBackground(GREEN);
  else if (WiFi.getMode() == WIFI_MODE_STA)
  {
    if (WiFi.status() == WL_CONNECTED)
      thisItem->setAuxTextBackground(GREEN);
    else
      thisItem->setAuxTextBackground(RED);
  }
  else
    thisItem->setAuxTextBackground(TFT_GREY);
}

/// @brief Prints the Wi-Fi status and details when the Wi-Fi status item (<--) on the Wi-Fi page is highlighted
void lcdPrintWiFiStatus(void *page)
{
  if (page)
    if (menu.currentPageId() == wifiPageId && (*(MinuPage *)page).highlightedIndex() != wifiStatusItem)
      return;

  M5.Lcd.setTextColor(MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
  if (WiFi.getMode() == WIFI_MODE_NULL)
  {
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.println("WiFi not INIT!");
    M5.Lcd.setTextColor(MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
    return;
  }
  else
    M5.Lcd.printf("\n%s:", WiFi.getMode() == WIFI_MODE_AP ? "(AP) " : "(STA)");

  if ((WiFi.status() == WL_CONNECTED) || WiFi.getMode() == WIFI_MODE_AP)
  {
    M5.Lcd.setTextColor(BLACK, GREEN);
    M5.Lcd.println(" ");
    M5.Lcd.setTextColor(MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);

    M5.Lcd.print("SSID :");
    M5.Lcd.println((WiFi.getMode() == WIFI_MODE_AP) ? WiFi.softAPSSID().c_str() : WiFi.SSID().c_str());

    M5.Lcd.print("IPAdr:");
    M5.Lcd.println((WiFi.getMode() == WIFI_MODE_AP) ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str());

    return;
  }
  M5.Lcd.setTextColor(BLACK, RED);
  M5.Lcd.println(" ");
  M5.Lcd.setTextColor(MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
}

/// @brief Sync RTC time with NTP server and RTC time
void lcdPrintTime(void *arg = NULL)
{
	syncNtpToRtc(TIMEZONE);
  auto dt = M5.Rtc.getDateTime();
  Serial.printf("%02d:%02d:%02dH\n%02d/%02d/%04d\n",
                dt.time.hours, dt.time.minutes, dt.time.seconds,
                dt.date.date, dt.date.month, dt.date.year);
  M5.Lcd.printf("%02d:%02d:%02dH\n%02d/%02d/%04d\n",
                dt.time.hours, dt.time.minutes, dt.time.seconds,
                dt.date.date, dt.date.month, dt.date.year);
}

/// @brief Print information about the fob's current status
void lcdPrintFobInfo(void *arg = NULL)
{
  M5.Lcd.printf("Name:" MINU_FOB_NAME "\n");
  M5.Lcd.printf("Time:%ld\n", millis());
  M5.Lcd.printf("HWID:0x");
  for (size_t i = 0; i < UniqueIDsize; i++)
    M5.Lcd.print(UniqueID[i], HEX);
  M5.Lcd.printf("\nBATT:%u%%\n", M5.Power.getBatteryLevel());
}

/// @brief Stop the task that periodically performs HTTP requests
void stopDataUpdate(void *arg = NULL)
{
  if (dataUpdateTaskHandle)
  {
    vTaskDelete(dataUpdateTaskHandle);
    dataUpdateTaskHandle = NULL;
  }
}

/// @brief Check whether ping targets can be reached
void updatePingTargetsStatus(void *arg = NULL)
{
  size_t targetCount = pingTargets.size();
  Serial.printf("Pinging %d targets...\n", targetCount);

  for (size_t i = 0; i < targetCount; ++i)
  {
    if (pingTargets[i].useIP)
      pingTargets[i].pingOK = Ping.ping(pingTargets[i].pingIP, 5);
    else
      pingTargets[i].pingOK = Ping.ping(pingTargets[i].fqn.c_str(), 5);
    pingTargets[i].pinged = true;

    if (pingTargets[i].pingOK)
      menu.pages()[pingTargetsPageId]->items()[i].setAuxTextBackground(GREEN);
    else
      menu.pages()[pingTargetsPageId]->items()[i].setAuxTextBackground(RED);
    Serial.printf("Target %d(%s) -> ping %s\n", i, pingTargets[i].pingIP.toString().c_str(), (pingTargets[i].pingOK) ? "OK" : "FAIL");
    if (screenUpdateTaskHandle)
      xTaskNotify(screenUpdateTaskHandle, 1, eSetValueWithOverwrite);
  }
}

/// @brief Starts the task that periodically performs HTTP requests
void startDataUpdate(void *arg = NULL)
{
  UiUpdateType ut;
  if (menu.currentPageId() == timePageId)
    ut = UI_UPDATE_TYPE_TIME;
  else if (menu.currentPageId() == fobInfoPageId)
    ut = UI_UPDATE_TYPE_FOB_INFO;
  else if (menu.currentPageId() == pingTargetsPageId)
  {
    ut = UI_UPDATE_TYPE_PING;
    for (auto item : menu.currentPage()->items())
      item.setAuxTextBackground(MINU_BACKGROUND_COLOUR_DEFAULT);
    menu.currentPage()->highlightItem(0);
    if (screenUpdateTaskHandle)
      xTaskNotify(screenUpdateTaskHandle, 1, eSetValueWithOverwrite);
  }
  else
    return;

  // If the data update task is already running, delete and create it afresh
  stopDataUpdate();
  xTaskCreatePinnedToCore(dataUpdateTask, "Data Update", 4096, (void *)ut, 2, &dataUpdateTaskHandle, ARDUINO_RUNNING_CORE);
}

void startWiFiSTA(void *arg = NULL)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    goToHomePage();
    return;
  }

  if (WiFi.getMode() != WIFI_MODE_STA)
  {
      Serial.printf("Connecting to Wi-Fi: SSID - '%s', Pass - '%s'\n", WIFI_SSID_DEFAULT, WIFI_PASSWORD_DEFAULT);
      WiFi.begin(WIFI_SSID_DEFAULT, WIFI_PASSWORD_DEFAULT);
  }
}

/// @brief Start the Wi-Fi access point
void startWiFiAP(void *arg)
{
  if (WiFi.getMode() != WIFI_MODE_AP)
  {
    WiFi.mode(WIFI_MODE_NULL);
    delay(100);
    WiFi.mode(WIFI_MODE_AP);
    Serial.printf("Starting softAP...\tSSID:'%s'\tPassword:'%s'\n", WIFI_AP_SSID_DEFAULT, WIFI_AP_PASSWORD_DEFAULT);
    WiFi.softAP(WIFI_AP_SSID_DEFAULT, WIFI_AP_PASSWORD_DEFAULT);
  }
  delay(1000);
  goToWiFiPage();
}

/// @brief Delete all child items of a menu page
void deleteAllPageItems(void *arg)
{
  if (!arg)
    return;

  MinuPage *thisPage = (MinuPage *)arg;
  thisPage->removeAllItems();
}

/// @brief Perform a Wi-Fi scan
void startWiFiScan(void *arg)
{
  goToScanResultPage();
  if (screenUpdateTaskHandle)
    xTaskNotify(screenUpdateTaskHandle, 1, eSetValueWithOverwrite);
  while (!menu.rendered())
    delay(10);
  const int cursorX = M5.Lcd.getCursorX();
  const int cursorY = M5.Lcd.getCursorY();

  WiFi.mode(WIFI_MODE_NULL);
  delay(100);
  WiFi.mode(WIFI_MODE_STA);

  M5.Lcd.println("Scanning...");
  int n = WiFi.scanNetworks();
  M5.Lcd.setCursor(cursorX, cursorY);
  M5.Lcd.print("Scan ");

  if (n == 0)
    M5.Lcd.println("done. 0 found.");
  else if (n < 0)
    M5.Lcd.printf("error %d!\n", n);
  else if (n > 0)
  {
    M5.Lcd.printf("done. %d found\n", n);

    for (int i = 0; i < n; ++i)
      menu.pages()[scanResultPageId]->addItem(NULL, WiFi.SSID(i).c_str(), String(WiFi.RSSI(i)).c_str());

    WiFi.scanDelete();
  }
    
  delay(3000);
  menu.pages()[scanResultPageId]->addItem(goToWiFiPage, "<--", NULL);
  if (screenUpdateTaskHandle)
    xTaskNotify(screenUpdateTaskHandle, 1, eSetValueWithOverwrite);
    
  while (!menu.rendered())
    delay(10);
}

void uiMenuInit(void)
{
  MinuPage homePage("HOMEPAGE", menu.numPages());
  homepageWifiItem = homePage.addItem(goToWiFiPage, "Wi-Fi", " ", updateWiFiItem);
  homePage.addItem(goToPingTargetsPage, "Ping targets", NULL);
  homePage.addItem(goToTimePage, "Time", NULL);
  homePage.addItem(goToFobInfoPage, "Fob Info", NULL);
  homePage.setOpenedCallback(pageOpenedCallback);
  homePage.setClosedCallback(pageClosedCallback);
  homePage.setRenderedCallback(pageRenderedCallback);
  homePageId = menu.addPage(homePage);

  MinuPage wifiPage("WI-FI", menu.numPages());
  wifiPage.addItem(startWiFiSTA, "Connect STA", NULL);
  wifiPage.addItem(startWiFiAP, "Start AP", NULL);
  wifiPage.addItem(startWiFiScan, "Scan", NULL);
  wifiStatusItem = wifiPage.addItem(goToHomePage, "<--", NULL);
  wifiPage.setOpenedCallback(pageOpenedCallback);
  wifiPage.setClosedCallback(pageClosedCallback);
  wifiPage.setRenderedCallback(lcdPrintWiFiStatus);
  wifiPageId = menu.addPage(wifiPage);

  MinuPage scanResultPage("SCAN RESULT", menu.numPages());
  scanResultPage.setOpenedCallback(pageOpenedCallback);
  scanResultPage.setClosedCallback(deleteAllPageItems);
  scanResultPage.setRenderedCallback(pageRenderedCallback);
  scanResultPageId = menu.addPage(scanResultPage);

  MinuPage pingTargetsPage("PING TARGETS", menu.numPages());
  for (PingTarget target : pingTargets)
    pingTargetsPage.addItem(NULL, target.displayHostname.c_str(), " ");
  pingTargetsPage.addItem(goToHomePage, "<--", NULL);
  pingTargetsPage.setOpenedCallback(startDataUpdate);
  pingTargetsPage.setClosedCallback(stopDataUpdate);
  pingTargetsPage.setRenderedCallback(pageRenderedCallback);
  pingTargetsPageId = menu.addPage(pingTargetsPage);

  MinuPage timePage("TIME", menu.numPages(), true);
  timePage.addItem(goToHomePage, NULL, NULL);
  timePage.setOpenedCallback(pageOpenedCallback);
  timePage.setClosedCallback(stopDataUpdate);
  timePage.setRenderedCallback(startDataUpdate);
  timePageId = menu.addPage(timePage);

  MinuPage fobInfoPage("FOB INFO", menu.numPages(), true);
  fobInfoPage.addItem(goToHomePage, NULL, NULL);
  fobInfoPage.setOpenedCallback(pageOpenedCallback);
  fobInfoPage.setClosedCallback(stopDataUpdate);
  fobInfoPage.setRenderedCallback(startDataUpdate);
  fobInfoPageId = menu.addPage(fobInfoPage);

  String cookie;
   if ( homePageId < 0 ||
        wifiPageId < 0 ||
        scanResultPageId < 0 ||
        timePageId < 0 ||
        fobInfoPageId < 0)
      goto err;
      

  xTaskCreatePinnedToCore(screenWatchTask, "Screen Watch Task", 4096, NULL, 1, &screenWatchTaskHandle, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(screenUpdateTask, "Screen Update Task", 4096, NULL, 1, &screenUpdateTaskHandle, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(buttonWatchTask, "Button Task", 4096, NULL, 1, &buttonWatchTaskHandle, ARDUINO_RUNNING_CORE);
  goToHomePage();
  return;

err:
  while (1)
  {
    delay(1000);
    Serial.println("Failed to init menu!");
  }
}

/// @brief Watch for any detected button press and manipulate UI accordingly
void screenWatchTask(void *arg)
{
  Serial.println("Started screenWatchTask");
  ssize_t lastSelectedPage = menu.currentPageId();
  ssize_t lastHighlightedItem = menu.currentPage()->highlightedIndex();

  bool updateScreen = false;
  long lastStackCheckTime = 0;

  for (;;)
  {
    // If a short press of A is detected, highlight the next item
    if (shortPressA)
    {
#ifdef UI_BEEP
      M5.Speaker.tone(8000, 30);
#endif
 
      menu.currentPage()->highlightNextItem();
      if(lastHighlightedItem != menu.currentPage()->highlightedIndex())
        updateScreen = true;

      shortPressA = false;
    }

    // If a long press of A or short press of B is detected, select the currently highlighted item
    if (longPressA || shortPressB)
    {
#ifdef UI_BEEP
      M5.Speaker.tone(5000, 30);
#endif
      if (menu.currentPage()->highlightedIndex() >= 0)
      {
        MinuPageItem highlightedItem = menu.currentPage()->highlightedItem();
        if (highlightedItem.link())
          highlightedItem.link()(&highlightedItem);
#ifdef UI_DEBUG_LOG
        Serial.printf("Executed selected item link = %p\n", highlightedItem.link());
#endif
      }

      longPressA = false;
      shortPressB = false;
    }

    // If a long press of B is detected, cancel any ongoing shutdown
    if (longPressB)
    {
      longPressB = false;
    }

    // If the current page has changed, log the change
    if (lastSelectedPage != menu.currentPageId())
    {
#ifdef UI_DEBUG_LOG
      Serial.printf("Changed from page %d to page %d\n", lastSelectedPage, menu.currentPageId());
#endif
      lastSelectedPage = menu.currentPageId();
      lastHighlightedItem = menu.currentPage()->highlightedIndex();
    }

    // If the current page has changed, log the change
    if (lastHighlightedItem != menu.currentPage()->highlightedIndex())
    {
#ifdef UI_DEBUG_LOG
      Serial.printf("Highlighted item changed: Old = %d | New = %d\n", lastHighlightedItem, menu.currentPage()->highlightedIndex());
#endif
      lastHighlightedItem = menu.currentPage()->highlightedIndex();
    }

    if (updateScreen || !menu.rendered())
    {
      updateScreen = false;

      if (screenUpdateTaskHandle)
        xTaskNotify(screenUpdateTaskHandle, 1, eSetValueWithOverwrite);
      while(!menu.rendered())
        delay(10);
    }

#ifdef UI_DEBUG_LOG
    if (lastStackCheckTime + 10000 < millis())
    {
      Serial.printf("Screen Watch Task available stack:  %d * %d bytes\n", uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
      lastStackCheckTime = millis();
    }
#endif
  }
}

void buttonWatchTask(void *arg)
{
  Serial.println("buttonWatchTask started");

  long lastStackCheckTime = 0;

  for (;;)
  {
    M5.update();
    currentTime = millis();
    if (M5.BtnA.wasPressed())
      lastButtonADownTime = currentTime;

    if (M5.BtnA.wasReleased())
    {
      if (lastButtonADownTime + LONG_PRESS_THRESHOLD_MS < currentTime)
      {
        longPressA = true;
#ifdef UI_DEBUG_LOG
        Serial.print("Long");
#endif
      }
      else
      {
        shortPressA = true;
#ifdef UI_DEBUG_LOG
        Serial.print("Short");
#endif
      }
#ifdef UI_DEBUG_LOG
      Serial.printf(" press A: %lums\n", currentTime - lastButtonADownTime);
#endif
    }

    if (M5.BtnB.wasPressed())
      lastButtonBDownTime = currentTime;

    if (M5.BtnB.wasReleased())
    {
      if (lastButtonBDownTime + LONG_PRESS_THRESHOLD_MS < currentTime)
      {
        longPressB = true;
#ifdef UI_DEBUG_LOG
        Serial.print("Long");
#endif
      }
      else
      {
        shortPressB = true;
#ifdef UI_DEBUG_LOG
        Serial.print("Short");
#endif
      }
#ifdef UI_DEBUG_LOG
      Serial.printf(" press B: %ldms\n", currentTime - lastButtonBDownTime);
#endif
    }
#ifdef UI_DEBUG_LOG
    if (lastStackCheckTime + 10000 < currentTime)
    {
      Serial.printf("Button Watch Task available stack:  %d * %d bytes\n", uxTaskGetStackHighWaterMark(NULL), sizeof(portBASE_TYPE));
      lastStackCheckTime = currentTime;
    }
#endif
  }
}

void dataUpdateTask(void *arg)
{
  while (!menu.rendered())
    delay(10);
  
  const int cursorX = M5.Lcd.getCursorX();
  const int cursorY = M5.Lcd.getCursorY();

  int updateType = (int)arg;
#ifdef UI_DEBUG_LOG
  Serial.printf("Started data update task: type %d\n", updateType);
#endif
  M5.Lcd.fillRect(0, cursorY, M5.Lcd.width(), M5.Lcd.height() - cursorY, MINU_BACKGROUND_COLOUR_DEFAULT);

  while (1)
  {
    M5.Lcd.setCursor(cursorX, cursorY);
    Serial.printf("Update type %d started\n", updateType);

    if (updateType == UI_UPDATE_TYPE_TIME)
      lcdPrintTime();
    else if (updateType == UI_UPDATE_TYPE_FOB_INFO)
      lcdPrintFobInfo();
    else if (updateType == UI_UPDATE_TYPE_PING)
      updatePingTargetsStatus();

    Serial.printf("Update type %d done\n", updateType);
    vTaskDelay(pdMS_TO_TICKS(1000));

}

  dataUpdateTaskHandle = NULL;
  vTaskDelete(NULL);
}

/// @brief Performs the actual rendering of the screen upon receiving a task notification
void screenUpdateTask(void *arg)
{
  uint32_t renderMenu;
  while (1)
  {
    renderMenu = ulTaskNotifyTake(false, portMAX_DELAY);
    if (renderMenu == 1)
    {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextSize(TEXT_SIZE_DEFAULT);
      menu.render(MINU_ITEM_MAX_COUNT);
    }
  }
  vTaskDelete(NULL);
}