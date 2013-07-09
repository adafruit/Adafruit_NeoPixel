/*--------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  --------------------------------------------------------------------*/

#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

// 'type' flags for LED pixels (third parameter to constructor):
#define NEO_RGB     0x00 // Wired for RGB data order
#define NEO_GRB     0x01 // Wired for GRB data order
#define NEO_COLMASK 0x01
#define NEO_KHZ400  0x00 // 400 KHz datastream
#define NEO_KHZ800  0x02 // 800 KHz datastream
#define NEO_SPDMASK 0x02

class Adafruit_NeoPixel {

 public:

  // Constructor: number of LEDs, pin number, LED type
  Adafruit_NeoPixel(uint16_t n, uint8_t p=6, uint8_t t=NEO_GRB + NEO_KHZ800);

  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    setBrightness(uint8_t);
  uint16_t
    numPixels(void);
  static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  uint32_t
    getPixelColor(uint16_t n);

 private:

  const uint16_t
    numLEDs,       // Number of RGB LEDs in strip
    numBytes;      // Size of 'pixels' buffer below
  const uint8_t
    pin,           // Output pin number
    type;          // Pixel flags (400 vs 800 KHz, RGB vs GRB color)
  uint8_t
    brightness,
   *pixels;        // Holds LED color values (3 bytes each)
  uint32_t
    endTime;       // Latch timing reference
#ifdef __AVR__
  const volatile uint8_t
    *port;         // Output PORT register
  const uint8_t
    pinMask;       // Output PORT bitmask
#endif

};

#endif // ADAFRUIT_NEOPIXEL_H
