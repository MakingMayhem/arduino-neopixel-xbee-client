# NeoPixel XBee Client

This Arduino sketch allows Adafruit NeoPixels to be controlled by a remote 
ZigBee device. 


## Getting Started

You will need to install the following third-party Arduino libraries:

* [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel)
* [XBee-Arduino Library](https://code.google.com/p/xbee-arduino/)

This sketch works out-of-the-box with the following hardware:

* [Arduino UNO](http://arduino.cc/en/Main/ArduinoBoardUno)
* [Arduino Wireless SD Shield](http://arduino.cc/en/Main/ArduinoWirelessShield) 
  or [equivalent](https://www.sparkfun.com/products/12847)
* [Adafruit NeoPixel Shield for Arduino](https://www.adafruit.com/products/1430)
* [Digi XBee ZB](http://www.digi.com/products/wireless-wired-embedded-solutions/zigbee-rf-modules/zigbee-mesh-module/xbee-zb-module), 
  formerly "XBee Series 2"

The XBee ZB must be configured with:

* `ATAP 2` (API mode with escaping, required by the XBee-Arduino Library)
* `ATAO 0` (Default receive API indicators enabled)


## Protocol

The sketch expects to a binary payload containing a starting byte ("marker") 
that specifies the command type followed by data for that command. 

Currently, only the **Indexed Updates** command is supported. This command 
allows the sender to specify each pixel to be updated. The command's marker is 
`0x4E`. That is followed by up to 16 pixel updates. Each pixel update is 4 
bytes: the first byte identifies the pixel to change; the rest specify the 
pixel's RGB color. To update more than 16 pixels, send multiple packets. 

### BNF

The protocol payload is defined as follows:

    PAYLOAD     = CMD_INDEXED
    CMD_INDEXED = IDX_MARKER IDX_UPDATES  ; update pixels by addressing each pixel
    IDX_MARKER  = %x4E  ; hex value 0x4E is used to specify Indexed Updates
    IDX_UPDATES = *16(IDX_PIXEL) ; 0-16 pixel updates
    IDX_PIXEL   = PIXEL_IDX PIXEL_RGB
    PIXEL_IDX   = OCTET ; the 0-based index of the pixel to be updated
    PIXEL_RGB   = RED GREEN BLUE ; the RGB values (0-255) for the pixel
    RED         = OCTET
    GREEN       = OCTET
    BLUE        = OCTET
    OCTET       = <any 8-bit sequence of data>
	
### Examples

To change the 12th pixel (at index 11) to bright purple:

    4E 0B FF 00 FF
    
To change the first four pixels green:

    4E 00 00 FF 00 01 00 FF 00 02 00 FF 00 03 00 FF 00

### Discussion

The protocol's maximum payload size is 65 bytes. This limit was chosen due to 
the following constraints:

* The XBee-Arduino library supports a maximum _packet_ size of 100 bytes. (This 
  can be changed by editing the library's `.h` file.)
* The [Digi knowledge base](http://www.digi.com/support/kbase/kbaseresultdetl?id=3345) 
  states that the maximum _non-fragmented_ payload size is 66 bytes when using 
  encryption. 


## Control Through Device Cloud

If you have a [Digi XBee Gateway](http://www.digi.com/products/wireless-routers-gateways/gateways/xbee-gateway) 
or [Digi ConnectPort](http://www.digi.com/products/wireless-routers-gateways/gateways/connectportx2gateways), 
you can control the NeoPixels across the Internet using [Device Cloud](http://www.etherios.com/products/devicecloud/) 
by sending [Server Command Interface (SCI)](http://ftp1.digi.com/support/documentation/html/90002008/90002008_R/Default.htm#Programming%20Topics/SCI.htm%3FTocPath%3DDevice%2520Cloud%2520Programming%2520Guide%7CSCI%7C_____0) 
commands. 

An example SCI command to change the first four pixels green:

    <sci_request version="1.0">
     <send_message>
      <targets>
       <device id="00000000-00000000-004321AB-CDEF0123"/>
      </targets>
      <rci_request version="1.1">
       <do_command target="xbgw">
        <send_serial addr="0013A20012345678" encoding="base64">
         TgAA/wABAP8AAgD/AAMA/wA=
        </send_serial>
       </do_command>
      </rci_request>
     </send_message>
    </sci_request>

You will need to change `device id="…"` to the Device Cloud identifier for your 
XBee Gateway or ConnectPort and `send_serial addr="…"` to the 64bit address of 
your XBee attached to the Arduino.


## License

This software is licensed under the terms of the GPLv3. Both of its third-party 
software dependencies (Adafruit's NeoPixel library and the XBee-Arduino 
library) are also licensed under the terms of the GPLv3. See the `LICENSE` file 
for more information. 
