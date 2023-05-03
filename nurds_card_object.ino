#include "mqtt.h"
#include "tft.h"

#include "CardList.h"

bool isWifiConnected = false;
CardList cardPaths;

//////////////
//  Core 0  //
//////////////
void setup()
{
  mqttSetup(isWifiConnected);
}
void loop()
{
  mainMqttLoop(cardPaths, isWifiConnected);
}

//////////////
//  Core 1  //
//////////////
void setup1()
{
  tftSetup();
}

void loop1()
{
  tftMainLoop(cardPaths, isWifiConnected);
}
