/*!
 *  @file oled.cpp
 *
 *  @mainpage  Class for OLED 128x64 display.
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction class for OLED 128x64 display.
 *  12 pixels in y are yellow, starting from 13 they are blue.
 *  it provides an abstraction for 5 lines of messages, since some text is too long 
 *  for one line, 2 pages are maintained and altered after PAGE_DURATION ms
 *       display.drawString(0, 0, "Zeile by y 0");      water zone x for y min
 *       display.drawString(0, 13, "Zeile by y 13");    SSID        own IP
 *       display.drawString(0, 25, "Zeile by y 25");    MQTT IP     MQTT Port
 *       display.drawString(0, 38, "Zeile by y 38");    hum & T
 *       display.drawString(0, 51, "Zeile by y 51");    current action 
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution.
 */

#include "OLED.h"

#define PAGE_DURATION 2000     // time to alter display pages in ms

/*!
 *  @brief  Setup OLED lines buffer and initialize display class.
 */
void OLED::initialize() {
  for (int i = 1; i < ARRAY_LEN; i++) {
    strncpy(lines[i], "", MAX_CHAR_IN_LINE - 1);
  }
  dataChanged = true;

  display.init();
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
}

/*!
 *  @brief  Update the OLED screen with the current buffer content.
 *
 *  needs to be called periodically as it alters the pages
 */
void OLED::updateScreen() {
  if (millis() - lastPageUpdate > PAGE_DURATION) {
    dataChanged = true;
    lineIndex++;
    if (lineIndex > MAX_PAGES - 1) {
      lineIndex = 0;
    }
    lastPageUpdate = millis();
  }
  if (dataChanged) {
    display.clear();
    display.drawString(0, 0, lines[0 + (lineIndex * MAX_LINES)]); 
    display.drawString(0, 13, lines[1 + (lineIndex * MAX_LINES)]);
    display.drawString(0, 25, lines[2 + (lineIndex * MAX_LINES)]);
    display.drawString(0, 38, lines[3 + (lineIndex * MAX_LINES)]);
    display.drawString(0, 51, lines[4 + (lineIndex * MAX_LINES)]);

    // Write the buffer to the display
    display.display();
    dataChanged = false;
  }  
}

/*!
 *  @brief  Update the OLED screen with WiFi information.
 *  @param  ip    The IP address.
 *  @param  ssid  The SSID of the WiFi network.
 */
void OLED::updateWifiInfo(const char* ip, const char* ssid) {
  strncpy(lines[1], String(String("SSID: ") + String(ssid)).c_str(), MAX_CHAR_IN_LINE - 1);
  strncpy(lines[6], String(String("Own IP: ") + String(ip)).c_str(), MAX_CHAR_IN_LINE - 1);
  dataChanged = true;
  updateScreen();
}

/*!
 *  @brief  Update the OLED screen with MQTT information.
 *  @param  ip         The IP address of the MQTT broker.
 *  @param  port       The port of the MQTT broker.
 *  @param  connected  Connection status to the MQTT broker.
 */
void OLED::updateMqttInfo(const char* ip, const int port, boolean connected) {
  String status = connected ? "" : "X - ";
  strncpy(lines[2], String(status + String("Broker: ") + String(ip)).c_str(), MAX_CHAR_IN_LINE - 1);
  strncpy(lines[7], String(status + String("MQTT port: ") + String(port)).c_str(), MAX_CHAR_IN_LINE - 1);
  dataChanged = true;
  updateScreen();
}

/*!
 *  @brief  Update the OLED screen with Hunter information.
 *  @param  zone     The zone number.
 *  @param  time     The time in minutes.
 *  @param  program  The program to be started, if <>0 zone and time is ignored
 */
void OLED::updateHunterInfo(const int zone, const int time, const int program) {
  String message = String(String("Zone: ") + String(zone));
  String onOffMsg = time ? String(" on for ") + String(time) + String(" min") : String(" off");
  message = message + String(onOffMsg);
  if (program != 0) {
    message = "Prog " + String(program) + " started";
  }
  strncpy(lines[0], message.c_str(), MAX_CHAR_IN_LINE - 1);
  strncpy(lines[5], lines[0], MAX_CHAR_IN_LINE);
  dataChanged = true;
  updateScreen();
}

/*!
 *  @brief  Update the OLED screen with DHT sensor information.
 *  @param  t  The temperature.
 *  @param  h  The humidity.
 */
void OLED::updateDHT(const float t, const float h) {
  // Round temperature to 1 decimal place
  char cTemp[6];
  dtostrf(t, 4, 1, cTemp);
  int iHumidity = (int)h;
  strncpy(lines[3], String(String("Temp: ") + String(cTemp) + String(" °C  Hum: ") + String(iHumidity) + String(" %")).c_str(), MAX_CHAR_IN_LINE - 1);
  strncpy(lines[8], lines[3], MAX_CHAR_IN_LINE);
  dataChanged = true;
  updateScreen();
}

/*!
 *  @brief  Update the OLED screen with a custom action message.
 *  @param  message  The action message.
 */
void OLED::updateAction(const char* message) {
  strncpy(lines[4], message, MAX_CHAR_IN_LINE - 1);
  strncpy(lines[9], lines[4], MAX_CHAR_IN_LINE);  
  dataChanged = true;
  updateScreen();
}