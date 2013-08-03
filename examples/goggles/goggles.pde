/*
NeoPixel Ring goggles sketch -- for steampunk, rave or Burning Man fashion!
Welding or costume goggles using 50mm round lenses can be outfitted with
a pair of Adafruit NeoPixel Rings: http://www.adafruit.com/product/1463

Please exercise common sense.  These goggles emit a LOT of stray light and
should NOT BE WORN ON YOUR EYES.  They're for fashion and costuming only,
for display on a hat or on your forehead.

Draws a spinning rainbow on both eyepieces.  Not a Mac beachball, honest.
"Eyes" glance around and blink at random.

For 'reflected' colors (rainbows rotate in opposite directions, while eyes
look in same direction), connect the output of the first ring to the input
of the second.  Or you can connect the inputs of both rings to the same
Arduino pin if that's easier -- the rainbows will both twirl in the same
direction in that case.

Pixel #1 on both rings should be at the TOP of the goggles.  Looking at
the BACK of the board, pixel #1 is immediately clockwise from the OUT
connection.
*/

#include <Adafruit_NeoPixel.h>

#define PIN 4

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(32, PIN, NEO_GRB + NEO_KHZ800);

// Vertical coordinate of each pixel.  First pixel is at top.
const int8_t PROGMEM yCoord[] = {
  127,117,90,49,0,-49,-90,-117,-127,-117,-90,-49,0,49,90,117 };

// Eyelid vertical coordinates.  Eyes shut slightly below center.
#define upperLidTop     130
#define upperLidBottom  -45
#define lowerLidTop     -40
#define lowerLidBottom -130

// Gamma correction improves appearance of midrange colors
const uint8_t PROGMEM gamma8[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
  0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,
  0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x06,
  0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,0x09,0x09,0x09,
  0x0a,0x0a,0x0a,0x0b,0x0b,0x0b,0x0c,0x0c,0x0d,0x0d,0x0d,0x0e,
  0x0e,0x0f,0x0f,0x10,0x10,0x11,0x11,0x12,0x12,0x13,0x13,0x14,
  0x14,0x15,0x15,0x16,0x16,0x17,0x18,0x18,0x19,0x19,0x1a,0x1b,
  0x1b,0x1c,0x1d,0x1d,0x1e,0x1f,0x1f,0x20,0x21,0x22,0x22,0x23,
  0x24,0x25,0x26,0x26,0x27,0x28,0x29,0x2a,0x2a,0x2b,0x2c,0x2d,
  0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
  0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x44,0x45,0x46,
  0x47,0x48,0x49,0x4b,0x4c,0x4d,0x4e,0x50,0x51,0x52,0x54,0x55,
  0x56,0x58,0x59,0x5a,0x5c,0x5d,0x5e,0x60,0x61,0x63,0x64,0x66,
  0x67,0x69,0x6a,0x6c,0x6d,0x6f,0x70,0x72,0x73,0x75,0x77,0x78,
  0x7a,0x7c,0x7d,0x7f,0x81,0x82,0x84,0x86,0x88,0x89,0x8b,0x8d,
  0x8f,0x91,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,0xa0,0xa2,0xa4,
  0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,
  0xbf,0xc1,0xc3,0xc5,0xc7,0xca,0xcc,0xce,0xd1,0xd3,0xd5,0xd7,
  0xda,0xdc,0xdf,0xe1,0xe3,0xe6,0xe8,0xeb,0xed,0xf0,0xf2,0xf5,
  0xf7,0xfa,0xfc,0xff
};

uint32_t
  iColor[16][3];      // Background colors for eyes
int16_t
  hue          =   0; // Initial hue around perimeter (0-1535)
uint8_t
  iBrightness[16],    // Brightness map -- eye colors get scaled by these
  brightness   = 220, // Global brightness (0-255)
  blinkFrames  =   5, // Speed of current blink
  blinkCounter =  30, // Countdown to end of next blink
  eyePos       = 192, // Current 'resting' eye (pupil) position
  newEyePos    = 192, // Next eye position when in motion
  gazeCounter  =  75, // Countdown to next eye movement
  gazeFrames   =  50; // Duration of eye movement (smaller = faster)
int8_t
  eyeMotion    =   0; // Distance from prior to new position


void setup() {
  pixels.begin();

  // Seed random number generator from an unused analog input:
  randomSeed(analogRead(A0));
}

