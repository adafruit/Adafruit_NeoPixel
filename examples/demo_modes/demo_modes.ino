// A six-mode showcase for an 8-LED NeoPixel ring, cycling every 6 seconds.
//
// Modes, in cycle order:
//   0. Loading spinner   - trailing comet, for UI busy states
//   1. Analog gauge     - green/yellow/red bar driven by a simulated value
//   2. Fireplace        - flickering warm amber, for ambient mood lighting
//   3. Rainbow wheel    - rotating HSV spectrum
//   4. Emergency strobe - alternating red/blue halves
//   5. Sleep mode       - slow brightness breathe
//
// Every mode is self-contained and needs no hardware beyond the ring itself.
// Wiring: DIN on the ring -> Arduino digital pin 4, with a 330 ohm resistor
// in series. Power the ring separately when drawing more than a few LEDs.

#include <Adafruit_NeoPixel.h>

#define DATA_IN_PIN 4
#define NUM_LEDS 8
#define NUM_MODES 6
#define MODE_DURATION 6000  // Switch modes every 6 seconds

Adafruit_NeoPixel ring(NUM_LEDS, DATA_IN_PIN, NEO_GRB + NEO_KHZ800);

int activeMode = 0;
unsigned long modeStartTime = 0;

void setup() {
  ring.begin();
  ring.show();
  ring.setBrightness(60);  // Controlled baseline brightness
  modeStartTime = millis();
}

void loop() {
  if (millis() - modeStartTime >= MODE_DURATION) {
    activeMode = (activeMode + 1) % NUM_MODES;
    modeStartTime = millis();
    ring.clear();
  }

  switch (activeMode) {
    case 0: runLoadingSpinner(); break;
    case 1: runAnalogDialGauge(); break;
    case 2: runFireplaceFlicker(); break;
    case 3: runColorWheelClock(); break;
    case 4: runEmergencyStrobe(); break;
    case 5: runSleepMode(); break;
  }
}

// MODE 0: smooth loading spinner, a comet tail chasing around the ring
void runLoadingSpinner() {
  int head = (millis() / 40) % NUM_LEDS;  // Spin speed set by time
  ring.clear();
  for (int i = 0; i < 6; i++) {
    int pixel = (head - i + NUM_LEDS) % NUM_LEDS;
    float fade = 1.0 - ((float)i / 6.0);
    ring.setPixelColor(pixel, ring.Color(0, 200 * fade, 255 * fade));
  }
  ring.show();
  delay(10);
}

// MODE 1: analog dial gauge, a bar that grows and shrinks over time
void runAnalogDialGauge() {
  // Simulated sensor reading, normalised 0.0 to 1.0
  float value = (sin(millis() / 800.0) + 1.0) / 2.0;
  int targetLeds = round(value * NUM_LEDS);

  // Thresholds scaled to NUM_LEDS so every band stays reachable at any ring
  // size. With 8 LEDs this gives 4 green, 2 yellow, 2 red.
  int greenEnd = round(NUM_LEDS * 0.50);
  int yellowEnd = round(NUM_LEDS * 0.75);

  ring.clear();
  for (int i = 0; i < targetLeds; i++) {
    if (i < greenEnd) {
      ring.setPixelColor(i, ring.Color(0, 150, 0));    // Green
    } else if (i < yellowEnd) {
      ring.setPixelColor(i, ring.Color(150, 120, 0)); // Yellow
    } else {
      ring.setPixelColor(i, ring.Color(180, 0, 0));   // Red peak
    }
  }
  ring.show();
  delay(20);
}

// MODE 2: cozy fireplace flicker for ambient mood lighting
void runFireplaceFlicker() {
  // random(0, 60) yields 0-59, so these stay in range and never wrap to 255
  for (int i = 0; i < NUM_LEDS; i++) {
    int flicker = random(0, 60);
    ring.setPixelColor(i, ring.Color(200 - flicker, 65 - flicker / 2, 0));
  }
  ring.show();
  delay(random(50, 120));  // Variable frame delay for an organic look
}

// MODE 3: chrono rainbow wheel, a rotating colour gradient
void runColorWheelClock() {
  long hueOffset = (millis() * 5) % 65536;  // Built-in 16-bit HSV

  for (int i = 0; i < NUM_LEDS; i++) {
    // Spatial shift plus time shift creates the orbit
    long hue = hueOffset + (i * 65536L / NUM_LEDS);
    ring.setPixelColor(i, ring.gamma32(ring.ColorHSV(hue)));
  }
  ring.show();
  delay(10);
}

// MODE 4: emergency strobe, alternating halves like an emergency vehicle
void runEmergencyStrobe() {
  int half = NUM_LEDS / 2;
  bool leftSide = (millis() % 1000 < 500);

  ring.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    // Left half goes red, right half goes blue
    if ((i < half) == leftSide) {
      ring.setPixelColor(i, leftSide ? ring.Color(255, 0, 0)
                                     : ring.Color(0, 0, 255));
    }
  }
  ring.show();
  delay(30);
}

// MODE 5: sleep mode, a slow brightness breathe
void runSleepMode() {
  for (int b = 10; b < 120; b += 2) {          // Fade up
    fillRing(ring.Color(0, 150, 255));
    ring.setBrightness(b);
    ring.show();
    delay(15);
  }
  for (int b = 120; b > 10; b -= 2) {          // Fade down
    fillRing(ring.Color(0, 150, 255));
    ring.setBrightness(b);
    ring.show();
    delay(15);
  }
}

// Fill the whole ring instantly
void fillRing(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.setPixelColor(i, color);
  }
}