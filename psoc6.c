#if defined(ARDUINO_ARCH_PSOC6)

#include "cyhal_gpio.h"
#include "cyhal_system.h"
#include "cy_syslib.h"
#include "Arduino.h"
 
 void psoc6_show(uint8_t pin, uint8_t *pixels, uint32_t numBytes, bool is800KHZ)
 {
    if (!pixels) return;
        noInterrupts();    
    GPIO_PRT_Type *base = CYHAL_GET_PORTADDR(mapping_gpio_pin[pin]);
    uint32_t pinNum = CYHAL_GET_PIN(mapping_gpio_pin[pin]);
    if(is800KHZ){
        for (uint16_t i = 0; i < numBytes; i++) {
            uint8_t b = pixels[i];
            for (uint8_t j = 0; j < 8; j++) {
                bool bit = (b & 0x80) != 0;
                b <<= 1;
                switch (bit) {
                    case 1:
                    // Send a 1-bit
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        break;
                    case 0:
                    // Send a 0-bit
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        break;
                }
            }
        }
    }
    else{ 
        for (size_t i = 0; i < numBytes; i++){   
            uint8_t b = pixels[i];
            for (uint8_t j = 0; j < 8; j++) {
                bool bit = (b & 0x80) != 0;
                b <<= 1;
                    switch (bit){
                    case 1:
                    // Send a 1-bit
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        break;
                    case 0:
                    // Send a 0-bit
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 1);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        Cy_GPIO_Write(base, pinNum, 0);
                        break;
                    }
            }
        }
    }
  interrupts();
  delayMicroseconds(50); 
  return;
 }
 #endif 