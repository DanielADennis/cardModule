#ifndef touch_h
#define touch_h

#include <TFT_eSPI.h>
#include "CardList.h"

void buttonPressActions(int button, CardList &cardPaths);
void touch_calibrate(TFT_eSPI &tft);
void touchSetup(TFT_eSPI &tft);
void getTouchCommands(TFT_eSPI &tft, CardList &cardPaths);

#endif
