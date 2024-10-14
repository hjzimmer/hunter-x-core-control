/*!
 *  @file app_data.cpp
 *
 *  @mainpage Implementation for a class maintained all common data for the application
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction for data, it stores all data in EEPROM and takes care of the data
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution.
 */
 
#include "APP_DATA.h"
#include <ESP_EEPROM.h>

/*!
 *  @brief  Debug function to print EEPROM data to Serial.
 *  @param  eepromData  Pointer to the EEPROM data structure.
 */
void debugEEprom(const EEPROMStruct* eepromData) {
  char buf[30];

  Serial.print("SSID:");
  Serial.println(eepromData->wifiSSID);
  Serial.print("PW:");
  Serial.println(eepromData->wifiPassword);
  Serial.print("MQTT port:");
  Serial.println(eepromData->mqttBrokerPort);
  Serial.print("MQTT ip:");
  sprintf(buf, "%d.%d.%d.%d", eepromData->mqttBrokerIp[0],eepromData->mqttBrokerIp[1],eepromData->mqttBrokerIp[2],eepromData->mqttBrokerIp[3] );
  Serial.println(buf);
  Serial.print("DHT Temp Level:");
  Serial.println(eepromData->dhtTempLevel);
  Serial.print("DHT Hum Level:");
  Serial.println(eepromData->dhtHumLevel);
  Serial.print("DHT Temp Offset:");
  Serial.println(eepromData->dhtTemperaturOffset);
}

/*!
 *  @brief  Initializes an instance of the APP_DATA class using a specified sector of flash memory.
 *
 *  It reads the EEPROM content into the working data structure or 
 *  initializes the data structure with defaults and the function parameters
 *  in case the EEPROM does not contain valid data.
 *
 *  @param  ssid        The WiFi SSID to use if not stored in EEPROM.
 *  @param  pw          The WiFi password to use if not stored in EEPROM.
 *  @param  brokerIp    The MQTT broker IP address (format 10.10.2.1) to use if not stored in EEPROM.
 *  @param  brokerPort  The MQTT broker port to use if not stored in EEPROM.
 *  @return The percentage used (0-100) or -1 if the flash does not hold any copies of the data.
 */
void APP_DATA::initialize(const char* ssid, const char* pw, const char* brokerIp, const int brokerPort) {
  this->brokerIp = IPAddress(0, 0, 0, 0);
  this->wifiIp = IPAddress(0, 0, 0, 0);
  this->dataUpdate = DATA_UPDATE::DATA_UNSET;

  boolean valid = readEEpromData();
  if (!valid) {
    Serial.println("init EEProm with defaults, since eeprom data is not valid");
    strncpy(this->sData.wifiSSID, ssid, sizeof(this->sData.wifiSSID) - 1);
    strncpy(this->sData.wifiPassword, pw, sizeof(this->sData.wifiPassword) - 1);
    int ipPart1, ipPart2, ipPart3, ipPart4;
    if (sscanf(brokerIp, "%d.%d.%d.%d", &ipPart1, &ipPart2, &ipPart3, &ipPart4) != 4) {
      Serial.print("invalid broker IP: ");
      Serial.println(brokerIp);
    }
    this->sData.mqttBrokerIp[0] = (byte)ipPart1; this->sData.mqttBrokerIp[1] = (byte)ipPart2; 
    this->sData.mqttBrokerIp[2] = (byte)ipPart3; this->sData.mqttBrokerIp[3] = (byte)ipPart4;
    this->brokerIp = IPAddress((byte)ipPart1, (byte)ipPart2, (byte)ipPart3, (byte)ipPart4);
    this->sData.mqttBrokerPort = brokerPort;
    this->sData.dhtTempLevel = 0.2;   // init values for DHT 
    this->sData.dhtHumLevel = 5;    
    this->sData.dhtTemperaturOffset = 0;  
    this->sData.dataValid = EEPROM_DATA_TOSTORE;
  }
  debugEEprom(&sData);
}

/*!
 *  @brief  Sets the new data flag.
 *  @param  dataUpdate  The data update flag to set.
 */
void APP_DATA::setNewDataFlag(DATA_UPDATE dataUpdate) {
  this->dataUpdate = (DATA_UPDATE)((char)this->dataUpdate | (char)dataUpdate);
}

/*!
 *  @brief  Gets the new data flag.
 *  @return The current data update flag.
 */
DATA_UPDATE APP_DATA::getNewDataFlag() {
  return this->dataUpdate;
}

/*!
 *  @brief  Clears the specified new data flag.
 *  @param  dataUpdate  The data update flag to clear.
 */
void APP_DATA::clearNewDataFlag(DATA_UPDATE dataUpdate) {
  this->dataUpdate = (DATA_UPDATE)((byte)this->dataUpdate & ~(byte)dataUpdate);
}

// ======= private functions ===================================================

/*!
 *  @brief  Reads the EEPROM data.
 *  @return True if data read and valid, otherwise false.
 */
boolean APP_DATA::readEEpromData() {
  boolean ret = false;
  // Read EEPROM and use it if present
  EEPROM.begin(sizeof(EEPROMStruct));
  // Check if the EEPROM contains valid data from another run
  // If so, overwrite the 'default' values set up in our struct
  if(EEPROM.percentUsed()>=0) {
    Serial.println("READ: EEPROM has data from a previous run.");
    Serial.print(EEPROM.percentUsed());
    Serial.println("% of ESP flash space currently used");
    EEPROM.get(0, (EEPROMStruct&)this->sData); 
    if ((this->sData.dataValid == EEPROM_DATA_VALID) || (this->sData.dataValid == EEPROM_DATA_TOSTORE)) {
      Serial.println("read eeprom data is valid");
      ret = true;  
    } 
  }
  if (ret == false) {
    // clear EEProm data struct
    Serial.println("eeprom data is invalid, clearing");
    char* pData = (char*)&this->sData;
    for (int i=0; i<sizeof(EEPROMStruct); i++) {
      *pData = 0;
      pData++;
    }
  }
 
  EEPROM.end();
  return ret;
}

/*!
 *  @brief  Reads the EEPROM data.
 *  @return True if data stored and valid, otherwise false, return true as well, if EEPROM data was equal to buffer to be stored
 */
boolean APP_DATA::storeEEpromData() {
  boolean ok = false;
  if (this->sData.dataValid == EEPROM_DATA_TOSTORE) {
    // Read EEProm and use it if present
    EEPROM.begin(sizeof(EEPROMStruct));
    // Check if the EEPROM contains valid data from another run
    // If so, overwrite the 'default' values set up in our struct
    // if(EEPROM.percentUsed()>=0) {
    //   //EEPROM.get(0, eepromData);
    //   Serial.println("WRITE: EEPROM has data from a previous run.");
    //   Serial.print(EEPROM.percentUsed());
    //   Serial.println("% of ESP flash space currently used");
    // } else {
    //   Serial.println("WRITE: EEPROM size changed - EEPROM data zeroed - commit() to make permanent");    
    // }
    this->sData.dataValid = EEPROM_DATA_VALID;
    EEPROM.put(0, this->sData);
    // write the data to EEPROM
    ok = EEPROM.commit();
    Serial.println((ok) ? "EEProm storing OK (in case it changed)" : "EEProm storing failed");
    EEPROM.end();
  }
  return ok;;
}
