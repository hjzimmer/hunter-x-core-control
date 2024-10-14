/*!
 *  @file hunter_ctrl.h
 *
 *  This is an abstraction class to control the Hunter XCORE
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef HUNTER_CTRL_H
#define HUNTER_CTRL_H

#include <Arduino.h>
#include "app_data.h"
#include "oled.h"

#define USE_PUMP          false  // set true to control a pump
#define PUMP_PIN_DEFAULT  false  // Set to true to set PUMP_PIN as On by default
#define PUMP_PIN          D1     // GPIO5 = 5 = D1

#define HUNTER_PIN     D0 // GPIO pin 16
//#define ENABLE_PIN 14 // D7 - not used
//#define LED_PIN 2 // LED on D1 mini

class HUNTER_CTRL {
public:
	// constructor
  HUNTER_CTRL() {};
  // public methods
  void initialize(OLED* oled, APP_DATA* appData);
  void startZone(const int zone, const int time);
  void startProgram(const int programID);

private:
  OLED*     oled;
  APP_DATA* appData;
  void switchPump(boolean onOff);

};

#endif // HUNTER_CTRL_H
