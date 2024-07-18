#ifndef _MINU_M5CP2_UI_H_
#define _MINU_M5CP2_UI_H_

#include "../../minu.hpp"
#include "config.h"

/// Variables used to keep track of button states
extern long currentTime;
extern bool longPressA;
extern bool longPressB;
extern bool shortPressA;
extern bool shortPressB;

/// Menu and menu pages
extern Minu menu;
extern size_t homePageId;
extern size_t wifiPageId;
extern size_t scanResultPageId;
extern size_t pingTargetsPageId;
extern size_t timePageId;
extern size_t fobInfoPageId;

/// @brief Initialize the menu system and set up child pages and items
void uiMenuInit(void);

/// Task handles for UI threads
extern TaskHandle_t screenWatchTaskHandle;
extern TaskHandle_t screenUpdateTaskHandle;
extern TaskHandle_t buttonWatchTaskHandle;
extern TaskHandle_t dataUpdateTaskHandle;

#endif