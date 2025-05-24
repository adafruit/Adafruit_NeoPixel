#if defined(ARDUINO_ARCH_RP2040)// RP2040 specific driver

#include "Adafruit_NeoPixel.h"

bool Adafruit_NeoPixel::rp2040claimPIO(void) {
  // Find a PIO with enough available space in its instruction memory
  pio = NULL;

#ifdef pio2
  // RP2350 - has 3 PIOs
  PIO pios[] = {pio0, pio1, pio2};
#else
  // RP2040 - has 2 PIOs  
  PIO pios[] = {pio0, pio1};
#endif

  uint8_t pio_count = sizeof(pios) / sizeof(pios[0]);

  for (uint8_t i = 0; i < pio_count; i++) {
    if (pio_can_add_program(pios[i], &ws2812_program)) {
        pio = pios[i];
        pio_program_offset = pio_add_program(pio, &ws2812_program);
        break;
    }
  }
  if (pio == NULL) {
    return false; // No PIO available
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
