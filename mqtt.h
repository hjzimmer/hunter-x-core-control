/*!
 *  @file mqtt.h
 *
 *  This is an abstraction class for all MQTT related code.
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include "ESP8266WiFi.h" //https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
#include "oled.h"
#include "app_data.h"

#define MQTT_SERVER_IP    "MQTT SERVER IP ADDRESS"  // default MQTT broker IP
#define MQTT_SERVER_PORT  MQTT_SERVER_PORT               // default MQTT broker port

#define PRE_MQTT          "hunter"          // preamble of all MQTT topics

class MQTT {
public:
	// constructor
	MQTT() : oled(nullptr), appData(nullptr) {}
	// public methods
  boolean publishDHT(const float temp, const float humidity);
  boolean publishDHTParams(const float temp_level, const int hum_level, const int temp_offset);
  boolean publishMQTT(const char* ip, const int port);
  boolean publishWIFI(const char* ssid, const char* ip);
  void    initialize(OLED* oled, APP_DATA* appData);
  boolean initMqttServer();
  void    loop();
  void    print();

private:
  boolean   reconnect();
  boolean   _publish(const char* topic, const char* payload);
  boolean   _publish(const char* topic, const uint8_t* payload, unsigned int length);
  const String addOrigin(const char* topic);
  OLED*     oled;
  APP_DATA* appData;
};

#endif // MQTT_H
