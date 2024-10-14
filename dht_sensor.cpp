/*!
 *  @file DHT_sensor.cpp
 *
 *  @mainpage DHT series of low cost temperature/humidity sensors.
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction for DHT series of low cost temperature/humidity sensors.
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution.
 */
 
#include "DHT_SENSOR.h"

/* New data is passed via flag and values in AppData.
   MQTT or the main function publishes based on the flag. */
   
/*!
 *  @brief  Initializes the DHT_SENSOR class.
 *  @param  oled     Pointer to the OLED display object.
 *  @param  appData  Pointer to the application data object.
 */
void DHT_SENSOR::initialize(OLED* oled, APP_DATA* appData) {
  this->oled = oled;
  this->appData = appData;
    
  newData = false;
  temp_lastRead = temp_actual = 0;
  humidity_lastRead = humidity_actual = 0; 
  timeSinceLastRead = 0;   

  dht.begin(DEFAULT_DHT_PULLUP_TIME);  

  // Setup data from app_data
  if (appData) {
    appData->setDhtTempLevel(0.1);  // Indicate changes of 0.1°C in boolean newData
    appData->setDhtHumLevel(1);
  }
}

/*!
 *  @brief  Gets the actual temperature.
 *  @return The current temperature as a float.
 */
const float DHT_SENSOR::getTemperature() {
  temp_lastRead = temp_actual;
  newData = false;

  return temp_lastRead;
}

/*!
 *  @brief  Gets the actual humidity.
 *  @return The current humidity as a float.
 */
const float DHT_SENSOR::getHumidity() {
  humidity_lastRead = humidity_actual;
  newData = false;

  return humidity_lastRead;
}

/*!
 *  @brief  Checks if new data is available.
 *          Reads value from sensor if less than MIN_INTERVAL.
 *          Checks if change is greater than level for temp or humidity.
 *  @return True if new data is available, false otherwise.
 */
const bool DHT_SENSOR::newDataAvailable() {
  boolean newData = false;

  if (millis() - timeSinceLastRead > MIN_INTERVAL) {
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit
    float f = dht.readTemperature(true);
    timeSinceLastRead = millis();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)){      
      return false;  // Read failed somehow
    }
    
    temp_actual = t;
    if (appData) {
      temp_actual = t + appData->getDhtTempOffset();
    }
    humidity_actual = h;

    if (appData) {
      float diff = temp_actual - temp_lastRead;
      if (abs(diff) >= appData->getDhtTempLevel()) {
        newData = true;
      }

      diff = humidity_actual - humidity_lastRead;
      if (abs(diff) >= appData->getDhtHumLevel()) {
        newData = true;
      }
    }
    // Set display values
    if (oled) {
      oled->updateDHT(temp_actual, humidity_actual);   
    }

  } 
  return newData;
}
