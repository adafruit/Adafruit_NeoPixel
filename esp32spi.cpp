/*!
 * @file esp32spi.cpp 
 *
 * @mainpage Arduino Library for driving Adafruit NeoPixel addressable LEDs,
 * FLORA RGB Smart Pixels and compatible devicess -- WS2811, WS2812, WS2812B,
 * SK6812, etc.
 * This is SPI implementation for ESP32-C2/C61 as this does not have RMT peripheral
 * But can also be used on ESP32 other variants of ESP32 that have SPI peripheral
 * Then you will need to define ADAFRUIT_SPI before including the Adafruit_NeoPixel library
 *
 * @section author Author
 *
 * Written by Phil "Paint Your Dragon" Burgess for Adafruit Industries,
 * with contributions by PJRC, Michael Miller and other members of the
 * open source community. This SPI implementation was written by Emile van der Laan.
 * 
 * @section license License
 *
 * This file is part of the Adafruit_NeoPixel library.
 *
 * Adafruit_NeoPixel is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Adafruit_NeoPixel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoPixel. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#if defined(ESP32)
#include <Arduino.h>
#endif 

#if defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C61)
#ifndef ADAFRUIT_SPI
#define ADAFRUIT_SPI  
#endif
#endif

#if defined(ADAFRUIT_SPI) 

//#warning "Using SPI implementation for NeoPixels on ESP32 devices"

#if defined(ESP_IDF_VERSION_MAJOR)
#if ESP_IDF_VERSION_MAJOR == 4
#define HAS_ESP_IDF_4
#endif
#if ESP_IDF_VERSION_MAJOR == 5
#define HAS_ESP_IDF_5
#endif
#endif

#ifdef HAS_ESP_IDF_4
#warning "Support for ESP32-C2 in ESP-IDF 4.x is not implemented"
#endif
#define SEMAPHORE_TIMEOUT_MS 50

#ifdef HAS_ESP_IDF_5
#include <SPI.h>
#include "driver/spi_master.h"

static SemaphoreHandle_t show_mutex = NULL;

struct SpiHostUsed {
    spi_host_device_t spi_host;
    bool used;
};

static SpiHostUsed gSpiHostUsed[] = {
    {SPI2_HOST, false},  // in order of preference
#if SOC_SPI_PERIPH_NUM > 2
    {SPI3_HOST, false},
#endif
#if SOC_SPI_PERIPH_NUM > 3
    {SPI4_HOST, false},
#endif
#if SOC_SPI_PERIPH_NUM > 4
    {SPI5_HOST, false},
#endif
    {SPI1_HOST, false},
};

/*!
  @brief   Returns the next available SPI host device 
  @param   None.
  @note    Returns the next available SPI host device by scanning the gSpiHostUsed array for an unused entry, marking it as used, and returning its spi_host value; if none are available, triggers an error and returns SPI_HOST_MAX. This function is static and intended for internal use only.
  @return  spi_host_device_t The next available SPI host device, or SPI_HOST_MAX if none are available.
*/
static spi_host_device_t getNextAvailableSpiHost()
{
    for (int i = 0; i < sizeof(gSpiHostUsed) / sizeof(gSpiHostUsed[0]); i++)
    {
        if (!gSpiHostUsed[i].used)
        {
            gSpiHostUsed[i].used = true;
            return gSpiHostUsed[i].spi_host;
        }
    }
    ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
    return SPI_HOST_MAX;
}

/*!
  @brief   Releases the specified SPI host device 
  @param   None.
  @note    Releases the specified SPI host by marking it as unused in the gSpiHostUsed array; if the host is not found, triggers an error check with ESP_ERR_NOT_FOUND. This function is static and only accessible within the current translation unit.
*/
static void releaseSpiHost(spi_host_device_t spi_host)
{
    for (int i = 0; i < sizeof(gSpiHostUsed) / sizeof(gSpiHostUsed[0]); i++)
    {
        if (gSpiHostUsed[i].spi_host == spi_host)
        {
            gSpiHostUsed[i].used = false;
            return;
        }
    }
    ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
}

