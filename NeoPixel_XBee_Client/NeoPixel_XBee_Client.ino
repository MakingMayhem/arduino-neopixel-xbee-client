// 
// WX_Lantern_Client.ino
// Copyright (C) 2014  David L Kinney <david@kinney.io>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

////////////////////////////////////////////////////////////////////////////////
// USAGE
// 
// This sketch allows remote control of an Adafruit NeoPixel Shield attached to 
// an Arduino UNO using an XBee Series 2 (ZigBee) device on an Arduino Wireless 
// SD Shield. The XBee must be configured with: 
// 
// * `ATZS 2` (ZigBee-PRO stack)
// * `ATAP 2` (API mode with escaping)
// * `ATAO 1` (enable explicit RX)

////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION

// Maximum brightness for the NeoPixels, between 0 and 255. See 
// Adafruit_NeoPixel.setBrightness(uint8_t) for more information.
#define NEOPIXEL_BRIGHTNESS 255

// Number of NeoPixels being controlled. The NeoPixel Shield has 40.
#define NEOPIXEL_COUNT 40

// Digital IO pin to use to communicate with the NeoPixels. The NeoPixel Shield 
// uses pin 6.
#define NEOPIXEL_PIN 6

// Speed of serial communication between the Arduino and XBee. Usually 9600. 
#define XBEE_BAUD 9600

// Flag to control whether XBee serial communication uses Serial or 
// SoftwareSerial. It is useful to use SoftwareSerial (e.g., with the SparkFun 
// XBee Shield) while debugging so that status messages can be printed to the 
// USB console. It is recommended that the hardware-supported Serial interface 
// be used after debugging is complete. 
// 
// 0 -> XBee uses Serial (pins 0,1)
// 1 -> XBee uses SoftwareSerial (pins 2,3)
#define XBEE_SOFTWARE_SERIAL 0

////////////////////////////////////////////////////////////////////////////////
// INCLUDES

// Standard Arduino libraries
#include <SoftwareSerial.h>

// Third-party libraries
#include <Adafruit_NeoPixel.h>
#include <XBee.h>


////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

#define COMMAND_INDEXED_UPDATE 0x4e

Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

XBee xbee = XBee();
ZBRxResponse zbRx = ZBRxResponse();

#if XBEE_SOFTWARE_SERIAL
SoftwareSerial xbeeSerial = SoftwareSerial(2,3);
#else
#define xbeeSerial Serial
#endif

////////////////////////////////////////////////////////////////////////////////
// PROGRAM

void setup()  
{
  #if XBEE_SOFTWARE_SERIAL
  Serial.begin(57600);
  #endif
  
  xbeeSerial.begin(XBEE_BAUD);
  xbee.setSerial(xbeeSerial);
  
  neopixels.begin();
  neopixels.setBrightness(NEOPIXEL_BRIGHTNESS);
  neopixels.show(); // initialize all pixels to "off"
}

void loop() // run over and over
{
  xbee.readPacket(1000);
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // copy data out of xbee object because next call to readPacket() destroys it
      xbee.getResponse().getZBRxResponse(zbRx);
      uint8_t dataLength = zbRx.getDataLength();
      uint8_t *data = zbRx.getData();
      
      // extract the NeoPixel command type
      uint8_t commandType = (dataLength > 0) ? data[0] : 0x00;
      
      // extract the command's input
      uint8_t *pixelUpdates = data + sizeof(uint8_t); // start at index 1
      uint8_t pixelUpdatesLength = dataLength - 1; // starting at index 1 means length is 1 less
      
      if (COMMAND_INDEXED_UPDATE == commandType) {
        handleIndexedUpdateCommand(pixelUpdates, pixelUpdatesLength);
      } else {
        // not a recognized command
      }
      
      // prepare the RX object for reuse
      zbRx.reset();
    }   
  } else if (xbee.getResponse().isError()) {
    // TODO: handle error somehow
  }
}

void handleIndexedUpdateCommand(uint8_t const * const pixelUpdates, uint8_t const pixelUpdatesLength)
{
  #define PIXEL_UPDATE_SIZE 4
  #define PIXEL_OFFSET 0
  #define RED_OFFSET   1
  #define GREEN_OFFSET 2
  #define BLUE_OFFSET  3

  int index = 0;
  while (index + PIXEL_UPDATE_SIZE <= pixelUpdatesLength) {
    uint8_t pixel = pixelUpdates[index + PIXEL_OFFSET];
    uint8_t red   = pixelUpdates[index + RED_OFFSET];
    uint8_t green = pixelUpdates[index + GREEN_OFFSET];
    uint8_t blue  = pixelUpdates[index + BLUE_OFFSET];
    
    // debugging
    #if XBEE_SOFTWARE_SERIAL
    Serial.print('[');
    Serial.print(index, DEC);
    Serial.print("]: ");
    
    Serial.print(pixel, DEC);
    Serial.print(' ');
    Serial.print(red,   DEC);
    Serial.print(' ');
    Serial.print(green, DEC);
    Serial.print(' ');
    Serial.print(blue,  DEC);
    Serial.println();
    #endif
    
    neopixels.setPixelColor(pixel, red, green, blue);
    
    index += PIXEL_UPDATE_SIZE;
  }
  
  // Send the new pixel colors to the NeoPixels
  neopixels.show();
}

