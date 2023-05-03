#ifndef CARD_H
#define CARD_H

#include "Arduino.h"

struct Card
{
  String frontPath;
  String backPath;
  int aCounter;
  int dCounter;
  bool isBackActive;

  Card(String fp, String bp)
  {
    frontPath = fp;
    backPath = bp;
    aCounter = 0;
    aCounter = 0;
    isBackActive = false;
  };
  
  Card()
  {
    frontPath = "";
    backPath = "";
    aCounter = 0;
    aCounter = 0;
    isBackActive = false;
  };
  
  String getActivePath()
  {
    if(isBackActive)
      return backPath;
    else
      return frontPath;
  }
  
};

#endif
