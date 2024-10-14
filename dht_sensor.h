/*!
 *  @file DHT_sensor.h
 *
 *  This is an abstraction class for DHT series of low cost temperature/humidity sensors.
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include "Arduino.h"
#include "app_data.h"
#include "oled.h"
#include <DHT.h>

#define DHTPIN  D2      /**< what pin we're connected to */
#define DHTTYPE DHT21   /**< type of DHT sensor to use, DHT 11 */
#define DEFAULT_DHT_PULLUP_TIME  55   /**< Optionally pass pull-up time (in microseconds) before DHT \
                                           reading starts. Default is 55 (see function declaration in DHT.h) */

#define MIN_INTERVAL 2000 /**< min interval value in ms between reads of the sensor */

/*!
 *  @brief  Class that stores state and functions for DHT
 */
class DHT_SENSOR {
public:
// constructor
  DHT_SENSOR(): oled(nullptr), appData(nullptr), dht(DHTPIN, DHTTYPE) {};
  // public methods
  void initialize(OLED* oled, APP_DATA* appData);
  const bool newDataAvailable();
  const float getTemperature();
  const float getHumidity();
  void updateDisplay();

private:
  float     temp_actual, humidity_actual;    // R
  float     temp_lastRead, humidity_lastRead;    // R
  bool      newData;                   // R, indicates new data
  float     newDataLevelTemp;          // R/W, which change in value triggers new data
  float     newDataLevelHum;           // R/W, which change in value triggers new data
  long      timeSinceLastRead;
  uint8_t   _pin, _type;
  DHT       dht;
  OLED*     oled;
  APP_DATA* appData;
};

#endif // DHT_SENSOR_H
