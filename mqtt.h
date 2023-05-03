#include "CardList.h"
#include "Card.h"

void mqttSetup(bool &isWifiConnected);
void mainMqttLoop(CardList &cardPaths, bool &isWifiConnected);
void checkWifiStatus(bool &isWifiConnected);
