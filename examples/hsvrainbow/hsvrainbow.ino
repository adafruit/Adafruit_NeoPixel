// NeoPixel Rainbow example using HSV color-space control
//
// (C) 2016 B. Stultiens
//
// Released under the GPLv3 license to match the rest of the AdaFruit NeoPixel
// library.
//
#include "Adafruit_NeoPixel.h"

#define PIN_STRIP	6	// Pin for the strip of pixels
#define NUMPIXELS	30	// Number of pixels connected

#define INTERVAL	10	// 10ms update interval

// Dynamic change and rate-of-change of value and color saturation. Set these
// to zero (0) if you just want to see the rainbow in full color and no other
// effects.
#define VCHANGE		(-3)	// Value (intensity) change
#define SCHANGE		(-1)	// Color saturation change

#define HCHANGE		7	// Speed of the rainbow changes
#define HUE_REVERSE_CNT	1000	// Rainbow direction reversal interval

// Setup the NeoPixel library with the number of pixels, which pin to send the
// data to and the type of LED to use. See also other examples.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_STRIP, NEO_GRB + NEO_KHZ800);

static int16_t hue;		// Hue angle counter
static uint8_t val;		// Current value
static uint8_t sat;		// Current saturation
static int8_t  hdir;		// Direction of the hue changes
static int8_t  vdir;		// Direction of the value changes
static int8_t  sdir;		// Direction of the saturation changes
static uint8_t prevtime;	// Last time an update occurred
static uint16_t hcnt;		// Hue reversal interval counter

void setup()
{
	hue = NEO_HSV_HUE_MIN;	// Start at Hue = 0 (red)
	val = NEO_HSV_VAL_MAX;	// Start at maximum intensity
	sat = NEO_HSV_SAT_MAX;	// Start at full color
	hdir = HCHANGE;		// Direction and magnitude of the hue changes
	vdir = VCHANGE;		// Direction and magnitude of the value changes
	sdir = SCHANGE;		// Direction and magnitude of the saturation changes

	pixels.begin();		// Init the pixels
	prevtime = millis();	// Record current time for interval timing
	hcnt = 0;
}

void loop()
{
	uint8_t now = millis();
	uint8_t tdiff = now - prevtime;

	if(tdiff >= INTERVAL) {
		prevtime = now;		// Change color every [interval] ms

		// Increase global hue to circle and wrap
		// This will create a cycling rainbow effect
		hue += hdir;
		if(hue > NEO_HSV_HUE_MAX)	// Wrap circle
			hue -= NEO_HSV_HUE_MAX;
		else if(hue < NEO_HSV_HUE_MIN)	// Wrap circle
			hue += NEO_HSV_HUE_MAX;

		// Reverse the rainbow direction
		if(++hcnt >= HUE_REVERSE_CNT) {
			hdir = -hdir;
			hcnt = 0;
		}

		// Vary value between 1/3 and 3/3 of NEO_HSV_VAL_MAX
		// This changes the overall intensity periodically
		val += vdir;
		if(val < NEO_HSV_VAL_MAX / 3 || val == NEO_HSV_VAL_MAX)
			vdir = -vdir;	// Reverse value direction

		// Vary saturation between 2/3 and 3/3 (full) color
		// This makes colors appear periodically bright or pale
		sat += sdir;
		if(sat == NEO_HSV_SAT_MAX * 2 / 3 || sat == NEO_HSV_SAT_MAX)
			sdir = -sdir;	// Reverse saturation direction

		uint16_t h = hue;	// The global hue is the starting point of the rainbow

		for(uint8_t i = 0; i < NUMPIXELS; i++) {
			// Set pixel at fully saturated color with current value
			pixels.setPixelColorHsv(i, h, sat, val);

			// Increase local hue for each pixel to create the rainbow
			h += 27;
			if(h > NEO_HSV_HUE_MAX)
				h -= NEO_HSV_HUE_MAX;
		}

		// Show the new rainbow
		pixels.show();
	}
}
