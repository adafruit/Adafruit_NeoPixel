// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
// This sketch show how to use "new" operator with Adafruit_NeoPixel library
// It's helpful if you don't know NeoPixel settings at compile time or
// you just want to store this settings in EEPROM

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
int numPixel = 16;

// Color order, for more information see https://github.com/adafruit/Adafruit_NeoPixel/blob/master/Adafruit_NeoPixel.h
uint8_t colorOrder = 0x52; //or just use NEO_GBR

// Define new pointer for NeoPixel
Adafruit_NeoPixel *pixels;


int delayval = 500; // delay for half a second

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
  
  // Here is a good place to read numPixel & colorOrder from EEPROM or what ever.
  // create a new NeoPixel instance with new values
  pixels = new Adafruit_NeoPixel(numPixel, PIN, colorOrder);
  pixels->begin(); // This initializes the NeoPixel library.
}

void loop() {

  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

  for(int i=0;i<numPixel;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels->setPixelColor(i, pixels->Color(0,150,0)); // Moderately bright green color.

    pixels->show(); // This sends the updated pixel color to the hardware.

    delay(delayval); // Delay for a period of time (in milliseconds).

  }
}
