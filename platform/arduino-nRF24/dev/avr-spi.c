#include <avr-spi.h>

/* Prevents interrupts using SPI at inappropriate times */
/*
 * On the Tmote sky access to I2C/SPI/UART0 must always be
 * exclusive. Set spi_busy so that interrupt handlers can check if
 * they are allowed to use the bus or not. 
 * 
 */
unsigned char spi_busy = 0;



void
spi_init()
{
  uint8_t sreg = SREG;cli();
  static uint8_t initialised = 0;
  if (!initialised) {
    // Set SS to high so a connected chip will be "deselected" by default
    digitalWrite(SS, HIGH);
    
    // When the SS pin is set as OUTPUT, it can be used as
    // a general purpose output port (it doesn't influence
    // SPI operations).
    pinMode(SS, OUTPUT);
    
    // Warning: if the SS pin ever becomes a LOW INPUT then SPI
    // automatically switches to Slave, so the data direction of
    // the SS pin MUST be kept as OUTPUT.
    SPCR = BV(MSTR);
    SPCR |= BV(SPE);
    
    // Set direction register for SCK and MOSI pin.
    // MISO pin automatically overrides to INPUT.
    // By doing this AFTER enabling SPI, we avoid accidentally
    // clocking in a single bit since the lines go directly
    // from "input" to SPI control.
    
    /* Initalize ports for communication with SPI units. */
    /* CSN=SS and must be output when master! */
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);

    /* Clock rate FCK / 4 */
    SPSR = 0;//BV(SPI2X);
    
    initialised = 1;
    SREG = sreg;
  }
}
