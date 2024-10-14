/*
  This project is a udp triggered control for a Hunter X-Core Sprinkler controller
  dedicated for an ESP8266 or ESP32. 
  Tested on a ESP8266 with version v3.0.2 in the Arduino IDE Board Manager
  
  Installation:
  - set initial SSID and the password in the programm below to your Wifi, can be changed via UDP
  - Connect GPIO 16 of the ESP to the REM pin of the X-Core controller.
  - Connect an ESP-pin with 3,3V to the AC pin next to REM on the X-Core controller.
  - Power-up the ESP with a floating power-supply.
  
  Features:
  - all values will be sent to a default IP/port via UDP on change, also own IP address
  - Humidity and temperatur are measured 
  - a Rain sensor provides current rain state
  - LCD display (128x64) will show main values

  Use as follows:
  - Read the IP address shown in the serial monitor during start-up 
    of the ESP or from display.
  - Send UDP packet with content defined in "UDP protocol" to the read IP address to change behavior or configure the controller 

  Options:
  - GPIO 5 can be used to control a pump that should start during activation
    of the zone.
  
  Copyright 2024, Hans-Joachim Zimmer
  All rights reserved.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Import required WIFI libraries
#include "ESP8266WiFi.h" //https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include "dht_sensor.h"  	// DHT11/22 abstration class
#include "oled.h"			// OLED client class
#include "wifi.h"			// WIFI client abstraction class
#include "mqtt.h"			// MQTT client abstraction class
#include "app_data.h"		// class for all application data
#include "hunter_ctrl.h" // HunterCore abstraction class

// ==================================================
// instanciates needed classes
DHT_SENSOR dhtSensor;
OLED oledDisplay;
WIFI_CTRL wifiCtrl;
MQTT mqttCtrl;
APP_DATA appData;
HUNTER_CTRL hunterCtrl;

void setup() {
  // Serial port for debugging purposes
  Serial.begin(74880);  
  delay(8000);  // to give time for serial port
  Serial.println(" ");
  
  // components init
  Serial.println("Starting init of all components ...");
  // initialize dependencies
  appData.initialize(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER_IP, MQTT_SERVER_PORT);  // params are defaults in case eeprom is empty
  dhtSensor.initialize(&oledDisplay, &appData);
  wifiCtrl.initialize(&oledDisplay, &appData);
  mqttCtrl.initialize(&oledDisplay, &appData);
  hunterCtrl.initialize(&oledDisplay, &appData);
  oledDisplay.initialize();

  // start wifi (ssid and pw from appData)
  if (false == wifiCtrl.connect()) {
    while(1) {}
  }

  // start mqtt connection (brokerIP and brokerPort from appData)
  if (false == mqttCtrl.initMqttServer()) {
    while(1) {}
  }
}

void loop() {
  // in case MQTT has updated WIFI or MQTT connection parameter, trigger new connections
  DATA_UPDATE  newData = appData.getNewDataFlag();
  if (newData == DATA_UPDATE::WIFI_UPDATED) {
    Serial.println("NewData flag for WIFI");
    appData.clearNewDataFlag(DATA_UPDATE::WIFI_UPDATED);
    wifiCtrl.connect();
    // wifi is connected, if we are here
    // set flag to reastablish MQTT as well before storing epprom in successful MQTT connection
    appData.setNewDataFlag(DATA_UPDATE::MQTT_UPDATED);  
  }
  if (newData == DATA_UPDATE::MQTT_UPDATED) {
    Serial.println("NewData flag for MQTT");
    if (mqttCtrl.initMqttServer()) {
      appData.clearNewDataFlag(DATA_UPDATE::MQTT_UPDATED);
      appData.storeEEProm();
    } else {
      // new MQTT parameter did not work, reset to connection params to EEProm data 
      appData.initialize(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER_IP, MQTT_SERVER_PORT);  // params are defaults in case eeprom is empty
    }
  }
  if (newData == DATA_UPDATE::DHT_UPDATED) {
    Serial.println("NewData flag for DHT");
    appData.clearNewDataFlag(DATA_UPDATE::DHT_UPDATED);
    mqttCtrl.publishDHTParams(appData.getDhtTempLevel(), appData.getDhtHumLevel(), appData.getDhtTempOffset());
  }       
  if (newData == DATA_UPDATE::HUNTER_ZONE_UPDATED) {
    Serial.println("NewData flag for HUNTER ZONE");
    appData.clearNewDataFlag(DATA_UPDATE::HUNTER_ZONE_UPDATED);
    hunterCtrl.startZone(appData.getHunterZone(), appData.getHunterTime());
  }
  if (newData == DATA_UPDATE::HUNTER_PROGRAM_UPDATED) {
    Serial.println("NewData flag for HUNTER PROGRAM");
    appData.clearNewDataFlag(DATA_UPDATE::HUNTER_PROGRAM_UPDATED);
    hunterCtrl.startProgram(appData.getHunterProgram());
  }
  
  // check for new data from DHT sensor
  if (dhtSensor.newDataAvailable()) {
    // new temperatur or humidity
    float h = dhtSensor.getHumidity();
    // Read temperature as Celsius
    float t = dhtSensor.getTemperature();
    // Publishes Temperature and Humidity values via MQTT
    if (false == mqttCtrl.publishDHT(t, h) ) {
      Serial.println("DHT publishing failed");
    } else {
      Serial.println("DHT publishing Done");
    };
  }

  wifiCtrl.loop();
  mqttCtrl.loop();
  
  oledDisplay.updateScreen();

  delay(500);  // to give time for serial port
}
