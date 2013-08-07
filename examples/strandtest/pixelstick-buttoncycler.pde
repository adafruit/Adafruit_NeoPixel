#include <Adafruit_NeoPixel.h>
// This is a demonstration on how to use an input device to trigger changes on your neo pixels.  


#define PIN 2
// Connect button to give +5v on press to pin2.
const int buttonPin = 2; 
int buttonState = 0;
// Initial showtype isn't run on load, and show type 0 is set to wipe all LEDs to blank.
int showType = 0;

#define PIN 6


// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick



Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, 6, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(buttonPin, INPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {  
    showType++;
    if (showType >9) showType=0;
    startShow(showType);
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void colorTest(uint32_t c, uint16_t n, uint8_t wait) {
  strip.setPixelColor(n, c);
  strip.show();
  delay(wait);
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

void startShow(int i) {
  switch(i){
   case 0:  colorWipe(strip.Color(0, 0, 0), 50); break; 
   case 1:  colorTest(strip.Color(255, 0, 0), 0, 150);
            colorTest(strip.Color(0, 255, 0), 1, 150); 
            colorTest(strip.Color(255, 0, 0), 2, 150); 
            colorTest(strip.Color(0, 255, 0), 3, 150); break; 
   case 2:  colorTest(strip.Color(255, 0, 0), 4, 150);
            colorTest(strip.Color(0, 255, 0), 5, 150); 
            colorTest(strip.Color(255, 0, 0), 6, 150); 
            colorTest(strip.Color(0, 255, 0), 7, 150); break;  
   case 3:  colorTest(strip.Color(255, 255, 0), 0, 150);
            colorTest(strip.Color(0, 255, 255), 1, 150); 
            colorTest(strip.Color(255, 255, 255), 2, 150); 
            colorTest(strip.Color(255, 0, 255), 3, 150);
            colorTest(strip.Color(100, 20, 150), 4, 150);
            colorTest(strip.Color(0, 180, 60), 5, 150); 
            colorTest(strip.Color(200, 0, 50), 6, 150); 
            colorTest(strip.Color(255, 255, 255), 7, 150); break;
   case 4:  colorWipe(strip.Color(255, 0, 0), 50); break;
   case 5:  colorWipe(strip.Color(0, 255, 0), 50); break;
   case 6:  colorWipe(strip.Color(0, 0, 255), 50); break;
   case 7:  rainbow       (20); break;
   case 8:  rainbowCycle  (3); break;
  }
}
