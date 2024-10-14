/*!
 *  @file mqtt.cpp
 *
 *  @mainpage  class to control a MQTT connection in a HUNTER XCORE watering application.
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction class for a MQTT connection
 *  It sets up a connection to an MQTT broker, maintains it and reconencts of broker parameters have changed
 *  It offers methods to publish differnt kind of application data as well a callback method 
 *  to handle subsription messages
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution
 */
 
 #include "MQTT.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT broker credentials (set to NULL if not required)
const char* MQTT_username = "REPLACE_WITH_MQTT_USERNAME"; 
const char* MQTT_password = "REPLACE_WITH_MQTT_PASSWORD"; 

// Initializes the hunterClient. You should change the hunterClient name if you have multiple ESPs running in your home automation system
WiFiClient hunterClient;
PubSubClient mqttClient(hunterClient);

APP_DATA* pAppDataClass;    // pointer to APP_DATA class needed in subscription callback

/*!
 * @brief initalizes the MQTT class 
 *
 * @param oled      pointer to OLED class to display status information
 * @param appData   pointer to APP_DATA class, storing shared data used in the application
 */
 void MQTT::initialize(OLED* oled, APP_DATA* appData) {
    this->oled = oled;
    this->appData = appData;
    pAppDataClass = appData;
}

/*!
 * @brief callback for receive of subscibed topics
 * 
 * This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
 * Change the function below to add logic to your program, so when a device publishes a message to a topic that 
 * your ESP8266 is subscribed you can actually do something
 *
 * @param topic  received topic which triggered this callback
 * @param message   received message
 * @param length  lebgth of received message
 */
void mqttSubscriptionCallback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  JsonDocument doc;
  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="hunter/config"){
    DeserializationError error = deserializeJson(doc, messageTemp);
    if (error) {
      Serial.print("deserializeJson() returned ");
      Serial.println(error.c_str());
      return;
    }
    // Extract the values 
    // "wifi" array with ssid & pw
    // "mqtt" array with ip & port
    // "dht" array with t_offset(-3..3) & t_hold(-3.0..3.0) & h_hold(-10..10)
    // "water" array with zone(int 1..8) & time(int 0..240 ) or  program (1..)
    if (doc.containsKey("wifi") ) {
      // handle wifi settings
      Serial.println("wifi detected");
      const char* ssid = doc["wifi"]["ssid"]; // "10.11.12.13"
      const char* pw = doc["wifi"]["pw"]; // "10.11.12.13"
      if (ssid != NULL) {
        // update data and trigger new connect via flag
        if (pAppDataClass) {
          pAppDataClass->setWifiSsid(ssid);
          pAppDataClass->setWifiPw(pw);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::WIFI_UPDATED);
          // storing eeprom from main loop, since connection HAS changed and needs to work first before storing
        }
      }
    }
    if (doc.containsKey("mqtt") ) {
      // handle mqtt settings
      Serial.println("mqtt detected");
      const char* mqtt_ip = doc["mqtt"]["ip"]; // "10.11.12.13"
      int mqtt_port = doc["mqtt"]["port"]; // 815
      if ((mqtt_ip != NULL) && (mqtt_port != 0)) {
        if (pAppDataClass) {
          pAppDataClass->setMqttIp(mqtt_ip);
          pAppDataClass->setMqttPort(mqtt_port);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::MQTT_UPDATED);  
          // storing eeprom from main loop, since connection HAS changed and needs to work first before storing
        }
      }
    }
    if (doc.containsKey("dht")) {
      // handle dht settings
      Serial.println("dht detected");
      if (doc["dht"].containsKey("t_offset")) {
        if (pAppDataClass) {
          int dht_t_offset = doc["dht"]["t_offset"];
          pAppDataClass->setDhtTempOffset(dht_t_offset);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::DHT_UPDATED);
        }
      }
      if (doc["dht"].containsKey("t_hold")) {
        if (pAppDataClass) {
          float dht_t_level = doc["dht"]["t_hold"];
          pAppDataClass->setDhtTempLevel(dht_t_level);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::DHT_UPDATED);
        }
      }
      if (doc["dht"].containsKey("h_hold")) {
        if (pAppDataClass) {
          int dht_h_level = doc["dht"]["h_hold"];
          pAppDataClass->setDhtHumLevel(dht_h_level);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::DHT_UPDATED);
        }
      }
      pAppDataClass->storeEEProm(); // store eeprom data right now, since connection has not changed, only params
    }
    if (doc.containsKey("water")) {
      // handle hunter watering settings
      Serial.println("water detected");
      if (doc["water"].containsKey("zone") && doc["water"].containsKey("time")) {
        int zone = doc["water"]["zone"];
        int time = doc["water"]["time"];
        if (pAppDataClass) {
          pAppDataClass->setHunterZone(zone);
          pAppDataClass->setHunterTime(time);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::HUNTER_ZONE_UPDATED);
        }
      } else if (doc["water"].containsKey("program")) {
        int program = doc["water"]["program"];
        if (pAppDataClass) {
          pAppDataClass->setHunterProgram(program);
          pAppDataClass->setNewDataFlag(DATA_UPDATE::HUNTER_PROGRAM_UPDATED);
        }
      }
    }      
  }
}

