/*--------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811-based RGB LED
  devices such as Adafruit FLORA RGB Smart Pixels.  Currently handles
  400 and 800 KHz bitstreams on both 8 MHz and 16 MHz ATmega MCUs,
  with LEDs wired for RGB or GRB color order.  8 MHz MCUs provide
  output on PORTB and PORTD, while 16 MHz chips can handle most output
  pins (possible exception with some of the upper PORT registers on
  the Arduino Mega).

  WILL NOT COMPILE OR WORK ON ARDUINO DUE.  Uses inline assembly.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  --------------------------------------------------------------------
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

#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel::Adafruit_NeoPixel(uint16_t n, uint8_t p, uint8_t t) {
  numBytes = n * 3;
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
    type    = t;
    pin     = p;
    port    = portOutputRegister(digitalPinToPort(p));
    pinMask = digitalPinToBitMask(p);
    endTime = 0L;
  } else {
    numLEDs = 0;
  }
}

void Adafruit_NeoPixel::begin(void) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}


#ifdef __arm__
static inline void delayShort(uint32_t) __attribute__((always_inline, unused));
static inline void delayShort(uint32_t num)
{
        asm volatile(
                "L_%=_delayMicroseconds:"               "\n\t"
                "subs   %0, #1"                         "\n\t"
                "bne    L_%=_delayMicroseconds"         "\n"
//#if F_CPU == 48000000
                //"nop"					"\n\t"
//#endif
                : "+r" (num) :
        );
}
#endif // __arm__


void Adafruit_NeoPixel::show(void) {

  if(!numLEDs) return;

  volatile uint16_t
    i   = numBytes; // Loop counter
  volatile uint8_t
   *ptr = pixels,   // Pointer to next byte
    b   = *ptr++,   // Current byte value
    hi,             // PORT w/output bit set high
    lo;             // PORT w/output bit set low

  // Data latch = 50+ microsecond pause in the output stream.
  // Rather than put a delay at the end of the function, the ending
  // time is noted and the function will simply hold off (if needed)
  // on issuing the subsequent round of data until the latch time has
  // elapsed.  This allows the mainline code to start generating the
  // next frame of data rather than stalling for the latch.
  while((micros() - endTime) < 50L);
  // endTime is a private member (rather than global var) so that
  // mutliple instances on different pins can be quickly issued in
  // succession (each instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with
  // any pin, SBI/CBI instructions are eschewed in favor of full
  // PORT writes via the OUT or ST instructions.  It relies on two
  // facts: that peripheral functions (such as PWM) take precedence
  // on output pins, so our PORT-wide writes won't interfere, and
  // that interrupts are globally disabled while data is being issued
  // to the LEDs, so no other code will be accessing the PORT.  The
  // code takes an initial 'snapshot' of the PORT state, computes
  // 'pin high' and 'pin low' values, and writes these back to the
  // PORT register as needed.

  cli(); // Disable interrupts; need 100% focus on instruction timing

#ifdef __AVR__

#if (F_CPU == 8000000UL) // FLORA, Lilypad, Arduino Pro 8 MHz, etc.

  if((type & NEO_SPDMASK) == NEO_KHZ800) { // 800 KHz bitstream

    volatile uint8_t n1, n2 = 0;  // First, next bits out

    // Squeezing an 800 KHz stream out of an 8 MHz chip requires code
    // specific to each PORT register.  At present this is only written
    // to work with pins on PORTD or PORTB, the most likely use case --
    // this covers all the pins on the Adafruit Flora and the bulk of
    // digital pins on the Arduino Pro 8 MHz (keep in mind, this code
    // doesn't even get compiled for 16 MHz boards like the Uno, Mega,
    // Leonardo, etc., so don't bother extending this out of hand).
    // Additional PORTs could be added if you really need them, just
    // duplicate the else and loop and change the PORT.  Each add'l
    // PORT will require about 150(ish) bytes of program space.

    // 10 instruction clocks per bit: HHxxxxxLLL
    // OUT instructions:              ^ ^    ^

    if(port == &PORTD) {

      hi = PORTD |  pinMask;
      lo = hi    & ~pinMask;
      n1 = lo;
      if(b & 0x80) n1 = hi;

      // Dirty trick here: meaningless MULs are used to delay two clock
      // cycles in one instruction word (rather than using two NOPs).
      // This was necessary in order to squeeze the loop down to exactly
      // 64 words -- the maximum possible for a relative branch.

      asm volatile(
       "headD:\n\t"         // Clk  Pseudocode
        // Bit 7:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %3, %4\n\t"   // 1    n2   = lo
        "out  %0, %2\n\t"   // 1    PORT = n1
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 6\n\t"    // 1-2  if(b & 0x40)
         "mov %3, %1\n\t"   // 0-1    n2 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 6:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %2, %4\n\t"   // 1    n1   = lo
        "out  %0, %3\n\t"   // 1    PORT = n2
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 5\n\t"    // 1-2  if(b & 0x20)
         "mov %2, %1\n\t"   // 0-1    n1 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 5:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %3, %4\n\t"   // 1    n2   = lo
        "out  %0, %2\n\t"   // 1    PORT = n1
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 4\n\t"    // 1-2  if(b & 0x10)
         "mov %3, %1\n\t"   // 0-1    n2 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 4:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %2, %4\n\t"   // 1    n1   = lo
        "out  %0, %3\n\t"   // 1    PORT = n2
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 3\n\t"    // 1-2  if(b & 0x08)
         "mov %2, %1\n\t"   // 0-1    n1 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 3:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %3, %4\n\t"   // 1    n2   = lo
        "out  %0, %2\n\t"   // 1    PORT = n1
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 2\n\t"    // 1-2  if(b & 0x04)
         "mov %3, %1\n\t"   // 0-1    n2 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 2:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %2, %4\n\t"   // 1    n1   = lo
        "out  %0, %3\n\t"   // 1    PORT = n2
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 1\n\t"    // 1-2  if(b & 0x02)
         "mov %2, %1\n\t"   // 0-1    n1 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "mul  r0, r0\n\t"   // 2    nop nop
        // Bit 1:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %3, %4\n\t"   // 1    n2   = lo
        "out  %0, %2\n\t"   // 1    PORT = n1
        "mul  r0, r0\n\t"   // 2    nop nop
        "sbrc %5, 0\n\t"    // 1-2  if(b & 0x01)
         "mov %3, %1\n\t"   // 0-1    n2 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "sbiw %6, 1\n\t"    // 2    i--  (dec. but don't act on zero flag yet)
        // Bit 0:
        "out  %0, %1\n\t"   // 1    PORT = hi
        "mov  %2, %4\n\t"   // 1    n1   = lo
        "out  %0, %3\n\t"   // 1    PORT = n2
        "ld   %5, %a7+\n\t" // 2    b = *ptr++
        "sbrc %5, 7\n\t"    // 1-2  if(b & 0x80)
         "mov %2, %1\n\t"   // 0-1    n1 = hi
        "out  %0, %4\n\t"   // 1    PORT = lo
        "brne headD\n"      // 2    while(i) (zero flag determined above)
        ::
        "I" (_SFR_IO_ADDR(PORTD)), // %0
        "r" (hi),                  // %1
        "r" (n1),                  // %2
        "r" (n2),                  // %3
        "r" (lo),                  // %4
        "r" (b),                   // %5
        "w" (i),                   // %6
        "e" (ptr)                  // %a7
      ); // end asm

    } else if(port == &PORTB) {

      // Same as above, just switched to PORTB and stripped of comments.
      hi = PORTB |  pinMask;
      lo = hi    & ~pinMask;
      n1 = lo;
      if(b & 0x80) n1 = hi;
      asm volatile(
       "headB:\n\t"
        "out  %0, %1\n\t"
        "mov  %3, %4\n\t"
        "out  %0, %2\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 6\n\t"
         "mov %3, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %2, %4\n\t"
        "out  %0, %3\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 5\n\t"
         "mov %2, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %3, %4\n\t"
        "out  %0, %2\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 4\n\t"
         "mov %3, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %2, %4\n\t"
        "out  %0, %3\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 3\n\t"
         "mov %2, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %3, %4\n\t"
        "out  %0, %2\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 2\n\t"
         "mov %3, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %2, %4\n\t"
        "out  %0, %3\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 1\n\t"
         "mov %2, %1\n\t"
        "out  %0, %4\n\t"
        "mul  r0, r0\n\t"
        "out  %0, %1\n\t"
        "mov  %3, %4\n\t"
        "out  %0, %2\n\t"
        "mul  r0, r0\n\t"
        "sbrc %5, 0\n\t"
         "mov %3, %1\n\t"
        "out  %0, %4\n\t"
        "sbiw %6, 1\n\t"
        "out  %0, %1\n\t"
        "mov  %2, %4\n\t"
        "out  %0, %3\n\t"
        "ld   %5, %a7+\n\t"
        "sbrc %5, 7\n\t"
         "mov %2, %1\n\t"
        "out  %0, %4\n\t"
        "brne headB\n" :: "I" (_SFR_IO_ADDR(PORTB)), "r" (hi),
          "r" (n1), "r" (n2), "r" (lo), "r" (b), "w" (i), "e" (ptr)
      ); // end asm
    } // endif PORTB
  } // end 800 KHz, see comments later re 'else'

#elif (F_CPU == 16000000UL)

  if((type & NEO_SPDMASK) == NEO_KHZ400) { // 400 KHz bitstream

    // The 400 KHz clock on 16 MHz MCU is the most 'relaxed' version.
    // Unrolling the inner loop for each bit is not necessary.

    // 40 inst. clocks per bit: HHHHHHHHxxxxxxxxxxxxxxxxxxxxxxxxLLLLLLLL
    // ST instructions:         ^       ^                       ^

    volatile uint8_t next, bit;

    hi   = *port |  pinMask;
    lo   = hi    & ~pinMask;
    next = lo;
    bit  = 8;

    asm volatile(
     "head40:\n\t"          // Clk  Pseudocode    (T =  0)
      "st   %a0, %1\n\t"    // 2    PORT = hi     (T =  2)
      "sbrc %2, 7\n\t"      // 1-2  if(b & 128)
       "mov  %4, %1\n\t"    // 0-1   next = hi    (T =  4)
      "mul  r0, r0\n\t"     // 2    nop nop       (T =  6)
      "mul  r0, r0\n\t"     // 2    nop nop       (T =  8)
      "st   %a0, %4\n\t"    // 2    PORT = next   (T = 10)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 12)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 14)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 16)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 18)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 20)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 22)
      "nop\n\t"             // 1    nop           (T = 23)
      "mov  %4, %5\n\t"     // 1    next = lo     (T = 24)
      "dec  %3\n\t"         // 1    bit--         (T = 25)
      "breq nextbyte40\n\t" // 1-2  if(bit == 0)
      "rol  %2\n\t"         // 1    b <<= 1       (T = 27)
      "nop\n\t"             // 1    nop           (T = 28)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 30)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 32)
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 34)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 36)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 38)
      "rjmp head40\n\t"     // 2    -> head40 (next bit out)
     "nextbyte40:\n\t"      //                    (T = 27)
      "ldi  %3, 8\n\t"      // 1    bit = 8       (T = 28)
      "ld   %2, %a6+\n\t"   // 2    b = *ptr++    (T = 30)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 32)
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 34)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 36)
      "sbiw %7, 1\n\t"      // 2    i--           (T = 38)
      "brne head40\n\t"     // 1-2  if(i != 0) -> head40 (next byte)
      ::
      "e" (port),          // %a0
      "r" (hi),            // %1
      "r" (b),             // %2
      "r" (bit),           // %3
      "r" (next),          // %4
      "r" (lo),            // %5
      "e" (ptr),           // %a6
      "w" (i)              // %7
    ); // end asm

  } // See comments later re 'else'

#elif ((F_CPU == 16500000UL) && defined(__AVR_ATtiny85__))
  if((type & NEO_SPDMASK) == NEO_KHZ400) {
    // Empty case.  400 KHz pixels not supported on 16.5 MHz ATtiny85.
  } // See comments later re 'else'
  // 800 KHz pixel support on 16.5 MHz ATtiny is experimental and
  // NOT guaranteed to work.  It's essentially the same loop as the
  // 16 MHz ATmega code...as a result, the timing is slightly off
  // (825 KHz vs 800), but the WS2811 datasheet suggests this is
  // within the allowable margin of error.
#else
 #error "CPU SPEED NOT SUPPORTED"
#endif

  // This bizarre floating 'else' is intentional.  Only one of the above
  // blocks is actually compiled (depending on CPU speed), each with one
  // specific 'if' case for pixel speed.  This block now handles the
  // common alternate case for either: 800 KHz pixels w/16 MHz CPU, or
  // 400 KHz pixels w/8 MHz CPU.  Instruction timing is the same.
  else {

    // Can use nested loop; no need for unrolling.  Very similar to
    // 16MHz/400KHz code above, but with fewer NOPs and different end.

    // 20 inst. clocks per bit: HHHHxxxxxxxxxxxxLLLL
    // ST instructions:         ^   ^           ^

    volatile uint8_t next, bit;

    hi   = *port |  pinMask;
    lo   = hi    & ~pinMask;
    next = lo;
    bit  = 8;

    asm volatile(
     "head20:\n\t"          // Clk  Pseudocode    (T =  0)
      "st   %a0, %1\n\t"    // 2    PORT = hi     (T =  2)
      "sbrc %2, 7\n\t"      // 1-2  if(b & 128)
       "mov  %4, %1\n\t"    // 0-1   next = hi    (T =  4)
      "st   %a0, %4\n\t"    // 2    PORT = next   (T =  6)
      "mov  %4, %5\n\t"     // 1    next = lo     (T =  7)
      "dec  %3\n\t"         // 1    bit--         (T =  8)
      "breq nextbyte20\n\t" // 1-2  if(bit == 0)
      "rol  %2\n\t"         // 1    b <<= 1       (T = 10)
#ifdef __AVR_ATtiny85__
      "nop\n\t"             // 1 ea.
      "nop\n\t"             // No MUL on ATtiny
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
#else
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 12)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 14)
      "mul  r0, r0\n\t"     // 2    nop nop       (T = 16)
#endif
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 18)
      "rjmp head20\n\t"     // 2    -> head20 (next bit out)
     "nextbyte20:\n\t"      //                    (T = 10)
      "nop\n\t"             // 1    nop           (T = 11)
      "ldi  %3, 8\n\t"      // 1    bit = 8       (T = 12)
      "ld   %2, %a6+\n\t"   // 2    b = *ptr++    (T = 14)
      "sbiw %7, 1\n\t"      // 2    i--           (T = 16)
      "st   %a0, %5\n\t"    // 2    PORT = lo     (T = 18)
      "brne head20\n\t"     // 2    if(i != 0) -> head20 (next byte)
      ::
      "e" (port),          // %a0
      "r" (hi),            // %1
      "r" (b),             // %2
      "r" (bit),           // %3
      "r" (next),          // %4
      "r" (lo),            // %5
      "e" (ptr),           // %a6
      "w" (i)              // %7
    ); // end asm

  } // end wacky else (see comment above)

#endif // __AVR__



#ifdef __MK20DX128__ // Teensy 3.0

// This implementation may not be quite perfect, but it seems to work
// reasonably well with an actual 20 LED WS2811 strip.  The timing at
// 48 MHz is off a bit, perhaps due to flash cache misses?  Ideally
// this code should execute from RAM to eliminate slight timing
// differences between flash caches hits and misses.  But it seems to
// quite well.  More testing is needed with longer strips.
#if F_CPU == 96000000
        #define DELAY_800_T0H       2
        #define DELAY_800_T0L       25
        #define DELAY_800_T1H       16
        #define DELAY_800_T1L       11
        #define DELAY_400_T0H       8
        #define DELAY_400_T0L       56
        #define DELAY_400_T1H       33
        #define DELAY_400_T1L       31
#elif F_CPU == 48000000
        #define DELAY_800_T0H       1
        #define DELAY_800_T0L       13
        #define DELAY_800_T1H       8
        #define DELAY_800_T1L       6
        #define DELAY_400_T0H       4
        #define DELAY_400_T0L       29
        #define DELAY_400_T1H       16
        #define DELAY_400_T1L       17
#elif F_CPU == 24000000
        #error "24 MHz not supported, use Tools > CPU Speed at 48 or 96 MHz"
#endif
	uint8_t *p   = pixels;
	uint8_t *end = pixels + numBytes;
	volatile uint8_t *set = portSetRegister(pin);
	volatile uint8_t *clr = portClearRegister(pin);
	if ((type & NEO_SPDMASK) == NEO_KHZ800) { // 800 KHz bitstream
		while (p < end) {
			uint8_t pix = *p++;
			for (int mask = 0x80; mask; mask >>= 1) {
				if (pix & mask) {
					*set = 1;
					delayShort(DELAY_800_T1H);
					*clr = 1;
					delayShort(DELAY_800_T1L);
				} else {
					*set = 1;
					delayShort(DELAY_800_T0H);
					*clr = 1;
					delayShort(DELAY_800_T0L);
				}
			}
		}
	} else { // 400 kHz bitstream
		while (p < end) {
			uint8_t pix = *p++;
			for (int mask = 0x80; mask; mask >>= 1) {
				if (pix & mask) {
					*set = 1;
					delayShort(DELAY_400_T1H);
					*clr = 1;
					delayShort(DELAY_400_T1L);
				} else {
					*set = 1;
					delayShort(DELAY_400_T0H);
					*clr = 1;
					delayShort(DELAY_400_T0L);
				}
			}
		}
	}

#endif // __MK20DX128__ Teensy 3.0

  sei();              // Re-enable interrupts
  endTime = micros(); // Note EOD time for latch on next call
}

// Set pixel color from separate R,G,B components:
void Adafruit_NeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    if((type & NEO_COLMASK) == NEO_GRB) { *p++ = g; *p++ = r; }
    else                                { *p++ = r; *p++ = g; }
    *p = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void Adafruit_NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t *p = &pixels[n * 3];
    if((type & NEO_COLMASK) == NEO_GRB) { *p++ = c >>  8; *p++ = c >> 16; }
    else                                { *p++ = c >> 16; *p++ = c >>  8; }
    *p = c;
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_NeoPixel::getPixelColor(uint16_t n) {

  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    return (uint32_t)(pixels[ofs + 2]) |
      (((type & NEO_COLMASK) == NEO_GRB) ?
        ((uint32_t)(pixels[ofs    ]) <<  8) |
        ((uint32_t)(pixels[ofs + 1]) << 16)
      :
        ((uint32_t)(pixels[ofs    ]) << 16) |
        ((uint32_t)(pixels[ofs + 1]) <<  8) );
  }

  return 0; // Pixel # is out of bounds
}

uint16_t Adafruit_NeoPixel::numPixels(void) {
  return numLEDs;
}
