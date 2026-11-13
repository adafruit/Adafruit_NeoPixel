// Uses a potentiometer with a 8-LED NeoPixel to make a 'power dial'.
// As you turn the dial, more LED's light up.
//
// Wiring:
//   Potentiometer outer legs -> 5V and GND, wiper (middle leg) -> A0
//   Ring DIN -> Arduino digital pin 4, through a 330 ohm resistor
//   Ring GND -> Arduino GND (common ground required)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DATA_IN_PIN  4
#define NUM_LEDS     8
#define POT_PIN     A0

Adafruit_NeoPixel ring(NUM_LEDS, DATA_IN_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
#if defined(__AVR__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1);
#endif
  ring.begin();
  ring.show();
  ring.setBrightness(60);
  pinMode(POT_PIN, INPUT);
}

void loop() {
  int val = analogRead(POT_PIN); // Read the dial (0 to 1023)
  ring.clear();

  // Light up each LED based on simple dial benchmarks
  if (val > 50)   ring.setPixelColor(0, ring.Color(0, 255, 0));   // Green
  if (val > 170)  ring.setPixelColor(1, ring.Color(60, 220, 0));
  if (val > 290)  ring.setPixelColor(2, ring.Color(120, 180, 0)); // Yellow-green
  if (val > 410)  ring.setPixelColor(3, ring.Color(180, 140, 0));
  if (val > 530)  ring.setPixelColor(4, ring.Color(220, 90, 0));  // Orange
  if (val > 650)  ring.setPixelColor(5, ring.Color(255, 40, 0));
  if (val > 770)  ring.setPixelColor(6, ring.Color(255, 0, 0));   // Red
  if (val > 890)  ring.setPixelColor(7, ring.Color(255, 0, 100)); // Pink/peak

  ring.show();
  delay(30); // Small delay keeps the signal stable
}
