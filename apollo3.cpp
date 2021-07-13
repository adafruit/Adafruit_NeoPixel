// This provides the functionality for Apollo3 devices.

#if defined(AM_PART_APOLLO3)

#include <ap3_types.h>
#include <am_hal_gpio.h>

#include "Adafruit_NeoPixel.h"

// The timing method used to control the NeoPixels
// TODO: Implement something better (interrupts, DMA, etc)
#define PIN_METHOD_FAST_GPIO

/*!
  @brief   Unset the NeoPixel output pad number.
  @param   pad  Apollo3 pad number
*/
void Adafruit_NeoPixel::apollo3UnsetPad(ap3_gpio_pad_t pad) {
#if defined(PIN_METHOD_FAST_GPIO)
  // Unconfigure the pad for Fast GPIO.
  am_hal_gpio_fastgpio_disable(pad);
#endif
}

/*!
  @brief   Set the NeoPixel output pad number.
  @param   pad  Apollo3 pad number
*/
void Adafruit_NeoPixel::apollo3SetPad(ap3_gpio_pad_t pad) {
#if defined(PIN_METHOD_FAST_GPIO)
  // Configure the pad to be used for Fast GPIO.
  am_hal_gpio_fastgpio_disable(pad);
  am_hal_gpio_fastgpio_clr(pad);

  am_hal_gpio_fast_pinconfig((uint64_t)0x1 << pad,
                             g_AM_HAL_GPIO_OUTPUT, 0);
  // uint32_t ui32Ret = am_hal_gpio_fast_pinconfig((uint64_t)0x1 << pad,
  //                                               g_AM_HAL_GPIO_OUTPUT, 0);
  // if (ui32Ret) {
  //   am_util_stdio_printf(
  //     "Error returned from am_hal_gpio_fast_pinconfig() = .\n", ui32Ret);
  // }
#endif
}

// Note - The timings used below are based on the Arduino Zero,
//   Gemma/Trinket M0 code.

/*!
  @brief   Transmit pixel data in RAM to NeoPixels.
  @note    The current design is a quick hack and should be replaced with
           a more robust timing mechanism.
*/
void Adafruit_NeoPixel::apollo3Show(
  ap3_gpio_pad_t pad, uint8_t *pixels, uint32_t numBytes, boolean is800KHz) {

  uint8_t  *ptr, *end, p, bitMask;
  ptr     =  pixels;
  end     =  ptr + numBytes;
  p       = *ptr++;
  bitMask =  0x80;

#if defined(PIN_METHOD_FAST_GPIO)

  // disable interrupts
  am_hal_interrupt_master_disable();

#ifdef NEO_KHZ400 // 800 KHz check needed only if 400 KHz support enabled
  if(is800KHz) {
#endif
    for(;;) {
      am_hal_gpio_fastgpio_set(pad);
      //asm("nop; nop; nop; nop; nop; nop; nop; nop;");
      asm("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      if(p & bitMask) {
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop;");
        am_hal_gpio_fastgpio_clr(pad);
      } else {
        am_hal_gpio_fastgpio_clr(pad);
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop;");
      }
      if(bitMask >>= 1) {
	asm("nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      } else {
        if(ptr >= end) break;
        p       = *ptr++;
        bitMask = 0x80;
      }
    }
#ifdef NEO_KHZ400
  } else { // 400 KHz bitstream
    // NOTE - These timings may need to be tweaked
    for(;;) {
      am_hal_gpio_fastgpio_set(pad);
      //asm("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      asm("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      if(p & bitMask) {
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop;");
        am_hal_gpio_fastgpio_clr(pad);
      } else {
        am_hal_gpio_fastgpio_clr(pad);
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop;");
      }
      asm("nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;");
      if(bitMask >>= 1) {
        asm("nop; nop; nop; nop; nop; nop; nop;");
      } else {
        if(ptr >= end) break;
        p       = *ptr++;
        bitMask = 0x80;
      }
    }
  }

  // re-enable interrupts
  am_hal_interrupt_master_enable();

#endif // NEO_KHZ400
#endif // PIN_METHOD_FAST_GPIO
}

#endif // AM_PART_APOLLO3
