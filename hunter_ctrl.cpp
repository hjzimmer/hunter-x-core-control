/*!
 *  @file hunter_ctrl.cpp
 *
 *  @mainpage  class controlling a HUNTER XCORE watering station.
 *
 *  @section intro_sec Introduction
 *
 *  This is an abstraction class to handle request to start a HUNTER XCORE zone for some time
 *  or start a program, the communication to XCORE is done via oneWire HUNTER_PIN
 *  it support a punp control as well, which can be connected with a relay
 *
 *  It has been adapted from
 * 		- Claude <https://www.loullingen.lu/projekte/Hunter/index.php>
 *    - Sebastian <https://github.com/seb821/OpenSprinkler-Firmware-Hunter>
 *
 *  @section author Author
 *
 *  Written by Hans-Joachim Zimmer.
 *
 *  @section license License
 *
 *  MIT license, all text above must be included in any redistribution
 */
 
#include "HUNTER_CTRL.h"
#include <vector>

// definitions to adjust signaling on the HUNTER control line
#define HUNTER_ONE HIGH         // This makes inverting the signal easy
#define HUNTER_ZERO LOW
#define START_INTERVAL 900
#define SHORT_INTERVAL 208
#define LONG_INTERVAL 1875

// Forward declarations
/*!
 *  @brief  Stops the specified Hunter zone.
 *  @param  zone  The zone to stop.
 */
void HunterStop(byte zone);

/*!
 *  @brief  Starts the specified Hunter zone for a given time.
 *  @param  zone  The zone to start.
 *  @param  time  The duration to run the zone.
 */
void HunterStart(byte zone, byte time);

/*!
 *  @brief  Starts a Hunter program.
 *  @param  num  The program number to start.
 */
void HunterProgram(byte num);

/*!
 *  @brief  Sets a bitfield in a vector of bytes.
 *  @param  bits  The vector of bytes.
 *  @param  pos   The position to start setting bits.
 *  @param  val   The value to set.
 *  @param  len   The length of the bitfield.
 */
void HunterBitfield(std::vector<byte>& bits, byte pos, byte val, byte len);

/*!
 *  @brief  Writes a buffer to the Hunter controller.
 *  @param  buffer    The buffer to write.
 *  @param  extrabit  An extra bit to write.
 */
void HunterWrite(std::vector<byte> buffer, bool extrabit);

/*!
 *  @brief  Sets the Hunter signal to low.
 */
void HunterLow(void);

/*!
 *  @brief  Sets the Hunter signal to high.
 */
void HunterHigh(void);


/*********************************************************
   Class functions */

/*!
 *  @brief  Initializes a new HUNTER class.
 *  @param  oled     Pointer to the OLED display object.
 *  @param  appData  Pointer to the application data object.
 */
void HUNTER_CTRL::initialize(OLED* oled, APP_DATA* appData) {
  this->oled = oled;
  this->appData = appData;

  if (USE_PUMP == true) {
    // Define outputs for pump control and bus communication to X-Core controller
    pinMode(PUMP_PIN, OUTPUT); // GPIO5 to switch the pump
    pinMode(HUNTER_PIN, OUTPUT); // Bus Port, see value of HUNTER_PIN in hunter.h

    // Set PUMP_PIN to default value
    if (PUMP_PIN_DEFAULT) {
      digitalWrite(PUMP_PIN, HIGH);
    } else {
      digitalWrite(PUMP_PIN, LOW);
    }
  }
}

/*!
 *  @brief  Switches the pump on or off.
 *  @param  onOff  Boolean indicating whether to turn the pump on or off.
 */
void HUNTER_CTRL::switchPump(boolean onOff) {
  if (USE_PUMP == true) {
    // Set GPIO5 to control the pump and set flag to send a message to the X-Core controller 
    if (onOff) {
      Serial.println("Set GPIO5 high.");
      digitalWrite(5, HIGH); 
    } else {
      Serial.println("Set GPIO5 low.");
      digitalWrite(5, LOW);
    }
  }
}

