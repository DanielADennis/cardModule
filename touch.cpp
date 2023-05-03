#include <LittleFS.h>
#include <SPI.h>

#include "touch.h"

#define CALIBRATION_FILE "/touchCalibrationData"
#define REPEAT_CAL false // Set to true to recalibreate every time
#define WIDTH 320
#define HEIGHT 480

char* keyLabel[4] = {"top", "left", "right", "bottom"};
TFT_eSPI_Button key[4];

void touch_calibrate(TFT_eSPI &tft)
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_GREEN, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void touchSetup(TFT_eSPI &tft) 
{
  touch_calibrate(tft);
  tft.fillScreen(TFT_BLACK);
  key[0].initButton(&tft, (WIDTH/2), 30, WIDTH, 60, TFT_WHITE, TFT_GREEN, TFT_WHITE,
                      keyLabel[0], 2);
  key[1].initButton(&tft, (WIDTH/4), (HEIGHT/2), (WIDTH/2),(HEIGHT-60-60), TFT_WHITE, TFT_RED, TFT_WHITE,
                      keyLabel[1], 2);
  key[2].initButton(&tft, (WIDTH/4)*3, (HEIGHT/2), (WIDTH/2), (HEIGHT-60-60), TFT_WHITE, TFT_BLUE, TFT_WHITE,
                      keyLabel[2], 2);
  key[3].initButton(&tft, (WIDTH/2), (HEIGHT-30), WIDTH, 60, TFT_WHITE, TFT_MAGENTA , TFT_WHITE,
                      keyLabel[3], 2);
}

void getTouchCommands(TFT_eSPI &tft, CardList &cardPaths)
{  

  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);
  
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 4; b++) {
    if (pressed && key[b].contains(t_x, t_y)) 
      key[b].press(true);  // tell the button it is pressed
    else 
      key[b].press(false);  // tell the button it is NOT pressed
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 4; b++) 
  {
    if (key[b].justPressed()) 
      buttonPressActions(b, cardPaths); // Call the button press function and pass the button number
  }
}

void buttonPressActions(int button, CardList &cardPaths)
{
  switch(button)
  {
    case 0:
     Serial.println("top pressed");
     break;
    case 1:
     Serial.println("left pressed");
     if (cardPaths.length > 1)
       cardPaths.prev();
     break;
    case 2:
     Serial.println("right pressed");
     // set the next card as the active card
     if (cardPaths.length > 1)
        cardPaths.next();
     break;
    case 3:
      int activeI = cardPaths.activeCardIndex;
      if (activeI >= 0)
      {
        cardPaths.cards[activeI].isBackActive = !cardPaths.cards[activeI].isBackActive;
        //cardPaths.getActiveCard().isBackActive = !cardPaths.getActiveCard().isBackActive; // toggle the back card
        Serial.println("Back Enabled:" + String(cardPaths.cards[activeI].isBackActive));        
      }

     break;
  }
}