/*!
  @brief Encode a single LED color byte to SPI bits (WS2812)
  @param data LED color byte (0-255)
  @param buf Output buffer (must be zeroed, 3 bytes)
  @return Length in bits
  @note Each LED bit → 3 SPI bits at 2.5MHz:
  - Low bit (0): 100 (binary) → ~400ns high, ~800ns low
  - High bit (1): 110 (binary) → ~800ns high, ~400ns low
  Ported directly from Espressif led_strip_spi_dev.cpp    
*/
static uint32_t encodeLeds800hz(uint8_t *pixels, uint8_t MaxPower, uint32_t numBytes, uint8_t *SpiBuffer) {
uint8_t* buf;
    // WS2812 timing via SPI at 2.5MHz (400ns per bit):
    // Each LED bit needs 3 SPI bits:
    //   LED '0' → 100 (1×400ns high + 2×400ns low = 400ns + 800ns)
    //   LED '1' → 110 (2×400ns high + 1×400ns low = 800ns + 400ns)
    //
    // Process byte from MSB to LSB:
    // Byte 0bABCDEFGH → SPI bits: AAA|BBB|CCC|DDD|EEE|FFF|GGG|HHH (24 bits)
    //
    // Output format (3 bytes):
    //   buf[0] = AAABBBCC (bits 7-2 of output)
    //   buf[1] = CDDDEEEF (bits 15-10 of output)
    //   buf[2] = FFGGGHHH (bits 23-18 of output)
    
    for (size_t i = 0; i < numBytes; i++) {
      buf=&SpiBuffer[i * 3];
    // Buffer is zero-initialized, so we only need to set high bits
    // Process each bit from MSB (bit 7) to LSB (bit 0)
    uint8_t LedPixel = (pixels[i] * MaxPower) / 255;
    for (int bit = 7; bit >= 0; bit--) {
          uint8_t pattern = (LedPixel & (1 << bit)) ? 0b110 : 0b100;  // High bit: 110, Low bit: 100

        // Map LED bit position to SPI byte/bit positions
        int spi_bit_offset = (7 - bit) * 3;  // Each LED bit → 3 SPI bits
        int byte_idx = spi_bit_offset / 8;
        int bit_offset = spi_bit_offset % 8;

        // Write 3-bit pattern into output buffer
        if (bit_offset <= 5) {
            // Pattern fits entirely in one byte
            buf[byte_idx] |= (pattern << (5 - bit_offset));
        } else {
            // Pattern spans two bytes
            buf[byte_idx] |= (pattern >> (bit_offset - 5));
            buf[byte_idx + 1] |= (pattern << (13 - bit_offset));
        }
    }
}
    return numBytes * 3 * 8; // Each LED byte → 3 SPI bytes → 24 bits
}

/*!
  @brief Encode a single LED color byte to SPI bits (WS2811)
  @param data LED color byte (0-255)
  @param buf Output buffer (must be zeroed, 10 bytes / LED byte)0
  @return Length in bits
  @note  WS2811 timing via SPI at 4.0MHz (250ns per SPI bit):
    Each LED bit needs 10 SPI bits.
    LED '0' → 1100000000 (2×250ns high + 8×250ns low = 500ns + 2000ns)
    LED '1' → 1111100000 (5×250ns high + 5×250ns low = 1250ns + 1250ns)
*/
uint32_t encodeLeds400hz(uint8_t *pixels, uint8_t MaxPower, uint32_t numBytes, uint8_t *SpiBuffer) {
    uint8_t Highbits=0;
    uint8_t Lowbits=0;
    uint32_t Bitpos=0;
    for (uint32_t i = 0; i < numBytes; i++) {
      uint8_t LedPixel = (pixels[i] * MaxPower) / 255;
      for (uint32_t bit = 0x80; bit; bit >>= 1) {
        if (LedPixel & bit) {
          Highbits =5;
          Lowbits =5;
        }
        else {
          Highbits =2;
          Lowbits =8;
        }
        for(uint8_t h=0;h<Highbits;h++) {
          //SpiBuffer[Bitpos / 8] |= (1 << (7 - (Bitpos % 8)));
          SpiBuffer[Bitpos / 8] |= (0x80 >> (Bitpos & 7));
          Bitpos++;
        }
        Bitpos+=Lowbits;       // Buffer is zero-initialized, so we only need to set the high bits
    }
  }
  return Bitpos;
}

 spi_host_device_t mSpiHost;
