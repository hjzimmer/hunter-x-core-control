# Application to control a HUNTER XCORE via an ESP8266 NodeMCU board

This Arduino project is designed to control an Hunter X-Core irrigation controller conntected to an ESP8266-based device, which can be configured and controlled via MQTT.
The project includes functionality for reading sensor data, displaying information on an OLED screen, and mainly it is to control a Hunter X-Core irrigation controller.
Provided functions are:
- start watering with an Hunter X-Core irrigation controller
- receive commands via MQTT subscription from a broker
- maintain MQTT infra structure and WIFI
- optional is
  - a OLED display to visualize alls actions and state
  - a DHT sensor which can be used to measure temp and humidity published via MQTT

## Key Features:
1. WiFi Connectivity: The project uses the ESP8266WiFi library to connect the ESP8266 to a WiFi network.
2. MQTT Communication: The device can publish and subscribe to MQTT topics to change its behavior or configuration.
3. Sensor Integration: The project includes a DHT sensor for reading temperature and humidity data.
4. OLED Display: The project supports displaying information on an OLED screen using the SSD1306Wire library.
5. Mainly: Issue command to Hunter X-Core irrigation controller to start/stop watering of sections.

The project uses several libraries to manage WiFi connectivity, sensor data, and display output.

## Control flow
In setup() all classes are instanciated and initialized. Only appData and oledDisplay instances are spread around.
Any other thing is maintained from main loop()
  - class appData - holds all data and controls persistant storage of modifiable parameters
  - class dhtSensor - alls stuff related to a DHT sensor
  - class wifiCtrl - set up and controls the WIFI connection
  - class mqttCtrl - set up and controls the MQTT connection, handles publishing and subscription messages
  - class hunterCtrl - class te control the sprinkler irrigation controller
  - class oledDisplay - all things to control the OLED display

In case no OLED or DHT is used, the instanciation has to be removed and pointers have to be set to "nullptr".

The next step is to connect WIFI and MQTT, which will lead to reboots and new trials if unsuccessful.

In loop() it is checked for updated data via the newData flag, typically set by a received MQTT subscription.
It can be changed WIFI, MQTT or DHT parameters. Or a command to start watering a zone for some time or to start a watering program.

Afterwards it is checked, if new value from DHT sensor are present and display and published.

Finally it calls the loop methods for WIFI and MQTT, which observes the connection and handles MQTT messages.

## MQTT protocol
The device publishes all values in a topic named "value" as json string. The topic can be configured with a preamble.
A typical example is
- topic:     hunter_control/value
- message:
```
  {
    "wifi": {
      "ssid": "<WIFI SSID>",
      "ip": "<own WIFI IP>"
    }
  }
```

The complete value topic is defined as, but will never be published all at once:
```
  {
    "wifi": {
      "ssid": "<WIFI SSID>",
      "ip": "<own WIFI IP>"
    },
    "broker": {
      "port": "<MQTT broker port>",
      "ip": "<MQTT broker IP>"
    },
    "dht": {
      "temp": <float value>,
      "humidity": <float value>,
      "temp_offset": <int value>,
      "temp_level": <float value>,
      "hum_level": <int value>
    }
  }
```

The application subscribes to "PRE_MQTT/config". The possible JSON config content is described here:
```
  {
    "wifi": {  // both wifi parameter have to be sent together
      "ssid": "<WIFI SSID>",
      "pw": "<own WIFI password>"
    },
    "broker": {  // both mqtt parameter have to be sent together
      "port": "<MQTT broker port>",
      "ip": "<MQTT broker IP>"
    },
    "dht": {
      "temp_offset": <int value>,
      "temp_level": <float value>,
      "hum_level": <int value>
    }
    "water": {   // zone and time have to be sent together, if program is sent do not sent time and zone
      "zone": <int value>,
      "time": <int value>,
      "program": <int value>
    }
  }
```

In case WIFI or MQTT parameters are changed, the application tries to connect with the new parameters.
If is succeeds, it stores the parameters permanently, otherwise the are thrown away and the previous parameters are used further.


