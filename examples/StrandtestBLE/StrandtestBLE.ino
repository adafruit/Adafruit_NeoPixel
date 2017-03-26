/****************************************************************************
 * This example was developed by the Hackerspace San Salvador to demonstrate
 * the simultaneous use of the NeoPixel library and the Bluetooth SoftDevice.
 * To compile this example you'll need to add support for the NRF52 based
 * following the instructions at: 
 *  https://github.com/sandeepmistry/arduino-nRF5
 * Or adding the following URL to the board manager URLs on Arduino preferences:
 *  https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json
 * Then you can install the BLEPeripheral library avaiable at:
 *  https://github.com/sandeepmistry/arduino-BLEPeripheral
 * To test it, compile this example and use the UART module from the nRF
 * Toolbox App for Android. Edit the interface and send the characters
 * 'a' to 'i' to switch the animation.
 * There is a delay because this example blocks the thread of execution but
 * the change will be shown after the current animation ends. (This might
 * take a couple of seconds)
 * For more info write us at: info _at- teubi.co
 */
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"
#include <Adafruit_NeoPixel.h>

#define PIN 15

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// create ble serial instance, see pinouts above
BLESerial BLESerial(BLE_REQ, BLE_RDY, BLE_RST);
            
uint8_t current_state = 0;
uint8_t rgb_values[3];

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");
  // custom services and characteristics can be added as well
  BLESerial.setLocalName("UART_HS");
  BLESerial.begin();

  strip.begin();
  changeColor(strip.Color(0, 0, 0));

  //pinMode(PIN, OUTPUT);
  //digitalWrite(PIN, LOW);

  current_state = 'a';
}


void loop() {
  while(BLESerial.available()) {
    uint8_t character = BLESerial.read();
    switch(character) {
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
        current_state = character;
        break;
    };
  }
  switch(current_state) {
    case 'a':
      colorWipe(strip.Color(255, 0, 0), 20); // Red
      break;
    case 'b':
      colorWipe(strip.Color(0, 255, 0), 20); // Green
      break;
    case 'c':
      colorWipe(strip.Color(0, 0, 255), 20); // Blue
      break;
    case 'd':
      theaterChase(strip.Color(255, 0, 0), 20); // Red
      break;
    case 'e':
      theaterChase(strip.Color(0, 255, 0), 20); // Green
      break;
    case 'f':
      theaterChase(strip.Color(255, 0, 255), 20); // Green
      break;
    case 'g':
      rainbowCycle(20);
      break;
    case 'h':
      rainbow(20);
      break;
    case 'i':
      theaterChaseRainbow(20);
      break;
  }
}

void changeColor(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    delay(wait);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
