/*!
 *  @file app_data.h
 *
 *  This is an abstraction class maintaining all common data for the application.
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  MIT license, all text above must be included in any redistribution.
 */

#include "IPAddress.h"
#ifndef APP_DATA_H
#define APP_DATA_H

#include <Arduino.h>

#define EEPROM_DATA_VALID   0xAA
#define EEPROM_DATA_TOSTORE 0x55

/*!
 *  @brief  Enum class for data update flags.
 */
enum class DATA_UPDATE : char {
    DATA_UNSET              = 0x00,
    WIFI_UPDATED            = 0x01, // 00000001
    MQTT_UPDATED            = 0x02, // 00000010
    DHT_UPDATED             = 0x04, // 00000100
    HUNTER_ZONE_UPDATED     = 0x08, // 00001000
    HUNTER_PROGRAM_UPDATED  = 0x10, // 00010000
};

/*!
 *  @brief  Struct for storing EEPROM data.
 */
typedef struct {
  int     dataValid;
  char    wifiSSID[32];
  char    wifiPassword[32];
  int     mqttBrokerPort;
  byte    mqttBrokerIp[4];
  float   dhtTempLevel;   
  int     dhtHumLevel;    
  int     dhtTemperaturOffset;   
} EEPROMStruct;

/*!
 *  @brief  Class for managing application data.
 */
class APP_DATA {
public:
  /*!
   *  @brief  Constructor for APP_DATA.
   */
  APP_DATA() {};

  /*!
   *  @brief  Initializes the APP_DATA class.
   *  @param  ssid        The WiFi SSID.
   *  @param  pw          The WiFi password.
   *  @param  brokerIP    The MQTT broker IP address.
   *  @param  brokerPort  The MQTT broker port.
   */
  void initialize(const char* ssid, const char* pw, const char* brokerIP, const int brokerPort);

  /*!
   *  @brief  Stores the EEPROM data.
   */
  void storeEEProm() {
    this->storeEEpromData();
  };

  // Getter and setter methods

  /*!
   *  @brief  Gets the WiFi SSID.
   *  @return The WiFi SSID.
   */
  const char* getWifiSsid() { return sData.wifiSSID; }

  /*!
   *  @brief  Sets the WiFi SSID.
   *  @param  ssid  The WiFi SSID.
   */
  void setWifiSsid(const char* ssid) { 
    strncpy(sData.wifiSSID, ssid, sizeof(sData.wifiSSID)); 
    this->sData.dataValid = EEPROM_DATA_TOSTORE;
  }

  /*!
   *  @brief  Gets the WiFi password.
   *  @return The WiFi password.
   */
  const char* getWifiPw() { return sData.wifiPassword; }

  /*!
   *  @brief  Sets the WiFi password.
   *  @param  pw  The WiFi password.
   */
  void setWifiPw(const char* pw) { 
    strncpy(sData.wifiPassword, pw, sizeof(sData.wifiPassword)); 
    this->sData.dataValid = EEPROM_DATA_TOSTORE;
  }

  /*!
   *  @brief  Gets the WiFi IP address.
   *  @return The WiFi IP address.
   */
  const IPAddress getWifiIp() { return wifiIp; }

  /*!
   *  @brief  Sets the WiFi IP address.
   *  @param  wifiIp  The WiFi IP address.
   */
  void setWifiIp(const char* wifiIp) { 
    int ipPart1, ipPart2, ipPart3, ipPart4;
    if (sscanf(wifiIp, "%d.%d.%d.%d", &ipPart1, &ipPart2, &ipPart3, &ipPart4) != 4) {
      Serial.print("invalid WIFI IP: ");
      Serial.println(wifiIp);
    }
    this->wifiIp = IPAddress((byte)ipPart1, (byte)ipPart2, (byte)ipPart3, (byte)ipPart4);
  }

  /*!
   *  @brief  Gets the MQTT broker IP address.
   *  @return The MQTT broker IP address.
   */
  const IPAddress getMqttIp() {
    brokerIp = IPAddress(sData.mqttBrokerIp[0], sData.mqttBrokerIp[1], sData.mqttBrokerIp[2], sData.mqttBrokerIp[3]);
    return brokerIp;
  }