/*!
 * @brief init the MQTT client, connects it and register the MQTT subsciption callback
 *
 * @return success or failure of connection to broker
 */
 boolean MQTT::initMqttServer() {
  char msg[40] = "Trying MQTT ";

  if (!appData) {
    Serial.println("no valid appData pointer");
    return false;
  }
  strcat(msg, appData->getMqttIp().toString().c_str());
  strcat(msg, " ...");
  if (oled) {oled->updateAction(msg);}

  mqttClient.disconnect(); // disconnect potential previous connection
  mqttClient.setServer(appData->getMqttIp(), appData->getMqttPort());
  mqttClient.setCallback(mqttSubscriptionCallback);

  if (oled) { oled->updateMqttInfo(appData->getMqttIp().toString().c_str(), appData->getMqttPort(), false);}

  Serial.println("initMqttServer -> reconnect");
  boolean result = this->reconnect();
  
  return result;
}

/*!
 * @brief loop called periodically to maintain MQTT connection 
 *
 * inits the MQTT Client if not connected and reconnect if connection is broken
 */
void MQTT::loop () {
  if (!mqttClient.connected()) {
    initMqttServer();
  }
  if(!mqttClient.loop()) {
    mqttClient.disconnect();
    Serial.println("mqtt loop failed, reconnect ..");
    reconnect();
  } 
}

/*!
 * @brief Publishes the parameter used to adjust the DHT sensor to broker 
 *
 * @param temp_level  level of temperatur change triggering a value update to broker
 * @param hum_level   level of humidity change triggering a value update to broker
 * @param temp_offset  the offset to the measured temperatur
 *
 * @return success or failure 
 */
boolean MQTT::publishDHTParams(const float temp_level, const int hum_level, const int temp_offset) {
  JsonDocument doc;
  doc["dht"]["temp_offset"] = temp_offset;
  doc["dht"]["temp_level"] = temp_level;
  doc["dht"]["hum_level"] = hum_level;

  char output[256];
  serializeJson(doc, output);

  Serial.println("Publishing DHT parameter");

  return _publish("value", output);
}

/*!
 * @brief Publishes the temperature and humidity values from DHT sensor 
 *
 * @param temp temperatur
 * @param humidity 
 *
 * @return success or failure 
 */
boolean MQTT::publishDHT(const float temp, const float humidity) {
  JsonDocument doc;
  doc["dht"]["temp"] = temp;
  doc["dht"]["humidity"] = humidity;

  char output[256];
  serializeJson(doc, output);

  Serial.println("Publishing DHT values");
  // show action on OLED
  if (oled) {oled->updateAction("Publishing DHT values");}     

  return _publish("value", output);
}

