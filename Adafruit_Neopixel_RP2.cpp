#if defined(ARDUINO_ARCH_RP2040)// RP2040 specific driver

#include "Adafruit_NeoPixel.h"

bool Adafruit_NeoPixel::rp2040claimPIO(void) {
  // Find a PIO with enough available space in its instruction memory
  pio = NULL;

  if (! pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, 
                                                         &pio, &pio_sm, &pio_program_offset, 
                                                         pin, 1, true)) {
    pio = NULL;
    pio_sm = -1;
    pio_program_offset = 0;
    return false; // No PIO available
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

  pio_remove_program_and_unclaim_sm(&ws2812_program, pio, pio_sm,  pio_program_offset);
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
