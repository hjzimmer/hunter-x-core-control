/*!
 *  @file wifi.h
 *
 *  This is an abstraction class for all WIFI related code.
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution
 */
 
#ifndef WIFI_CTRL_H
#define WIFI_CTRL_H

#include <Arduino.h>
#include "ESP8266WiFi.h" //https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
#include "oled.h"
#include "app_data.h"

#define WIFI_SSID              "WLAN_SSID"     // default wifi SSID
#define WIFI_PASSWORD          "WLAN_PASSWORD"    // default wifi password

class WIFI_CTRL {
public:
	// constructor
	WIFI_CTRL() : oled(nullptr), appData(nullptr) {};
  // public methods
  void    initialize(OLED* oled, APP_DATA* appData);
  void    loop();
  boolean connect();

private:
    OLED* oled;
    APP_DATA* appData;
    String getOwnIp();
};

#endif // WIFI_CTRL_H
