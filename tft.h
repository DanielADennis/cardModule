#include "LittleFS.h"
#include "SPI.h"
#include "CardList.h"

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void loadFile(String name);
void tftSetup();
void tftMainLoop(CardList &cardPaths, bool &isWifiConnected);