/*!
 *  @brief  Starts a watering zone for a specified time.
 *  @param  zone  The zone number.
 *  @param  time  The duration in minutes.
 */
void HUNTER_CTRL::startZone(const int zone, const int time) {
  String msg = "Watering zone " + (String)zone + " -> " + (String)time + (String)" min";
  Serial.println(msg);
  if (oled) {
    oled->updateAction(msg.c_str());
    oled->updateHunterInfo(zone, time);
  }
  
  if (time) {switchPump(true);} else {switchPump(false);}
  HunterStart((byte)zone, (byte)time);
}

/*!
 *  @brief  Starts a watering program.
 *  @param  programID  The ID of the program to start.
 */
void HUNTER_CTRL::startProgram(const int programID) {
  String msg = "Watering prog " + (String)programID + " ...";
  Serial.println(msg);
  if (oled) {
    oled->updateAction(msg.c_str());
    oled->updateHunterInfo(0, 0, programID);
  }
  HunterProgram(programID);
}

/*   endClass functions 
******************************************************** */


/////////////////////////////////////////////////////////////////////////////
// Function: HunterBitfield
// Description: Set a value with an arbitrary bit width to a bit position
// within a blob
// Arguments: bits - blob to write the value to
// pos - bit position within the blob
// val - value to write
// len - len in bits of the value
/////////////////////////////////////////////////////////////////////////////
void HunterBitfield(std::vector<byte>& bits, byte pos, byte val, byte len) {
  while (len > 0) {
    if (val & 0x1) {
      bits[pos / 8] = bits[pos / 8] | 0x80 >> (pos % 8);
    } else {
      bits[pos / 8] = bits[pos / 8] & ~(0x80 >> (pos % 8));
    }
    len--;
    val = val >> 1;
    pos++;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Function: HunterLow
// Description: writes a low bit on the bus
// Arguments: none
/////////////////////////////////////////////////////////////////////////////
void HunterLow(void) {
  digitalWrite(HUNTER_PIN, HUNTER_ONE);
  delayMicroseconds(SHORT_INTERVAL);  //microseconds
  digitalWrite(HUNTER_PIN, HUNTER_ZERO);
  delayMicroseconds(LONG_INTERVAL);  //microseconds
}

/////////////////////////////////////////////////////////////////////////////
// Function: HunterHigh
// Description: writes a high bit on the bus
// Arguments: none
/////////////////////////////////////////////////////////////////////////////
void HunterHigh(void) {
  digitalWrite(HUNTER_PIN, HUNTER_ONE);
  delayMicroseconds(LONG_INTERVAL);  //microseconds
  digitalWrite(HUNTER_PIN, HUNTER_ZERO);
  delayMicroseconds(SHORT_INTERVAL);  //microseconds
}

/////////////////////////////////////////////////////////////////////////////
// Function: HunterWrite
// Description: Write the bit sequence out of the bus
// Arguments: buffer - blob containing the bits to transmit
// extrabit - if true, then write an extra 1 bit
/////////////////////////////////////////////////////////////////////////////
void HunterWrite(std::vector<byte> buffer, bool extrabit) {
  // Bus start sequence
  //_flexwire.invert(false);
  //delay(325);
  //_flexwire.invert(true);
  //delay(65);

  // Resetimpulse
  digitalWrite(HUNTER_PIN, HUNTER_ONE);
  delay(325);  //milliseconds
  digitalWrite(HUNTER_PIN, HUNTER_ZERO);
  delay(65);  //milliseconds

  // Write the start pulse
  //_flexwire.invert(false);
  //udelay(900);
  //_flexwire.invert(true);

  // Startimpulse
  digitalWrite(HUNTER_PIN, HUNTER_ONE);
  delayMicroseconds(START_INTERVAL);  //microseconds
  digitalWrite(HUNTER_PIN, HUNTER_ZERO);
  delayMicroseconds(SHORT_INTERVAL);  //microseconds

  // Write the bits out
  //_flexwire.write(buffer);
  for (auto& sendByte : buffer) {
    for (byte inner = 0; inner < 8; inner++) {
      // Send high order bits first
      if (sendByte & 0x80) {
        HunterHigh();
      } else {
        HunterLow();
      }
      sendByte <<= 1;
    }
  }

  // Include an extra 1 bit
  if (extrabit) {
    //_flexwire.writebit(true);
    HunterHigh();
  }

  // Write the stop pulse
  //_flexwire.writebit(false);
  HunterLow();
}

/////////////////////////////////////////////////////////////////////////////
// Function: start
// Description: Start a zone
// Arguments: zone - zone number (1-48)
// time - time in minutes (0-240)
/////////////////////////////////////////////////////////////////////////////
void HunterStart(byte zone, byte time) {

  Serial.print("HunterStart zone ");
  Serial.print(zone);
  Serial.print(" for ");
  Serial.print(time);
  Serial.println(" min");

  // Start out with a base frame
  std::vector<byte> buffer = { 0xff, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x01, 0xb8, 0x3f };

  if (zone < 1 || zone > 48) {
    Serial.println("invalid zone");
    return;
  }

  if (time < 0 || time > 240) {
    Serial.println("invalid time");
    return;
  }

  // The bus protocol is a little bizzare, not sure why

  // Bits 9:10 are 0x1 for zones > 12 and 0x2 otherwise
  if (zone > 12) {
    HunterBitfield(buffer, 9, 0x1, 2);
  } else {
    HunterBitfield(buffer, 9, 0x2, 2);
  }

  // Zone + 0x17 is at bits 23:29 and 36:42
  HunterBitfield(buffer, 23, zone + 0x17, 7);
  HunterBitfield(buffer, 36, zone + 0x17, 7);

  // Zone + 0x23 is at bits 49:55 and 62:68
  HunterBitfield(buffer, 49, zone + 0x23, 7);
  HunterBitfield(buffer, 62, zone + 0x23, 7);

  // Zone + 0x2f is at bits 75:81 and 88:94
  HunterBitfield(buffer, 75, zone + 0x2f, 7);
  HunterBitfield(buffer, 88, zone + 0x2f, 7);

  // Time is encoded in three places and broken up by nibble
  // Low nibble: bits 31:34, 57:60, and 83:86
  // High nibble: bits 44:47, 70:73, and 96:99
  HunterBitfield(buffer, 31, time, 4);
  HunterBitfield(buffer, 44, time >> 4, 4);
  HunterBitfield(buffer, 57, time, 4);
  HunterBitfield(buffer, 70, time >> 4, 4);
  HunterBitfield(buffer, 83, time, 4);
  HunterBitfield(buffer, 96, time >> 4, 4);

  // Bottom nibble of zone - 1 is at bits 109:112
  HunterBitfield(buffer, 109, zone - 1, 4);

  // Write the bits out of the bus
  HunterWrite(buffer, true);
}

/////////////////////////////////////////////////////////////////////////////
// Function: stop
// Description: Stop all zones
// Arguments: None
/////////////////////////////////////////////////////////////////////////////
void HunterStop(byte zone) {
  Serial.print("HunterStop zone ");
  Serial.print(zone);
  Serial.print(": ");
  HunterStart(zone, 0);
}

/////////////////////////////////////////////////////////////////////////////
// Function: program
// Description: Run a program
// Arguments: num - program number (1-4)
/////////////////////////////////////////////////////////////////////////////
void HunterProgram(byte num) {
  // Start with a basic program frame
  std::vector<byte> buffer = { 0xff, 0x40, 0x03, 0x96, 0x09, 0xbd, 0x7f };

  if (num < 1 || num > 4) {
    Serial.println("invalid program");
    return;
  }

  // Program number - 1 is at bits 31:32
  HunterBitfield(buffer, 31, num - 1, 2);
  HunterWrite(buffer, false);
}