spi_device_handle_t mSpiDevice=NULL;
 spi_transaction_t mTransaction;   // SPI transaction descriptor
 spi_bus_config_t bus_config = {};
 spi_device_interface_config_t dev_config = {};
 uint8_t *SpiBuffer=NULL;
 int SpiBufferlen=0;  
uint8_t pinUsed=-1;

/**
 * @brief Deinitialize SPI resources allocated by espShowInit.
 *
 * Frees the SPI device and bus, releases the SPI host, frees the temporary
 * SPI buffer, and resets internal tracking variables.
 */
void espShowDestruct()
  {
  //log_e("espShowDestruct");
  pinUsed = -1;// Reset pin tracking

  if (mSpiDevice) {
    spi_bus_remove_device(mSpiDevice);
    mSpiDevice = NULL;
    spi_bus_free(mSpiHost);
    releaseSpiHost(mSpiHost);
  }

  if (SpiBuffer) {
    free(SpiBuffer);
    SpiBuffer = NULL;
  }

            SpiBufferlen = 0;
  }

/**
 * @brief Initialize SPI resources for SPI NeoPixel output on a given pin.
 *
 * Allocates internal SPI buffer and configures the SPI bus/device for
 * either 800 kHz (WS2812) or 400 kHz (WS2811) timing. If an existing buffer
 * or configuration for a different pin/size exists, it will be deinitialized
 * first by calling `espShowDestruct()`.
 *
 * @param pin GPIO pin number used as MOSI for SPI output.
 * @param numBytes Number of data bytes (typically number of LEDs * 3).
 * @param is800KHz true to configure for WS2812 (800 kHz timing), false for
 *                 WS2811 (400 kHz timing).
 *
 * @note This function selects an available SPI host with
 *       `getNextAvailableSpiHost()` and initializes the bus using
 *       `spi_bus_initialize(..., SPI_DMA_CH_AUTO)`. On failure it logs an
 *       error and calls `ESP_ERROR_CHECK()` which may abort.
 */
void espShowInit(uint8_t pin, uint32_t numBytes, bool is800KHz)
{
  uint32_t Memoryneded = numBytes * 3; // size for 800KHz // WS2812 
  if(!is800KHz) Memoryneded = numBytes * 10; // size for 400KHz // WS2811
  
  if(pinUsed!=pin || SpiBufferlen!=Memoryneded) espShowDestruct();
  if(pinUsed!=pin || SpiBufferlen!=Memoryneded)
  {
    // Allocate SPI buffer (3x LED data for WS2812 encoding) (10x LED data for WS2811 encoding)      
    //log_e("espShowInit pin = %d,numBytes = %d,is800KHz = %d",pin,numBytes,is800KHz);
    
        mSpiHost = getNextAvailableSpiHost();
        if(mSpiHost == SPI_HOST_MAX) {
            log_e("espShow No available SPI host");
            return ;
    }

        // Initialize SPI bus
        bus_config.mosi_io_num = pin;
        bus_config.miso_io_num = -1;  // Not used
        bus_config.sclk_io_num = -1;  // Not used (data-only SPI)
        bus_config.quadwp_io_num = -1;
        bus_config.quadhd_io_num = -1;
    bus_config.max_transfer_sz = 0; // SpiBufferlen // Maximum transfer size, in bytes. Defaults to 4092 if 0 when DMA enabled
    bus_config.data_io_default_level=LOW; ///< Output data IO default level when no transaction.
        bus_config.flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS;
    // the spi_bus_initialize will create a 6 us high pulse on MOSI line at the start of the transaction
    // which can cause issues with some led strips.
        esp_err_t ret = spi_bus_initialize(mSpiHost, &bus_config, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) {
      log_e("SPI bus initialize failed:\n");
            ESP_ERROR_CHECK(ret);
        }

        dev_config.address_bits = 0;
        dev_config.command_bits = 0;
        dev_config.dummy_bits=0;
        dev_config.mode = 0;  // SPI mode 0 
        if(is800KHz)
        dev_config.clock_speed_hz = 2500000;  // 2.5MHz for WS2812
        else 
          dev_config.clock_speed_hz = 4000000;  // 4MHz for WS2811
        dev_config.spics_io_num = -1;  // No CS pin
        dev_config.queue_size = 1;  // Single transaction at a time
        dev_config.flags = SPI_DEVICE_NO_DUMMY;
        ret = spi_bus_add_device(mSpiHost, &dev_config, &mSpiDevice);
        if (ret != ESP_OK) {
            log_e("SPI device add failed: " );
            spi_bus_free(mSpiHost);
            ESP_ERROR_CHECK(ret);
        }

    pinUsed = pin;
    SpiBufferlen = Memoryneded;
  }
}

