#include <TJpg_Decoder.h>
#include "tft.h"
#include "touch.h"

String activeImagePath = ""; // path of the card being displayed
TFT_eSPI tft = TFT_eSPI();

bool logoLoaded = false;

void tftSetup()
{
  //////////////////////////////////////////////
  //                    TFT                   //
  //////////////////////////////////////////////

  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  
  // Initialise LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  tft.setTextColor(0xFFFF, TFT_DARKGREY);
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(2);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true );

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  //backEnabled = false;

  //////////////////////////////////////////////
  //                  TOUCH                   //
  //////////////////////////////////////////////
  touchSetup(tft);

  // wait for mqtt to connect
}

void tftMainLoop(CardList &cardPaths, bool &isWifiConnected)
{ 
  tft.setTextFont(1);
  if (isWifiConnected)
  {
    tft.setTextColor(TFT_GREEN, TFT_DARKGREY);
    tft.setCursor(0, tft.height() - 16);
    tft.setTextSize(2);
    tft.print("W");
  }
  else
  {
    tft.setTextColor(TFT_RED, TFT_DARKGREY);
    tft.setCursor(0, tft.height() - 16);
    tft.setTextSize(2);
    tft.print("W");    
  }
  
  // check if the next image has been updated
  if (cardPaths.length > 0)
  {
    if (cardPaths.getActiveCard().getActivePath() != activeImagePath)
    {
      loadFile(cardPaths.getActiveCard().getActivePath());
      activeImagePath = cardPaths.getActiveCard().getActivePath();
      logoLoaded = false;
    }

    // print the number of cards
    tft.setTextColor(TFT_BLACK, TFT_DARKGREY);
    tft.setCursor(20, tft.height() - 16);
    tft.setTextSize(2);
    tft.print(String(cardPaths.activeCardIndex+1)+":"+String(cardPaths.length));
  }
  else //scan for existing files
  {
    File root = LittleFS.open("/", "r");
  
    while (File file = root.openNextFile()) 
    {
      String strname = file.name();
      strname = "/" + strname;
      //Serial.println(file.name());
      // If it is not a directory and filename ends in .jpg then load it
      if (!file.isDirectory() && strname.endsWith(".jpg"))
      {
        cardPaths.append(Card(strname, "/b/back.jpg"));
        logoLoaded = false;
        Serial.println(strname);
      }
    }
    root.close();

    Serial.println("Length of the list: " + String(cardPaths.length));
    
    // if there are still no cards, display the logo
    if (cardPaths.length == 0 && !logoLoaded)
    {
      loadFile("/util/logo.jpg");
      activeImagePath = "/util/logo.jpg";
      logoLoaded = true;
    }
  }
  getTouchCommands(tft, cardPaths);
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

void loadFile(String name)
{
  char fileName[32];
  name.toCharArray(fileName, sizeof(fileName));
  
  tft.fillScreen(TFT_BLACK);

  // Get the width and height in pixels of the jpeg if you wish
  uint16_t w = 0, h = 0, scale;
  TJpgDec.getFsJpgSize(&w, &h, fileName, LittleFS); // Note name preceded with "/"
  tft.setRotation(w > h ? 1 : 2);

  for (scale = 1; scale <= 8; scale <<= 1) {
    if (w <= tft.width() * scale && h <= tft.height() * scale) break;
  }
  TJpgDec.setJpgScale(scale);

  // Draw the image, top left at 0,0
  TJpgDec.drawFsJpg(0, 0, fileName, LittleFS);

  char buf[80];
  sprintf(buf, "%s %dx%d 1:%d %u ms", fileName, w, h, scale);
  Serial.println(buf);
}
