#if defined(ARDUINO_ARCH_RP2040)// RP2040 specific driver

#include "Adafruit_NeoPixel.h"

bool Adafruit_NeoPixel::rp2040claimPIO(void) {
  // Find a PIO with enough available space in its instruction memory
  pio = pio0;
  if (pio_can_add_program(pio, &ws2812_program)) {
    pio_program_offset = pio_add_program(pio, &ws2812_program);
  } else {
    // ok try pio1
    pio = pio1;
    if (pio_can_add_program(pio, &ws2812_program)) {
      pio_program_offset = pio_add_program(pio, &ws2812_program);
    } else {
      // ok try pio2 (RP2350)
      pio = pio2;
      if (pio_can_add_program(pio, &ws2812_program)) {
        pio_program_offset = pio_add_program(pio, &ws2812_program);
      } else {
        pio = NULL;
        return false;
      }
    }
  }

  // Find a free SM on one of the PIO's
  pio_sm = pio_claim_unused_sm(pio, false);  // don't panic
  if (pio_sm < 0) {
    return false;
  }

  // yay ok!
  
  if (is800KHz) {
    // 800kHz, 8 bit transfers
    ws2812_program_init(pio, pio_sm, pio_program_offset, pin, 800000, 8);
  } else {
    // 400kHz, 8 bit transfers
    ws2812_program_init(pio, pio_sm, pio_program_offset, pin, 400000, 8);
  }

  return true;
}

void Adafruit_NeoPixel::rp2040releasePIO(void) {
  if (pio == NULL) 
    return;

  pio_remove_program(pio, &ws2812_program, pio_program_offset);

  if (pio_sm < 0)
    return;

  pio_sm_set_enabled(pio, pio_sm, false);
  pio_sm_unclaim(pio, pio_sm);
}


// Private, called from show()
void  Adafruit_NeoPixel::rp2040Show(uint8_t *pixels, uint32_t numBytes)
{
  // verify we have a valid PIO and state machine
  if (! pio || (pio_sm < 0)) {
    return;
  }

  while(numBytes--)
    // Bits for transmission must be shifted to top 8 bits
    pio_sm_put_blocking(pio, pio_sm, ((uint32_t)*pixels++)<< 24);
}
#endif
