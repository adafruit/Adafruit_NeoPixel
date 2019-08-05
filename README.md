# Adafruit NeoPixel Library [![Build Status](https://travis-ci.com/adafruit/Adafruit_NeoPixel.svg?branch=master)](https://travis-ci.com/adafruit/Adafruit_NeoPixel)

Arduino library for controlling single-wire-based LED pixels and strip such as the [Adafruit 60 LED/meter Digital LED strip][strip], the [Adafruit FLORA RGB Smart Pixel][flora], the [Adafruit Breadboard-friendly RGB Smart Pixel][pixel], the [Adafruit NeoPixel Stick][stick], and the [Adafruit NeoPixel Shield][shield].

After downloading, rename folder to 'Adafruit_NeoPixel' and install in Arduino Libraries folder. Restart Arduino IDE, then open File->Sketchbook->Library->Adafruit_NeoPixel->strandtest sketch.

Compatibility notes: Port A is not supported on any AVR processors at this time

[flora]:  http://adafruit.com/products/1060
[strip]:  http://adafruit.com/products/1138
[pixel]:  http://adafruit.com/products/1312
[stick]:  http://adafruit.com/products/1426
[shield]: http://adafruit.com/products/1430

---

## Supported chipsets

We have included code for the following chips - *sometimes these break for exciting reasons that we can't control* in which case please open an issue!

 * AVR ATmega and ATtiny (any 8-bit) - 8 MHz, 12 MHz and 16 MHz
 * Teensy 3.x and LC
 * Arduino Due
 * Arduino 101
 * ATSAMD21 (Arduino Zero/M0 and other SAMD21 boards) @ 48 MHz
 * ATSAMD51 @ 120 MHz
 * Adafruit STM32 Feather @ 120 MHz
 * ESP8266 any speed
 * ESP32 any speed
 * Nordic nRF52 (Adafruit Feather nRF52), nRF51 (micro:bit)

Check forks for other architectures not listed here!

---

### Roadmap

The PRIME DIRECTIVE is to maintain backward compatibility with existing Arduino sketches -- many are hosted elsewhere and don't track changes here, some are in print and can never be changed!

Please don't reformat code for the sake of reformatting code. The resulting large "visual diff" makes it impossible to untangle actual bug fixes from merely rearranged lines. (Exception for first item in wishlist below.)

Things I'd Like To Do But There's No Official Timeline So Please Don't Count On Any Of This Ever Being Canonical:

  * For the show() function (with all the delicate pixel timing stuff), break out each architecture into separate source files rather than the current unmaintainable tangle of #ifdef statements!
  * Please don't use updateLength() or updateType() in new code. They should not have been implemented this way (use the C++ 'new' operator with the regular constructor instead) and are only sticking around because of the Prime Directive. setPin() is OK for now though, it's a trick we can use to 'recycle' pixel memory across multiple strips.
  * In the M0 and M4 code, use the hardware systick counter for bit timing rather than hand-tweaked NOPs (a temporary kludge at the time because I wasn't reading systick correctly). (As of 1.4.2, systick is used on M4 devices and it appears to be overclock-compatible. Not for M0 yet, which is why this item is still here.)
  * As currently written, brightness scaling is still a "destructive" operation -- pixel values are altered in RAM and the original value as set can't be accurately read back, only approximated, which has been confusing and frustrating to users. It was done this way at the time because NeoPixel timing is strict, AVR microcontrollers (all we had at the time) are limited, and assembly language is hard. All the 32-bit architectures should have no problem handling nondestructive brightness scaling -- calculating each byte immediately before it's sent out the wire, maintaining the original set value in RAM -- the work just hasn't been done. There's a fair chance even the AVR code could manage it with some intense focus. (The DotStar library achieves nondestructive brightness scaling because it doesn't have to manage data timing so carefully...every architecture, even ATtiny, just takes whatever cycles it needs for the multiply/shift operations.)
