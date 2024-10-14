/*!
 *  @file oled.h
 *
 *  This is an abstraction class for OLED 128x64 display.
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef OLED_H
#define OLED_H

#include "Arduino.h"
// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// #include "SH1106Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// #include "SH1106SPi.h"

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);
#define I2C_ADDR    0x3c
#define SDA_LINE    D3
#define SDC_LINE    D5
// Initialize the OLED display using Wire library
// SSD1306Wire  display(0x3c, D3, D5);
// SH1106 display(0x3c, D3, D5);

#define MAX_LINES  5   
#define MAX_PAGES  2
#define ARRAY_LEN  (MAX_LINES * MAX_PAGES)
#define MAX_CHAR_IN_LINE  40
// Abstraction is for 5 lines of text as defined below.
// since some text for lines are to long, 2 pages with a change in PAGE_DURATION ms is supported
  // display.drawString(0, 0, "Zeile by y 0");      water zone x for y min
  // display.drawString(0, 13, "Zeile by y 13");    SSID        own IP
  // display.drawString(0, 25, "Zeile by y 25");    MQTT IP     MQTT Port
  // display.drawString(0, 38, "Zeile by y 38");    hum & T
  // display.drawString(0, 51, "Zeile by y 51");    aktualle aktion

/*!
 *  @brief  Class that is resonsibly for the display layout and update
 */
class OLED {
public:
  // constructor
  OLED(): display(I2C_ADDR, SDA_LINE, SDC_LINE) {};
  // public methods
  void initialize();
  void updateWifiInfo(const char* ip, const char* ssid);
  void updateMqttInfo(const char* ip, const int port, boolean connected);  
  void updateHunterInfo(const int zone, const int time, const int program=0);
  void updateDHT(const float t, const float h);
  void updateAction(const char* message);
  void updateScreen();
  void displayMessage(const String& message);

private:
  char          lines[ARRAY_LEN][MAX_CHAR_IN_LINE];  // 0-4 sind die page 1, 5-9 die page 2
  uint8_t       lineIndex = 0;
  long          lastPageUpdate = 0;
  boolean       dataChanged;
  SSD1306Wire   display;
};

#endif // OLED_H
