#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. Flora pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
NeoPixel strip = NeoPixel(30, 6, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
}

void loop() {
  for(int i=0; i<256; i += 5) {
    fill(strip.Color(i, 0, 0));
    fill(strip.Color(0, i, 0));
    fill(strip.Color(0, 0, i));
    fill(strip.Color(0, 0, 0));
  }
}

void fill(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(20);
  }
}

