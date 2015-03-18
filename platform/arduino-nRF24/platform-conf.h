/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: platform-conf.h,v 1.1 2010/06/23 10:25:54 joxe Exp $
 */

/**
 * \file
 *         Platform configuration for Arduino
 * \author
 * 	   Ilya Dmitrichenko <errordeveloper@gmail.com>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

/*
 * Definitions below are dictated by the hardware and not really
 * changeable!
 */
#define PLATFORM PLATFORM_AVR

/*
 * MCU and clock rate.
 */
#if F_CPU == 8000000
 #define MCU_MHZ 8
 #warning "Setting MCU_MHZ value (@8MHz)"
#elif F_CPU == 4000000
 #define MCU_MHZ 4
 #warning "Setting MCU_MHZ value (@4MHz)"
#elif F_CPU == 1000000
 #define MCU_MHZ 1
 #warning "Setting MCU_MHZ value (@1MHz)"
#endif

#ifndef MCU_MHZ
 #define MCU_MHZ 16
 #warning "Setting default MCU_MHZ value (@16MHz)"
#endif

/* Clock ticks per second */
#if defined (__AVR_ATmega328P__) // for Uno and Duemilanove-238
  #if F_CPU == 8000000
   # define CLOCK_CONF_SECOND 128
  #elif F_CPU == 4000000
   # define CLOCK_CONF_SECOND 64
  #elif F_CPU == 1000000
   # define CLOCK_CONF_SECOND 32
  #else
   # define CLOCK_CONF_SECOND 256
  #endif
#else
# define CLOCK_CONF_SECOND 128
#endif

/* LED ports */
#define PLATFORM_HAS_LEDS    1
#if 0 
#define LEDS_PxDIR DDRA /**< port direction register */
#define LEDS_PxOUT PORTA /**< port register */

#define LEDS_CONF_RED    0x04 /**< red led */
#define LEDS_CONF_GREEN  0x02 /**< green led */
#define LEDS_CONF_YELLOW 0x01 /**< yellow led */
#endif

#include "dev/rs232.h"

/* USART port configuration for SLIP */

#ifndef SLIP_PORT
 #ifndef RS232_PORT_0
	#error  "RS232 header was not included ?"
 #else
 #error merda
  #define SLIP_PORT (RS232_PORT_0)
  #warning "Setting default SLIP port (#0)"
 #endif
#endif

#ifndef SLIP_BAUD
 #ifndef USART_BAUD_115200
	#error  "RS232 header was not included ?"
 #else
  #define SLIP_BAUD (USART_BAUD_115200)
  #warning "Setting default SLIP baud rate (@115200)"
 #endif
#endif

/* USART port configuration for serial I/O */

#ifndef USART_PORT
 #ifndef RS232_PORT_0
	#error  "RS232 header was not included ?"
 #else
  #define USART_PORT (RS232_PORT_0)
  #warning "Setting default RS232 I/O port (#0)"
 #endif
#endif

#ifndef USART_BAUD
 #ifndef USART_BAUD_9600 
	#error  "RS232 header was not included ?"
 #else
  #define USART_BAUD (USART_BAUD_9600)
  #warning "Setting default RS232 I/O baud rate (@9600)"
 #endif
#endif

/* Pre-allocated memory for loadable modules heap space (in bytes)*/
#define MMEM_CONF_SIZE 256

/* Use the following address for code received via the codeprop
 * facility
 */
#define EEPROMFS_ADDR_CODEPROP 0x8000

#define EEPROM_NODE_ID_START 0x00


/* #define NETSTACK_CONF_RADIO   cc2420_driver */


/*
 * SPI bus configuration.
 */

/* SPI input/output registers. */
#define SPI_TXBUF SPDR
#define SPI_RXBUF SPDR

#define BV(bitno) _BV(bitno)

#define SPI_WAITFOREOTx() do { while (!(SPSR & BV(SPIF))); } while (0)
#define SPI_WAITFOREORx() do { while (!(SPSR & BV(SPIF))); } while (0)


#define CSN SS

/*
 * nRF24 initial configuration.
 */
#define nRF24_CEPIN               9
#define nRF24_CSPIN               10
#define nRF24_PLUS_MODEL          1 //0 to false, 1 to true
//#define MINIMAL                     //This define makes printDetails function not compiling, and small code size
//#define nRF24_SPI_SPEED           
#define nRF24_PAYLOAD             32
#define nRF24_AUTO_PAYLOAD_SIZE   0 //0 to false, 1 to true
#define nRF24_ADRESS_SIZE         5 //3-5 bytes selectable 
//#define FAILURE_HANDLING          1
//#define SERIAL_DEBUG_NRF24


#endif /* __PLATFORM_CONF_H__ */
