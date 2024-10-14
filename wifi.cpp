/*!
 *  @file wifi.cpp
 *
 *  @mainpage  class to control a wifi connection in a HUNTER XCORE watering application.
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction class for a WIFI connection
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution
 */
 
 #include "WIFI.h"

/*!
 * @brief initalizes the WIFI_CTRL class 
 *
 * @param oled      pointer to OLED class to display status information
 * @param appData   pointer to APP_DATA class, storing shared data used in the application
 */
void WIFI_CTRL::initialize(OLED* oled, APP_DATA* appData) {
    this->oled = oled;
    this->appData = appData;
}

/*!
 * @brief Connect to a WIFI AP
 *
 * takes parameters from appData class and establish a WIFI connection
 * will not return until connection is established
 *
 * @return success on connection or failure if not correct initialized
 */
boolean WIFI_CTRL::connect() {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }
  char msg[40] = "Trying WIFI ";

  if (!appData) {
    Serial.println("no valid appData or oled pointer");
    return false;
  }
  strcat(msg, appData->getWifiSsid());
  strcat(msg, " ...");
  if (oled) {oled->updateAction(msg);}

  // Set in station mode and connect
  WiFi.begin(appData->getWifiSsid(), appData->getWifiPw());
  Serial.println("");
  Serial.print(msg );
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (oled) {oled->updateWifiInfo(getOwnIp().c_str(), appData->getWifiSsid());}

  Serial.print("Connected, IP address: ");
  Serial.println(getOwnIp());
  appData->setWifiIp(getOwnIp().c_str());

  return true;
}

/*!
 * @brief Get WIFI IP address 
 *
 * @return DHCP assigned WIFI IP address
 */
String WIFI_CTRL::getOwnIp() {
  return(WiFi.localIP().toString());
}

/*!
 * @brief reestablish a broken WIFI connection
 *
 * shall be periodically called
 *
 */
void WIFI_CTRL::loop() {
  if (! WiFi.isConnected()) {
    Serial.println("WIFI connection lost, reconnecting");
    this->connect();
  }
}
