//  This sketch is based on the SDK example here:
//  https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812

/**
   Copyright (c) 2020 Raspberry Pi (Trading) Ltd.

   SPDX-License-Identifier: BSD-3-Clause
*/

#if defined(ARDUINO_ARCH_RP2040)

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

typedef struct {
    pin_size_t pin;
    PIO pio;
    int sm;
} Neopixel;

#include "rp2040.pio.h"
static PIOProgram _neopixelPgm(&neopixel_program);

void rp2040Init(uint8_t pin, bool is800KHz)
{
    auto newNeopixel = new Neopixel();
    newNeopixel->pin = pin;
    pinMode(pin, OUTPUT); // Main class has done this already but no harm here

    int offset;
    if (!_neopixelPgm.prepare(&newNeopixel->pio, &newNeopixel->sm, &offset)) {
        DEBUGCORE("ERROR: tone unable to start, out of PIO resources\n");
        // ERROR, no free slots
	      delete newNeopixel;
        return;
    }
    uint offset = pio_add_program(newNeopixel->pio, &ws2812_program);

    if (is800KHz)
    {
        // 800kHz, 8 bit transfers
        ws2812_program_init(newNeopixel->pio, newNeopixel->sm, offset, pin, 800000, 8);
    }
    else
    {
        // 400kHz, 8 bit transfers
        ws2812_program_init(newNeopixel->pio, newNeopixel->sm, offset, pin, 400000, 8);
    }
}
 
void  rp2040Show(uint8_t pin, uint8_t *pixels, uint32_t numBytes, bool is800KHz)
{
    static bool init = true;
    
    if (init)
    {
        // On first pass through initialise the PIO
        rp2040Init(pin, is800KHz);
        init = false;
    }

    while(numBytes--)
        // Bits for transmission must be shifted to top 8 bits
        pio_sm_put_blocking(pio0, 0, ((uint32_t)*pixels++)<< 24);
}

#endif // KENDRYTE_K210