/*!
 * @brief Publishes the working MQTT parameter to broker 
 *
 * @param port MQTT broker port
 * @param ip   MQTT broker IP address
 *
 * @return success or failure 
 */
boolean MQTT::publishMQTT(const char* ip, const int port) {
  JsonDocument doc;
  doc["broker"]["ip"] = ip;
  doc["broker"]["port"] = port;

  char output[256];
  serializeJson(doc, output);

  Serial.println("Publishing MQTT settings");
  // show action on OLED
  if (oled) {oled->updateAction("Publishing MQTT values");}     

  return _publish("value", output);
}

/*!
 * @brief Publishes the working WIFI parameter to broker 
 *
 * @param ssid WIFI SSID
 * @param ip   WIFI DHCP IP address
 *
 * @return success or failure 
 */
boolean MQTT::publishWIFI(const char* ssid, const char* ip) {
  JsonDocument doc;
  doc["wifi"]["ssid"] = ssid;
  doc["wifi"]["ip"] = ip;

  char output[256];
  serializeJson(doc, output);

  Serial.println("Publishing WIFI settings");
  // show action on OLED
  if (oled) {oled->updateAction("Publishing WIFI values");}     

  return _publish("value", output);
}

// ================================ Private functions ==================================

/*!
 * @brief This functions reconnects your ESP8266 to your MQTT broker and it subscribes to topics
 * Change the function below if you want to subscribe to more topics with your ESP8266 
 * 5 connections tries are made, if successfull, working WIFI and MQTT parameters are stored in EEprom
 * and published to broker
 *
 * @return success or failure 
 */
boolean MQTT::reconnect() {
  if (!appData) {
    Serial.println("no valid appData pointer");
    return false;
  }

  char mqttServerIp[30];
  strcpy(mqttServerIp, appData->getMqttIp().toString().c_str());
  
  // Loop until we're reconnected
  int loopCnt = 0;

  while (!mqttClient.connected()) {
    if (loopCnt++ > 4) {
      return false;
    }
    Serial.print("Attempting MQTT connection...");
    if (oled) {oled->updateAction("Connecting to MQTT ..."); }

    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (mqttClient.connect("ESP_Hunter", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      mqttClient.subscribe((PRE_MQTT + String("/") + String("config")).c_str());
      if (oled) {oled->updateMqttInfo(mqttServerIp, appData->getMqttPort(), true);}
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  if (oled) {oled->updateAction("MQTT connected");} 

  // WIFI and MQTT is connected, can be stored to EEProm
  appData->storeEEProm();

  // Publishes WIFI IP address since MQTT is now connected
  publishWIFI( appData->getWifiSsid(), appData->getWifiIp().toString().c_str() );
  // Publish MQTT setting
  publishMQTT( appData->getMqttIp().toString().c_str(), appData->getMqttPort());

  return true;
}

/*!
 * @brief Publishes a payload to a MQTT broker 
 *
 * @param topic MQTT topic, will be prepended with define PRE_MQTT/topic
 * @param payload char pointer to mqtt payload message
 *
 * @return success or failure 
 */
 boolean MQTT::_publish(const char* topic, const char* payload) {
  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected, publish canceled");
    return false;
  }
  return mqttClient.publish(addOrigin(topic).c_str(), payload);
}  

/*!
 * @brief Publishes a payload to a MQTT broker 
 *
 * @param topic MQTT topic, will be prepended with define PRE_MQTT/topic
 * @param payload pointer to data bytes
 * @param length length of payload
 *
 * @return success or failure 
 */
boolean MQTT::_publish(const char* topic, const uint8_t* payload, unsigned int length) {
  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected, publish canceled");
    return false;
  }
  return mqttClient.publish(addOrigin(topic).c_str(), payload, length);
} 

const String MQTT::addOrigin(const char* topic) {
  return( PRE_MQTT + String("/") + String(topic) );
} 
