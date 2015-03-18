#ifndef __AVR_SPI_H__
#define __AVR_SPI_H__
#include <stdint.h>
#include <Arduino.h>
#include <dev/spi.h>
#include "contiki-conf.h"

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

// Write to the SPI bus (MOSI pin) and also receive (MISO pin)
inline static uint8_t spi_write_byte(uint8_t data) {
  spi_busy = 1;
  
  SPDR = data;
  /*
   * The following NOP introduces a small delay that can prevent the wait
   * loop form iterating when running at the maximum speed. This gives
   * about 10% more speed, even if it seems counter-intuitive. At lower
   * speeds it is unnoticed.
   */
  asm volatile("nop");
  while (!(SPSR & _BV(SPIF))) ; // wait
  
  spi_busy = 0;
  return SPDR;
}

inline static void spi_write_block(void *buf, size_t count) {
  if (count == 0) return;
  spi_busy = 1;
  uint8_t *p = (uint8_t *)buf;
  SPDR = *p;
  while (--count > 0) {
    uint8_t out = *(p + 1);
    while (!(SPSR & _BV(SPIF))) ;
    uint8_t in = SPDR;
    SPDR = out;
    *p++ = in;
  }
  while (!(SPSR & _BV(SPIF))) ;
  *p = SPDR;
  spi_busy = 0;
}

inline static void setBitOrder(uint8_t bitOrder) {
  if (bitOrder == LSBFIRST) SPCR |= _BV(DORD);
  else SPCR &= ~(_BV(DORD));
}

inline static void setDataMode(uint8_t dataMode) {
  SPCR = (SPCR & ~SPI_MODE_MASK) | dataMode;
}

inline static void setClockDivider(uint8_t clockDiv) {
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (clockDiv & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((clockDiv >> 2) & SPI_2XCLOCK_MASK);
}
#endif /* __AVR_SPI_H__ */