void loop() {
  uint8_t i, r, g, b, a, c, inner, outer, ep;
  int     y1, y2, y3, y4, h;
  int8_t  y;

  // Draw eye background colors (glotating rainbow)
  for(h=hue, i=0; i<16; i++, h += 96) {
    a = h >> 8;
    b = h;
    switch(a % 6) {
     case 0: // Red to yellow
      iColor[i][0] = 255; iColor[i][1] =   b; iColor[i][2] =   0;
      break;
     case 1: // Yellow to green
      iColor[i][0] =  ~b; iColor[i][1] = 255; iColor[i][2] =   0;
      break;
     case 2: // Green to cyan
      iColor[i][0] =   0; iColor[i][1] = 255; iColor[i][2] =   b;
      break;
     case 3: // Cyan to blue
      iColor[i][0] =   0; iColor[i][1] =  ~b; iColor[i][2] = 255;
      break;
     case 4: // Blue to magenta
      iColor[i][0] =   b; iColor[i][1] =   0; iColor[i][2] = 255;
      break;
     case 5: // Magenta to red
      iColor[i][0] = 255; iColor[i][1] =   0; iColor[i][2] =  ~b;
      break;
    }
  }
  hue += 7;
  if(hue >= 1536) hue -= 1536;

  // Render current blink (if any) into brightness map
  if(blinkCounter <= blinkFrames * 2) { // In mid-blink?
    if(blinkCounter > blinkFrames) {    // Eye closing
      outer = blinkFrames * 2 - blinkCounter;
      inner = outer + 1;
    } else {                            // Eye opening
      inner = blinkCounter;
      outer = inner - 1;
    }
    y1 = upperLidTop    - (upperLidTop - upperLidBottom) * outer / blinkFrames;
    y2 = upperLidTop    - (upperLidTop - upperLidBottom) * inner / blinkFrames;
    y3 = lowerLidBottom + (lowerLidTop - lowerLidBottom) * inner / blinkFrames;
    y4 = lowerLidBottom + (lowerLidTop - lowerLidBottom) * outer / blinkFrames;
    for(i=0; i<16; i++) {
      y = pgm_read_byte(&yCoord[i]);
      if(y > y1) {        // Above top lid
        iBrightness[i] = 0;
      } else if(y > y2) { // Blur edge of top lid in motion
        iBrightness[i] = brightness * (y1 - y) / (y1 - y2);
      } else if(y > y3) { // In eye
        iBrightness[i] = brightness;
      } else if(y > y4) { // Blur edge of bottom lid in motion
        iBrightness[i] = brightness * (y - y4) / (y3 - y4);
      } else {            // Below bottom lid
        iBrightness[i] = 0;
      }
    }
  } else { // Not in blink -- set all 'on'
    memset(iBrightness, brightness, sizeof(iBrightness));
  }

  if(--blinkCounter == 0) { // Init next blink?
    blinkFrames  = random(4, 8);
    blinkCounter = blinkFrames * 2 + random(5, 180);
  }

  // Calculate current eye movement, possibly init next one
  if(--gazeCounter <= gazeFrames) { // Is pupil in motion?
    ep = newEyePos - eyeMotion * gazeCounter / gazeFrames; // Current pos.
    if(gazeCounter == 0) {                   // Last frame?
      eyePos      = newEyePos;               // Current position = new pos
      newEyePos   = random(16) * 16;         // New pos. (always pixel center)
      eyeMotion   = newEyePos - eyePos;      // Distance to move
      gazeFrames  = random(10, 20);          // Duration of movement
      gazeCounter = random(gazeFrames, 130); // Count to END of next movement
    }
  } else ep = eyePos; // Not moving -- fixed gaze

  // Draw pupil -- 2 pixels wide, but sup-pixel positioning may span 3.
  a = ep >> 4;         // First candidate
  b = (a + 1)  & 0x0F; // 1 pixel CCW of a
  c = (a + 2)  & 0x0F; // 2 pixels CCW of a
  i = ep & 0x0F;       // Fraction of 'c' covered (0-15)
  iBrightness[a] = (iBrightness[a]  *       i ) >> 4;
  iBrightness[b] = 0;
  iBrightness[c] = (iBrightness[c]  * (16 - i)) >> 4;

  // Merge iColor with iBrightness, issue to NeoPixels
  for(i=0; i<16; i++) {
    // First eye
    r = iColor[i][0];            // Initial background RGB color
    g = iColor[i][1];
    b = iColor[i][2];
    a = iBrightness[i] + 1;
    if(a) {
      r = (r * a) >> 8;          // Scale by brightness map
      g = (g * a) >> 8;
      b = (b * a) >> 8;
    }
    pixels.setPixelColor(i,      // Gamma correct and set pixel
      pgm_read_byte(&gamma8[r]),
      pgm_read_byte(&gamma8[g]),
      pgm_read_byte(&gamma8[b]));

    // Second eye uses the same colors, but reflected horizontally.
    // The same brightness map is used, but not reflected (same left/right)
    r = iColor[15 - i][0];
    g = iColor[15 - i][1];
    b = iColor[15 - i][2];
    if(a) {
      r = (r * a) >> 8;
      g = (g * a) >> 8;
      b = (b * a) >> 8;
    }
    pixels.setPixelColor(16 + i,
      pgm_read_byte(&gamma8[r]),
      pgm_read_byte(&gamma8[g]),
      pgm_read_byte(&gamma8[b]));
  }
  pixels.show();

  delay(15);
}

