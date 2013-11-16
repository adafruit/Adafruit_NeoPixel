/*
NeoPixel Ring simple sketch (c) 2013 Shae Erisson
released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
*/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  #include <avr/power.h>
#endif

// Which pin on the FLORA is connected to the NeoPixel ring?
#define PIN            6

// We're using only one ring with 16 NeoPixel
#define NUMPIXELS      16

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

void setup() {
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
  // Seed random number generator from an unused analog input:
  randomSeed(analogRead(2));
#else
  randomSeed(analogRead(A0));
#endif
  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {
  // for one ring of 16, the first NeoPixel is 0, second is 1, all the way up to 15
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i,pixels.Color(0,150,0)); // we choose green
    pixels.show(); // this sends the value once the color has been set
    delay(delayval);
  }
}