/*!
  @brief   This will release all the recourses that are used in the espShow_init() function..
  @param   uint8_t pin 
  @param   uint8_t *pixels
  @param   uint32_t numBytes id the number of led * 3
  @param   boolean is800KHz
  @note    espShow sends pixel data to an LED strip using SPI on the specified pin, 
           encoding each byte and managing the SPI transaction with synchronization via a semaphore. 
           It supports both 800KHz and other timing modes, and handles errors during SPI queueing and transaction completion.
*/
  

extern "C" void espShow(uint8_t pin, uint8_t *pixels, uint32_t numBytes, boolean is800KHz, uint32_t maxCurrent, uint32_t CurrentPerLED,boolean Dynamic) {
uint8_t MaxPower=0xff;
uint32_t PowerUsed=0;  
  if(numBytes>0)
  {
    if( CurrentPerLED != 0)
    {
      if(Dynamic) {
        for (uint32_t i = 0; i < numBytes; i++) {
        PowerUsed += (pixels[i] * CurrentPerLED) / 255;
        } 
        if(PowerUsed > maxCurrent) 
          MaxPower = (maxCurrent*0xff)/PowerUsed;
      } else 
      {
        if( CurrentPerLED != 0 && numBytes * CurrentPerLED > maxCurrent) 
          MaxPower = ((maxCurrent*0xff)/numBytes)/CurrentPerLED;
        else 
          MaxPower = 0xff;
      }
    }
    espShowInit( pin,numBytes,is800KHz);         
    for(int attempt=0;attempt<1;attempt++) {
      SpiBuffer = (uint8_t *)malloc(SpiBufferlen); // 3 or 10 bytes per LED byte
      if(SpiBuffer == NULL) {
        SpiBufferlen = 0;
        log_e("espShow malloc failed ");
      }
      else
      {
        // Prepare SPI transaction
        memset(&mTransaction, 0, sizeof(mTransaction));
        // Encode LED buffer to SPI buffer and return the length in bits
        memset(SpiBuffer, 0, SpiBufferlen);     // Buffer is zero-initialized, so we only need to set high bits
    if(is800KHz) {
            mTransaction.length = encodeLeds800hz(pixels, MaxPower, numBytes, SpiBuffer);
    } else {
            mTransaction.length = encodeLeds400hz(pixels, MaxPower, numBytes, SpiBuffer);
    }
        if (show_mutex && xSemaphoreTake(show_mutex, SEMAPHORE_TIMEOUT_MS / portTICK_PERIOD_MS) == pdTRUE) {
    mTransaction.tx_buffer = SpiBuffer;
    mTransaction.rx_buffer = NULL;
        // Configure SPI device
        esp_err_t ret = spi_device_acquire_bus(mSpiDevice, portMAX_DELAY);
    if (ret != ESP_OK) {
                log_e("SPI transaction acquire bus failed: %d", ret);// << esp_err_to_name(ret));
        ESP_ERROR_CHECK(ret);
    }
            ret = spi_device_transmit(mSpiDevice, &mTransaction );
    if (ret != ESP_OK) {
                log_e("SPI transaction queue failed: %d", ret);// << esp_err_to_name(ret));
        ESP_ERROR_CHECK(ret);
    }
            spi_device_release_bus(mSpiDevice);
        xSemaphoreGive(show_mutex);
        }  else {
          log_e("espShow could not obtain mutex");
        }
        if (SpiBuffer) 
        {
            free(SpiBuffer);
            SpiBuffer = NULL;
        }
      }
  }
  } // espShow numBytes is zero"
}

/*!
  @brief   To avoid race condition initializing the mutex.
  @param   None.
  @note    To avoid race condition initializing the mutex, all instances of
           Adafruit_NeoPixel must be constructed before launching and child threads
*/
extern "C" void espInit() {
  if (!show_mutex) {
    show_mutex = xSemaphoreCreateMutex();
  }
}

#endif // ifdef HAS_ESP_IDF_5
#endif // if defined(ADAFRUIT_SPI)