- Wiring
<img src="hunter_esp8266_wiring.png" width="500" />
<!---   ![wire picture](hunter_esp8266_wiring.png)  --->

## Open Points
- [ ] make Serial.prints configurable to reduce code footprint
- [ ] Provide a reset via I/O pin to erase flash content and return to image defaults

## Parameter to be adjusted
### Class MQTT
- if your MQTT broker requests a username and password adjust the following variables
  - MQTT_username => "REPLACE_WITH_MQTT_USERNAME"
  - MQTT_password => "REPLACE_WITH_MQTT_PASSWORD"
- adjust MQTT_SERVER_IP to your default MQTT broker IP during build, can be configured with MQTT messages with working MQTT connection
- adjust MQTT_SERVER_PORT to your default MQTT broker port during build, can be configured with MQTT messages with working MQTT connection
- adjust PRE_MQTT in case you want to add specific preambles to your MQTT topics

The `MQTT` class is responsible for managing the connection to an MQTT broker, publishing data, and handling subscription messages.

#### Key Responsibilities
1. **Connection Management**:
   - The class sets up and maintains a connection to an MQTT broker.
   - It reconnects if the broker parameters change.

2. **Publishing Data**:
   - The class offers methods to publish different kinds of application data to the MQTT broker.

3. **Handling Subscription Messages**:
   - It includes a callback method to handle messages received from subscribed topics.

#### Key Components
- **MQTT Broker Credentials**:
  - `MQTT_username` and `MQTT_password` are used for authentication with the MQTT broker. They are set to placeholder values and should be replaced with actual credentials.

- **MQTT Client Initialization**:
  - `WiFiClient hunterClient`: This object is used to establish a WiFi connection.
  - `PubSubClient mqttClient(hunterClient)`: This object is used to manage the MQTT connection using the `hunterClient`.

- **Shared Data**:
  - `APP_DATA* pAppDataClass`: A pointer to an `APP_DATA` class instance, which is used to store shared data needed in the application. This pointer is also used in the subscription callback.

#### Initialization Method
- `void MQTT::initialize(OLED* oled, APP_DATA* appData)`:
  - This method initializes the MQTT class.
  - Parameters:
    - `OLED* oled`: A pointer to an OLED class instance, used to display status information.
    - `APP_DATA* appData`: A pointer to an `APP_DATA` class instance, storing shared data used in the application.
  - The method sets the class's `oled` and `appData` members and assigns the `appData` pointer to the global `pAppDataClass`.

#### Summary
The `MQTT` class provides essential functionality for connecting to an MQTT broker, publishing data, and handling incoming messages. It integrates with other components of the application, such as the OLED display for status updates and the `APP_DATA` class for shared data management.

### Class WIFI_CTRL
- adjust WIFI_SSID to your default WIFI SSID during build, can be configured with MQTT messages with working MQTT connection
- adjust WIFI_PASSWORD to your default WIFI password during build, can be configured with MQTT messages with working MQTT connection

### Class OLED
- check the defines in the header for OLED connections to the ESP8266 board
- it offers other options for other displays as well

### Class HUNTER_CTRL
- The header file offers defines for the water pump and the ESP8266 pin connection to HUNTER XCORE

### Class DHT_SENSOR
- adjust DHTPIN  to what pin is connected to the sensor
- adjust DHTTYPE to the type of DHT sensor to use, DHT 11 or DHT 21 or 22 ...
- adjust DEFAULT_DHT_PULLUP_TIME in case your sensor does not work correct
  Optionally pass pull-up time (in microseconds) before DHT
  reading starts. Default is 55 (see function declaration in DHT.h)
- adjust the MIN_INTERVAL in ms between reads of the sensor

## Class diagram

<img src="hunterCtrl.drawio.png" width="700" />
<!---  ![class diagram not found](hunterCtrl.drawio.png) --->

## License

Given that [Hans-Joachim](https://github.com/hjzimmer/hunter-control) published his code under the MIT license, this project follows the same license.

MIT License

Hunter Control
Copyright (C) 2024-2024  Hans-Joachim Zimmer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