  /*!
   *  @brief  Sets the MQTT broker IP address.
   *  @param  brokerIp  The MQTT broker IP address.
   */
  void setMqttIp(const char* brokerIp) { 
    int ipPart1, ipPart2, ipPart3, ipPart4;
    if (sscanf(brokerIp, "%d.%d.%d.%d", &ipPart1, &ipPart2, &ipPart3, &ipPart4) != 4) {
      Serial.print("invalid broker IP: ");
      Serial.println(brokerIp);
    }
    this->sData.mqttBrokerIp[0] = (byte)ipPart1; 
    this->sData.mqttBrokerIp[1] = (byte)ipPart2; 
    this->sData.mqttBrokerIp[2] = (byte)ipPart3; 
    this->sData.mqttBrokerIp[3] = (byte)ipPart4;
    this->sData.dataValid = EEPROM_DATA_TOSTORE;
  }

  /*!
   *  @brief  Gets the MQTT broker port.
   *  @return The MQTT broker port.
   */
  const int getMqttPort() { return sData.mqttBrokerPort; }

  /*!
   *  @brief  Sets the MQTT broker port.
   *  @param  port  The MQTT broker port.
   */
  void setMqttPort(const int port) { 
    sData.mqttBrokerPort = port; 
    this->sData.dataValid = EEPROM_DATA_TOSTORE;
  }  

  /*!
   *  @brief  Gets the DHT temperature level.
   *  @return The DHT temperature level.
   */
  float getDhtTempLevel() const { return sData.dhtTempLevel; }

  /*!
   *  @brief  Sets the DHT temperature level.
   *  @param  level  The DHT temperature level, level -3.0..3.0
   */
  void setDhtTempLevel(float level) { 
    if (abs(level) <= 3) {
      sData.dhtTempLevel = level; 
      this->sData.dataValid = EEPROM_DATA_TOSTORE;
    }
  }

  /*!
   *  @brief  Gets the DHT humidity level.
   *  @return The DHT humidity level.
   */
  int getDhtHumLevel() const { return sData.dhtHumLevel; }

  /*!
   *  @brief  Sets the DHT humidity level.
   *  @param  level  The DHT humidity level, level -10..10
   */
  void setDhtHumLevel(int level) { 
    if (abs(level) <= 10) {
      sData.dhtHumLevel = level; 
      this->sData.dataValid = EEPROM_DATA_TOSTORE;
    }
  }

  /*!
   *  @brief  Gets the DHT temperature offset.
   *  @return The DHT temperature offset.
   */
  int getDhtTempOffset() const { return sData.dhtTemperaturOffset; }

  /*!
   *  @brief  Sets the DHT temperature offset.
   *  @param  offset  The DHT temperature offset.
   */
  void setDhtTempOffset(int offset) { 
    sData.dhtTemperaturOffset = offset;
    this->sData.dataValid = EEPROM_DATA_TOSTORE; 
  }

  /*!
   *  @brief  Gets the Hunter zone.
   *  @return The Hunter zone.
   */
  const int getHunterZone() const { return this->hunterZone; }

  /*!
   *  @brief  Sets the Hunter zone.
   *  @param  hunterZone  The Hunter zone.
   */
  void setHunterZone(const int hunterZone) { this->hunterZone = hunterZone; }

  /*!
   *  @brief  Gets the Hunter time.
   *  @return The Hunter time.
   */
  const int getHunterTime() const { return this->hunterTime; }

  /*!
   *  @brief  Sets the Hunter time.
   *  @param  hunterTime  The Hunter time.
   */
  void setHunterTime(const int hunterTime) { this->hunterTime = hunterTime; }

  /*!
   *  @brief  Gets the Hunter program.
   *  @return The Hunter program.
   */
  const int getHunterProgram() const { return this->hunterProgram; }

  /*!
   *  @brief  Sets the Hunter program.
   *  @param  hunterProgram  The Hunter program.
   */
  void setHunterProgram(const int hunterProgram) { this->hunterProgram = hunterProgram; }
  void setNewDataFlag(DATA_UPDATE dataUpdate);
  DATA_UPDATE getNewDataFlag();
  void clearNewDataFlag(DATA_UPDATE dataUpdate);


private:
  EEPROMStruct  sData;
  IPAddress     brokerIp;
  IPAddress     wifiIp;
  int           hunterZone;
  int           hunterTime;
  int           hunterProgram;

  boolean       readEEpromData();
  boolean       storeEEpromData();

  DATA_UPDATE   dataUpdate;
};

#endif // APP_DATA_H
