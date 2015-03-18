#include "nRF24_driver.h"
#include "nRF24L01.h"


/**
 * Private variables
 */
uint8_t ce_pin; /**< "Chip Enable" pin, activates the RX or TX role */
uint8_t csn_pin; /**< SPI Chip select */
bool p_variant; /* False for RF24L01 and true for RF24L01P */
uint8_t payload_size; /**< Fixed size of payloads */
bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
uint8_t pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
uint8_t addr_width; /**< The address width to use - 3,4 or 5 bytes. */
uint32_t txRxDelay; /**< Var for adjusting delays depending on datarate */

/**
 * Private functions
 */
  /**
   * @name Low-level internal interface.
   *
   *  Protected methods that address the chip directly.  Regular users cannot
   *  ever call these.  They are documented for completeness and for developers who
   *  may want to extend this class.
   */
  /**@{*/

  /**
   * Set chip select pin
   *
   * Running SPI bus at PI_CLOCK_DIV2 so we don't waste time transferring data
   * and best of all, we make use of the radio's FIFO buffers. A lower speed
   * means we're less likely to effectively leverage our FIFOs and pay a higher
   * AVR runtime cost as toll.
   *
   * @param mode HIGH to take this unit off the SPI bus, LOW to put it on
   */
  void nRF24_csn(bool mode);

  /**
   * Set chip enable
   *
   * @param level HIGH to actively begin transmission or LOW to put in standby.  Please see data sheet
   * for a much more detailed description of this pin.
   */
  void nRF24_ce(bool level);

  /**
   * Read a chunk of data in from a register
   *
   * @param reg Which register. Use constants from nRF24L01.h
   * @param buf Where to put the data
   * @param len How many bytes of data to transfer
   * @return Current value of status register
   */
  uint8_t nRF24_read_register_block(uint8_t reg, uint8_t* buf, uint8_t len);

  /**
   * Read single byte from a register
   *
   * @param reg Which register. Use constants from nRF24L01.h
   * @return Current value of register @p reg
   */
  uint8_t nRF24_read_register(uint8_t reg);

  /**
   * Write a chunk of data to a register
   *
   * @param reg Which register. Use constants from nRF24L01.h
   * @param buf Where to get the data
   * @param len How many bytes of data to transfer
   * @return Current value of status register
   */
  uint8_t nRF24_write_register_block(uint8_t reg, const uint8_t* buf, uint8_t len);

  /**
   * Write a single byte to a register
   *
   * @param reg Which register. Use constants from nRF24L01.h
   * @param value The new value to write
   * @return Current value of status register
   */
  uint8_t nRF24_write_register(uint8_t reg, uint8_t value);

  /**
   * Write the transmit payload
   *
   * The size of data written is the fixed payload size, see nRF24_getPayloadSize()
   *
   * @param buf Where to get the data
   * @param len Number of bytes to be sent
   * @return Current value of status register
   */
  uint8_t nRF24_write_payload(const void* buf, uint8_t len, const uint8_t writeType);

  /**
   * Read the receive payload
   *
   * The size of data read is the fixed payload size, see nRF24_getPayloadSize()
   *
   * @param buf Where to put the data
   * @param len Maximum number of bytes to read
   * @return Current value of status register
   */
  uint8_t nRF24_read_payload(void* buf, uint8_t len);

  /**
   * Empty the receive buffer
   *
   * @return Current value of status register
   */
  uint8_t nRF24_flush_rx(void);

  /**
   * Retrieve the current status of the chip
   *
   * @return Current value of status register
   */
  uint8_t nRF24_get_status(void);

#if !defined (MINIMAL)
  /**
   * Decode and print the given status to stdout
   *
   * @param status Status value to print
   *
   * @warning Does nothing if stdout is not defined.  See fdevopen in stdio.h
   */
  void nRF24_print_status(uint8_t status);

  /**
   * Decode and print the given 'observe_tx' value to stdout
   *
   * @param value The observe_tx value to print
   *
   * @warning Does nothing if stdout is not defined.  See fdevopen in stdio.h
   */
  void nRF24_print_observe_tx(uint8_t value);

  /**
   * Print the name and value of an 8-bit register to stdout
   *
   * Optionally it can print some quantity of successive
   * registers on the same line.  This is useful for printing a group
   * of related registers on one line.
   *
   * @param name Name of the register
   * @param reg Which register. Use constants from nRF24L01.h
   * @param qty How many successive registers to print
   */
  void nRF24_print_byte_register(const char* name, uint8_t reg, uint8_t qty);

  /**
   * Print the name and value of a 40-bit address register to stdout
   *
   * Optionally it can print some quantity of successive
   * registers on the same line.  This is useful for printing a group
   * of related registers on one line.
   *
   * @param name Name of the register
   * @param reg Which register. Use constants from nRF24L01.h
   * @param qty How many successive registers to print
   */
  void nRF24_print_address_register(const char* name, uint8_t reg, uint8_t qty);
#endif /* !defined (MINIMAL) */

  /**
   * Turn on or off the special features of the chip
   *
   * The chip has certain 'features' which are only available when the 'features'
   * are enabled.  See the datasheet for details.
   */
  void nRF24_toggle_features(void);

  /**
   * Built in spi transfer function to simplify repeating code repeating code
   */

  uint8_t nRF24_spiTrans(uint8_t cmd);
  
  #if defined (FAILURE_HANDLING)
	void nRF24_errNotify(void);
  #endif
  
#if !defined (MINIMAL)

static const char rf24_datarate_e_str_0[] PROGMEM = "1MBPS";
static const char rf24_datarate_e_str_1[] PROGMEM = "2MBPS";
static const char rf24_datarate_e_str_2[] PROGMEM = "250KBPS";
static const char * const rf24_datarate_e_str_P[] PROGMEM = {
  rf24_datarate_e_str_0,
  rf24_datarate_e_str_1,
  rf24_datarate_e_str_2,
};
static const char rf24_model_e_str_0[] PROGMEM = "nRF24L01";
static const char rf24_model_e_str_1[] PROGMEM = "nRF24L01+";
static const char * const rf24_model_e_str_P[] PROGMEM = {
  rf24_model_e_str_0,
  rf24_model_e_str_1,
};
static const char rf24_crclength_e_str_0[] PROGMEM = "Disabled";
static const char rf24_crclength_e_str_1[] PROGMEM = "8 bits";
static const char rf24_crclength_e_str_2[] PROGMEM = "16 bits" ;
static const char * const rf24_crclength_e_str_P[] PROGMEM = {
  rf24_crclength_e_str_0,
  rf24_crclength_e_str_1,
  rf24_crclength_e_str_2,
};
static const char rf24_pa_dbm_e_str_0[] PROGMEM = "PA_MIN";
static const char rf24_pa_dbm_e_str_1[] PROGMEM = "PA_LOW";
static const char rf24_pa_dbm_e_str_2[] PROGMEM = "PA_HIGH";
static const char rf24_pa_dbm_e_str_3[] PROGMEM = "PA_MAX";
static const char * const rf24_pa_dbm_e_str_P[] PROGMEM = {
  rf24_pa_dbm_e_str_0,
  rf24_pa_dbm_e_str_1,
  rf24_pa_dbm_e_str_2,
  rf24_pa_dbm_e_str_3,
};
#endif

/****************************************************************************/
 
void
nRF24_csn(bool mode)
{
  // Minimum ideal SPI bus speed is 2x data rate
  // If we assume 2Mbs data rate and 16Mhz clock, a
  // divider of 4 is the minimum we want.
  // CLK:BUS 8Mhz:2Mhz, 16Mhz:4Mhz, or 20Mhz:5Mhz
	spi_init();

	digitalWrite(csn_pin,mode);	


}

/****************************************************************************/

void
nRF24_ce(bool level) {
  if (ce_pin != csn_pin) digitalWrite(ce_pin,level);
}

/****************************************************************************/

uint8_t
nRF24_read_register_block(uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;

  nRF24_csn(LOW);
  status = spi_write_byte( (R_REGISTER | ( REGISTER_MASK & reg )) );

  while ( len-- ){
    *buf++ = spi_write_byte(0xff);
  }
  nRF24_csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t
nRF24_read_register(uint8_t reg)
{
  uint8_t result;
  
  nRF24_csn(LOW);
  spi_write_byte( R_REGISTER | ( REGISTER_MASK & reg ) );
  
  result = spi_write_byte(0xff);
  

  nRF24_csn(HIGH);

  return result;
}


/****************************************************************************/

uint8_t
nRF24_write_register_block(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  nRF24_csn(LOW);
  
  status = spi_write_byte( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- ) {
    spi_write_byte(*buf++);
  }

  nRF24_csn(HIGH);

  return status;
}

/****************************************************************************/


uint8_t
nRF24_write_register(uint8_t reg, uint8_t value)
{
  uint8_t status;

  IF_SERIAL_DEBUG(printf_P(PSTR("write_register(%02x,%02x)\r\n"),reg,value));

  nRF24_csn(LOW);
  status = spi_write_byte( W_REGISTER | ( REGISTER_MASK & reg ) );
  spi_write_byte(value);

  nRF24_csn(HIGH);

  return status;
}

/****************************************************************************/


uint8_t
nRF24_write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
  uint8_t status;
  const uint8_t* current = (const uint8_t*)(buf);

   data_len = rf24_min(data_len, payload_size);
   uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  //printf("[Writing %u bytes %u blanks]",data_len,blank_len);
  IF_SERIAL_DEBUG( printf_P("[Writing %u bytes %u blanks]\n",data_len,blank_len); );
  
  nRF24_csn(LOW);
  
  status = spi_write_byte( writeType );
  while ( data_len-- ) {
    spi_write_byte(*current++);
  }
  while ( blank_len-- ) {
    spi_write_byte(0);
  }
  nRF24_csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t
nRF24_read_payload(void* buf, uint8_t data_len)
{
  uint8_t status;
  uint8_t* current = (uint8_t*)(buf);

  if(data_len > payload_size) data_len = payload_size;
  uint8_t blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  //printf("[Reading %u bytes %u blanks]",data_len,blank_len);

  IF_SERIAL_DEBUG( printf_P("[Reading %u bytes %u blanks]\n",data_len,blank_len); );
  
  nRF24_csn(LOW);
  
  status = spi_write_byte( R_RX_PAYLOAD );
  while ( data_len-- ) {
    *current++ = spi_write_byte(0xFF);
  }
  while ( blank_len-- ) {
    spi_write_byte(0xff);
  }
  nRF24_csn(HIGH);

  return status;
}

/****************************************************************************/

uint8_t
nRF24_flush_rx(void)
{
  return nRF24_spiTrans( FLUSH_RX );
}

/****************************************************************************/

uint8_t
nRF24_flush_tx(void)
{
  return nRF24_spiTrans( FLUSH_TX );
}

/****************************************************************************/

uint8_t
nRF24_spiTrans(uint8_t cmd){

  uint8_t status;
  
  nRF24_csn(LOW);
  status = spi_write_byte( cmd );
  nRF24_csn(HIGH);
  
  return status;
}

/****************************************************************************/

uint8_t
nRF24_get_status(void)
{
  return nRF24_spiTrans(NOP);
}

/****************************************************************************/
#if !defined (MINIMAL)

void
nRF24_printDetails(void)
{
  nRF24_print_status(nRF24_get_status());

  nRF24_print_address_register(PSTR("RX_ADDR_P0-1"),RX_ADDR_P0,2);
  nRF24_print_byte_register(PSTR("RX_ADDR_P2-5"),RX_ADDR_P2,4);
  nRF24_print_address_register(PSTR("TX_ADDR"),TX_ADDR,1);

  nRF24_print_byte_register(PSTR("RX_PW_P0-6"),RX_PW_P0,6);
  nRF24_print_byte_register(PSTR("EN_AA"),EN_AA,1);
  nRF24_print_byte_register(PSTR("EN_RXADDR"),EN_RXADDR,1);
  nRF24_print_byte_register(PSTR("RF_CH"),RF_CH,1);
  nRF24_print_byte_register(PSTR("RF_SETUP"),RF_SETUP,1);
  nRF24_print_byte_register(PSTR("CONFIG"),CONFIG,1);
  nRF24_print_byte_register(PSTR("DYNPD/FEATURE"),DYNPD,2);
  
  printf_P(PSTR("Data Rate\t = %s\r\n"), pgm_read_word(&rf24_datarate_e_str_P[nRF24_getDataRate()]));
  printf_P(PSTR("Model\t\t = %s\r\n"),   pgm_read_word(&rf24_model_e_str_P[nRF24_isPVariant()]));
  printf_P(PSTR("CRC Length\t = %s\r\n"),pgm_read_word(&rf24_crclength_e_str_P[nRF24_getCRCLength()]));
  printf_P(PSTR("PA Power\t = %s\r\n"),  pgm_read_word(&rf24_pa_dbm_e_str_P[nRF24_getPALevel()]));

}

/****************************************************************************/

void
nRF24_print_status(uint8_t status)
{
  printf_P(PSTR("STATUS\t\t = 0x%02x RX_DR=%x TX_DS=%x MAX_RT=%x RX_P_NO=%x TX_FULL=%x\r\n"),
           status,
           (status & _BV(RX_DR))?1:0,
           (status & _BV(TX_DS))?1:0,
           (status & _BV(MAX_RT))?1:0,
           ((status >> RX_P_NO) & 0b111),
           (status & _BV(TX_FULL))?1:0
          );
}

/****************************************************************************/

void
nRF24_print_observe_tx(uint8_t value)
{
  printf_P(PSTR("OBSERVE_TX=%02x: POLS_CNT=%x ARC_CNT=%x\r\n"),
           value,
           (value >> PLOS_CNT) & 0b1111,
           (value >> ARC_CNT) & 0b1111
          );
}

/****************************************************************************/

void
nRF24_print_byte_register(const char* name, uint8_t reg, uint8_t qty)
{
  //char extra_tab = strlen_P(name) < 8 ? '\t' : '\a';
  //printf_P(PSTR(PRIPSTR"\t%c ="),name,extra_tab);

  char extra_tab = strlen_P(name) < 8 ? '\t' : '\a';
  printf_P(PSTR(PRIPSTR"\t%c ="),name,extra_tab);
  
  while (qty--)
    printf_P(PSTR(" 0x%02x"),nRF24_read_register(reg++));
  printf_P(PSTR("\r\n"));
}

/****************************************************************************/

void
nRF24_print_address_register(const char* name, uint8_t reg, uint8_t qty)
{
  char extra_tab = strlen_P(name) < 8 ? '\t' : '\a';
  printf_P(PSTR(PRIPSTR"\t%c ="),name,extra_tab);

  while (qty--)
  {
    uint8_t buffer[addr_width];
    nRF24_read_register_block(reg++,buffer,sizeof buffer);

    printf_P(PSTR(" 0x"));
    uint8_t* bufptr = buffer + sizeof buffer;
    while( --bufptr >= buffer )
      printf_P(PSTR("%02x"),*bufptr);
  }

  printf_P(PSTR("\r\n"));
}

#endif /* !defined (MINIMAL) */
/****************************************************************************/

void
nRF24_setChannel(uint8_t channel)
{
  const uint8_t max_channel = 127;
  nRF24_write_register(RF_CH,rf24_min(channel,max_channel));
}

/****************************************************************************/

void
nRF24_setPayloadSize(uint8_t size)
{
  payload_size = rf24_min(size,32);
}

/****************************************************************************/

uint8_t
nRF24_getPayloadSize(void)
{
  return payload_size;
}

/****************************************************************************/

/****************************************************************************/

void
nRF24_startListening(void)
{
  nRF24_write_register(CONFIG, nRF24_read_register(CONFIG) | _BV(PRIM_RX));
  nRF24_write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );
  nRF24_ce(HIGH);
  // Restore the pipe0 adddress, if exists
  if (pipe0_reading_address[0] > 0){
    nRF24_write_register_block(RX_ADDR_P0, pipe0_reading_address, addr_width);	
  }else{
	nRF24_closeReadingPipe(0);
  }

  // Flush buffers
  //nRF24_flush_rx();
  if(nRF24_read_register(FEATURE) & _BV(EN_ACK_PAY)){
	nRF24_flush_tx();
  }

  // Go!
  //clock_delay_usec(100);
}

/****************************************************************************/
const uint8_t child_pipe_enable[] PROGMEM =
{
  ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

void
nRF24_stopListening(void)
{  
  nRF24_ce(LOW);

    clock_delay_usec(txRxDelay);
  
  if(nRF24_read_register(FEATURE) & _BV(EN_ACK_PAY)){
    clock_delay_usec(txRxDelay); //200
	nRF24_flush_tx();
  }
  //nRF24_flush_rx();
  nRF24_write_register(CONFIG, ( nRF24_read_register(CONFIG) ) & ~_BV(PRIM_RX) );
 
  nRF24_write_register(EN_RXADDR,nRF24_read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[0]))); // Enable RX on pipe0
  
  //clock_delay_usec(100);

}

/****************************************************************************/

int
nRF24_powerDown(void)
{
  nRF24_ce(LOW); // Guarantee CE is low on powerDown
  nRF24_write_register(CONFIG,nRF24_read_register(CONFIG) & ~_BV(PWR_UP));
  return 1;
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
int
nRF24_powerUp(void)
{
   uint8_t cfg = nRF24_read_register(CONFIG);

   // if not powered up then power up and wait for the radio to initialize
   if (!(cfg & _BV(PWR_UP))){
      nRF24_write_register(CONFIG,nRF24_read_register(CONFIG) | _BV(PWR_UP));

      // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
	  // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
	  // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
      clock_delay_msec(5);

   }
   return 1;
}

/******************************************************************/
#if defined (FAILURE_HANDLING)
void
nRF24_errNotify(){
	#if defined (SERIAL_DEBUG)
	  printf_P(PSTR("RF24 HARDWARE FAIL: Radio not responding, verify pin connections, wiring, etc.\r\n"));
	#endif
	#if defined (FAILURE_HANDLING)
	failureDetected = 1;
	#else
	clock_delay_msec(500);
	#endif
}
#endif
/******************************************************************/

//Similar to the previous write, clears the interrupt flags
bool
nRF24_write( const void* buf, uint8_t len, const bool multicast )
{
	//Start Writing
	nRF24_startFastWrite(buf,len,multicast,1);

	//Wait until complete or failed
	#if defined (FAILURE_HANDLING)
		static struct timer t;							  //Get the time that the payload transmission started
    timer_set(&t, 1+(85)*CLOCK_SECOND/1000); //As we may have problem with this conversion, adding 1 will round it up.
	#endif 
	
	while( ! ( nRF24_get_status()  & ( _BV(TX_DS) | _BV(MAX_RT) ))) { 
		#if defined (FAILURE_HANDLING)
			if(timer_expired(&t)){			
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				  return 0;		
				#else
				  clock_delay_msec(100);
				#endif
			}
		#endif
	}
    
	nRF24_ce(LOW);

	uint8_t status = nRF24_write_register(STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  //Max retries exceeded
  if( status & _BV(MAX_RT)){
  	nRF24_flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
  	return 0;
  }
	//TX OK 1 or 0
  return 1;
}

/****************************************************************************/

//For general use, the interrupt flags are not important to clear
bool
nRF24_writeBlocking( const void* buf, uint8_t len, uint32_t timeout )
{
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//This way the FIFO will fill up and allow blocking until packets go through
	//The radio will auto-clear everything in the FIFO as long as CE remains high

	static struct timer t;							  //Get the time that the payload transmission started
  timer_set(&t, 1+(timeout)*CLOCK_SECOND/1000);
  
	while( ( nRF24_get_status()  & ( _BV(TX_FULL) ))) {		  //Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

		if( nRF24_get_status() & _BV(MAX_RT)){					  //If MAX Retries have been reached
			nRF24_reUseTX();										  //Set re-transmit and clear the MAX_RT interrupt flag
			if(timer_expired(&t)){ return 0; }		  //If this payload has exceeded the user-defined timeout, exit and return 0
		}
		#if defined (FAILURE_HANDLING)
			if(timer_expired(&t)){			
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;			
                #endif				
			}
		#endif

  	}

  	//Start Writing
	nRF24_startFastWrite(buf,len,0,1);								  //Write the payload if a buffer is clear

	return 1;												  //Return 1 to indicate successful transmission
}

/****************************************************************************/

void
nRF24_reUseTX(){
		nRF24_write_register(STATUS,_BV(MAX_RT) );			  //Clear max retry flag
		nRF24_spiTrans( REUSE_TX_PL );
		nRF24_ce(LOW);										  //Re-Transfer packet
		nRF24_ce(HIGH);
}

/****************************************************************************/

bool
nRF24_writeFast( const void* buf, uint8_t len, const bool multicast )
{
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//Return 0 so the user can control the retrys and set a timer or failure counter if required
	//The radio will auto-clear everything in the FIFO as long as CE remains high

	#if defined (FAILURE_HANDLING)
		static struct timer t;							  //Get the time that the payload transmission started
    timer_set(&t, 1+(85)*CLOCK_SECOND/1000); //As we may have problem with this conversion, adding 1 will round it up.
	#endif
	
	while( ( nRF24_get_status()  & ( _BV(TX_FULL) ))) {			  //Blocking only if FIFO is full. This will loop and block until TX is successful or fail

		if( nRF24_get_status() & _BV(MAX_RT)){
			//nRF24_reUseTX();										  //Set re-transmit
			nRF24_write_register(STATUS,_BV(MAX_RT) );			  //Clear max retry flag
			return 0;										  //Return 0. The previous payload has been retransmitted
															  //From the user perspective, if you get a 0, just keep trying to send the same payload
		}
		#if defined (FAILURE_HANDLING)
			if(timer_expired(&t)){			
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;							
				#endif
			}
		#endif
  	}
		     //Start Writing
	nRF24_startFastWrite(buf,len,multicast,1);

	return 1;
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void
nRF24_startFastWrite( const void* buf, uint8_t len, const bool multicast, bool startTx){ //TMRh20

	//nRF24_write_payload( buf,len);
	nRF24_write_payload( buf, len,multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD ) ;
	if(startTx){
		nRF24_ce(HIGH);
	}

}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
void
nRF24_startWrite( const void* buf, uint8_t len, const bool multicast ){

  // Send the payload

  //nRF24_write_payload( buf, len );
  nRF24_write_payload( buf, len,multicast? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD ) ;
  nRF24_ce(HIGH);
  nRF24_ce(LOW);


}

/****************************************************************************/

bool
nRF24_rxFifoFull(){
	return nRF24_read_register(FIFO_STATUS) & _BV(RX_FULL);
}
/****************************************************************************/

bool
nRF24_txStandBy(){

  #if defined (FAILURE_HANDLING)
		static struct timer t;							  //Get the time that the payload transmission started
    timer_set(&t, 1+(85)*CLOCK_SECOND/1000); //As we may have problem with this conversion, adding 1 will round it up.
	#endif
	while( ! (nRF24_read_register(FIFO_STATUS) & _BV(TX_EMPTY)) ){
		if( nRF24_get_status() & _BV(MAX_RT)){
			nRF24_write_register(STATUS,_BV(MAX_RT) );
			nRF24_ce(LOW);
			nRF24_flush_tx();    //Non blocking, flush the data
			return 0;
		}
		#if defined (FAILURE_HANDLING) 
			if( timer_expired(&t)){
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;	
				#endif
			}
		#endif
	}

	nRF24_ce(LOW);			   //Set STANDBY-I mode
	return 1;
}

/****************************************************************************/

bool
nRF24_txStandBy_def(uint32_t timeout, bool startTx){

    if(startTx){
	  nRF24_stopListening();
	  nRF24_ce(HIGH);
	}
	static struct timer t;							  //Get the time that the payload transmission started
  timer_set(&t, 1+(timeout+85)*CLOCK_SECOND/1000); //As we may have problem with this conversion, adding 1 will round it up.

	while( ! (nRF24_read_register(FIFO_STATUS) & _BV(TX_EMPTY)) ){
		if( nRF24_get_status() & _BV(MAX_RT)){
			nRF24_write_register(STATUS,_BV(MAX_RT) );
				nRF24_ce(LOW);										  //Set re-transmit
				nRF24_ce(HIGH);
				if(timer_expired(&t)){
					nRF24_ce(LOW); nRF24_flush_tx(); return 0;
				}
		}
		#if defined (FAILURE_HANDLING)
			if( timer_expired(&t)){
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;	
				#endif
			}
		#endif
	}

	
	nRF24_ce(LOW);				   //Set STANDBY-I mode
	return 1;

}

/****************************************************************************/

void
nRF24_maskIRQ(bool tx, bool fail, bool rx){

	nRF24_write_register(CONFIG, ( nRF24_read_register(CONFIG) ) | fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR  );
}

/****************************************************************************/

uint8_t
nRF24_getDynamicPayloadSize(void)
{
  uint8_t result = 0;

  nRF24_csn(LOW);
  spi_write_byte( R_RX_PL_WID );
  result = spi_write_byte(0xff);
  nRF24_csn(HIGH);

  if(result > 32) { nRF24_flush_rx(); clock_delay_msec(2); return 0; }
  return result;
}

/****************************************************************************/

bool
nRF24_available(uint8_t* pipe_num)
{
  if (!( nRF24_read_register(FIFO_STATUS) & _BV(RX_EMPTY) )){

    // If the caller wants the pipe number, include that
    if ( pipe_num ){
	  uint8_t status = nRF24_get_status();
      *pipe_num = ( status >> RX_P_NO ) & 0b111;
  	}
  	return 1;
  }


  return 0;


}

/****************************************************************************/

void
nRF24_read( void* buf, uint8_t len ){

  // Fetch the payload
  nRF24_read_payload( buf, len );

  //Clear the two possible interrupt flags with one command
  nRF24_write_register(STATUS,_BV(RX_DR) | _BV(MAX_RT) | _BV(TX_DS) );

}

/****************************************************************************/

void
nRF24_whatHappened(bool *tx_ok,bool *tx_fail,bool *rx_ready)
{
  // Read the status & reset the status in one easy call
  // Or is that such a good idea?
  uint8_t status = nRF24_write_register(STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  // Report to the user what happened
  *tx_ok = status & _BV(TX_DS);
  *tx_fail = status & _BV(MAX_RT);
  *rx_ready = status & _BV(RX_DR);
}

/****************************************************************************/
void
nRF24_openWritingPipe(const uint8_t *address)
{
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  nRF24_write_register_block(RX_ADDR_P0,address, addr_width);
  nRF24_write_register_block(TX_ADDR, address, addr_width);

  //const uint8_t max_payload_size = 32;
  //nRF24_write_register(RX_PW_P0,rf24_min(payload_size,max_payload_size));
  nRF24_write_register(RX_PW_P0,payload_size);
}

/****************************************************************************/
const uint8_t child_pipe[] PROGMEM =
{
  RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5
};
const uint8_t child_payload_size[] PROGMEM =
{
  RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5
};


/****************************************************************************/
void
nRF24_setAddressWidth(uint8_t a_width){

	if(a_width -= 2){
		nRF24_write_register(SETUP_AW,a_width%4);
		addr_width = (a_width%4) + 2;
	}

}

/****************************************************************************/

void
nRF24_openReadingPipe(uint8_t child, const uint8_t *address)
{
  // If this is pipe 0, cache the address.  This is needed because
  // nRF24_openWritingPipe() will overwrite the pipe 0 address, so
  // nRF24_startListening() will have to restore it.
  if (child == 0){
    memcpy(pipe0_reading_address,address,addr_width);
  }
  if (child <= 6)
  {
    // For pipes 2-5, only write the LSB
    if ( child < 2 ){
      nRF24_write_register_block(pgm_read_byte(&child_pipe[child]), address, addr_width);
    }else{
      nRF24_write_register_block(pgm_read_byte(&child_pipe[child]), address, 1);
	}
    nRF24_write_register(pgm_read_byte(&child_payload_size[child]),payload_size);

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    nRF24_write_register(EN_RXADDR,nRF24_read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child])));

  }
}

/****************************************************************************/

void
nRF24_closeReadingPipe( uint8_t pipe )
{
  nRF24_write_register(EN_RXADDR,nRF24_read_register(EN_RXADDR) & ~_BV(pgm_read_byte(&child_pipe_enable[pipe])));
}

/****************************************************************************/

void
nRF24_toggle_features(void)
{
  nRF24_csn(LOW);
  spi_write_byte( ACTIVATE );
  spi_write_byte( 0x73 );
  nRF24_csn(HIGH);
}

/****************************************************************************/

void
nRF24_enableDynamicPayloads(void)
{
  // Enable dynamic payload throughout the system

    //nRF24_toggle_features();
    nRF24_write_register(FEATURE,nRF24_read_register(FEATURE) | _BV(EN_DPL) );


  IF_SERIAL_DEBUG(printf_P("FEATURE=%i\r\n",nRF24_read_register(FEATURE)));

  // Enable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  nRF24_write_register(DYNPD,nRF24_read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

  dynamic_payloads_enabled = true;
}

/****************************************************************************/

void
nRF24_enableAckPayload(void)
{
  //
  // enable ack payload and dynamic payload features
  //

    //nRF24_toggle_features();
    nRF24_write_register(FEATURE,nRF24_read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL) );

  IF_SERIAL_DEBUG(printf_P("FEATURE=%i\r\n",nRF24_read_register(FEATURE)));

  //
  // Enable dynamic payload on pipes 0 & 1
  //

  nRF24_write_register(DYNPD,nRF24_read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
  dynamic_payloads_enabled = true;
}

/****************************************************************************/

void
nRF24_enableDynamicAck(void){
  //
  // enable dynamic ack features
  //
    //nRF24_toggle_features();
    nRF24_write_register(FEATURE,nRF24_read_register(FEATURE) | _BV(EN_DYN_ACK) );

  IF_SERIAL_DEBUG(printf_P("FEATURE=%i\r\n",nRF24_read_register(FEATURE)));


}

/****************************************************************************/

void
nRF24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
  const uint8_t* current = (const uint8_t*)(buf);

  uint8_t data_len = rf24_min(len,32);

  nRF24_csn(LOW);
  spi_write_byte(W_ACK_PAYLOAD | ( pipe & 0b111 ) );

  while ( data_len-- ) {
    spi_write_byte(*current++);
  }

  nRF24_csn(HIGH);
}

/****************************************************************************/

bool
nRF24_isAckPayloadnRF24_available(void)
{
  return !(nRF24_read_register(FIFO_STATUS) & _BV(RX_EMPTY));
}

/****************************************************************************/

bool
nRF24_isPVariant(void)
{
  return p_variant ;
}

/****************************************************************************/

void nRF24_setAutoAck_AllPipes(bool enable)
{
  if ( enable )
    nRF24_write_register(EN_AA, 0b111111);
  else
    nRF24_write_register(EN_AA, 0);
}

/****************************************************************************/

void
nRF24_setAutoAck( uint8_t pipe, bool enable )
{
  if ( pipe <= 6 )
  {
    uint8_t en_aa = nRF24_read_register( EN_AA ) ;
    if( enable )
    {
      en_aa |= _BV(pipe) ;
    }
    else
    {
      en_aa &= ~_BV(pipe) ;
    }
    nRF24_write_register( EN_AA, en_aa ) ;
  }
}

/****************************************************************************/

bool
nRF24_testCarrier(void)
{
  return ( nRF24_read_register(CD) & 1 );
}

/****************************************************************************/

int
nRF24_testRPD(void)
{
  return ( nRF24_read_register(RPD) & 1 ) ;
}

/****************************************************************************/

void
nRF24_setPALevel(uint8_t level)
{

  uint8_t setup = nRF24_read_register(RF_SETUP) & 0b11111000;

  if(level > 3){  						// If invalid level, go to max PA
	  level = (RF24_PA_MAX << 1) + 1;		// +1 to support the SI24R1 chip extra bit
  }else{
	  level = (level << 1) + 1;	 		// Else set level as requested
  }


  nRF24_write_register( RF_SETUP, setup |= level ) ;	// Write it to the chip
}

/****************************************************************************/

uint8_t
nRF24_getPALevel(void)
{

  return (nRF24_read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1 ;
}

/****************************************************************************/

bool
nRF24_setDataRate(rf24_datarate_e speed)
{
  bool result = false;
  uint8_t setup = nRF24_read_register(RF_SETUP) ;

  // HIGH and LOW '00' is 1Mbs - our default
  setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH)) ;
  
  txRxDelay=85;
  
  if( speed == RF24_250KBPS )
  {
    // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
    // Making it '10'.
    setup |= _BV( RF_DR_LOW ) ;
  
    txRxDelay=155;
  
  }
  else
  {
    // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
    // Making it '01'
    if ( speed == RF24_2MBPS )
    {
      setup |= _BV(RF_DR_HIGH);
      txRxDelay=65;
    }
  }
  nRF24_write_register(RF_SETUP,setup);

  // Verify our result
  if ( nRF24_read_register(RF_SETUP) == setup )
  {
    result = true;
  }

  return result;
}

/****************************************************************************/

bool
nRF24_isValid(void)
{
  return ((ce_pin != 0xff) && (csn_pin != 0xff));
}

/****************************************************************************/

rf24_datarate_e
nRF24_getDataRate( void )
{
  rf24_datarate_e result ;
  uint8_t dr = nRF24_read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

  // switch uses RAM (evil!)
  // Order matters in our case below
  if ( dr == _BV(RF_DR_LOW) )
  {
    // '10' = 250KBPS
    result = RF24_250KBPS ;
  }
  else if ( dr == _BV(RF_DR_HIGH) )
  {
    // '01' = 2MBPS
    result = RF24_2MBPS ;
  }
  else
  {
    // '00' = 1MBPS
    result = RF24_1MBPS ;
  }
  return result ;
}

/****************************************************************************/

void
nRF24_setCRCLength(rf24_crclength_e length)
{
  uint8_t config = nRF24_read_register(CONFIG) & ~( _BV(CRCO) | _BV(EN_CRC)) ;

  // switch uses RAM (evil!)
  if ( length == RF24_CRC_DISABLED )
  {
    // Do nothing, we turned it off above.
  }
  else if ( length == RF24_CRC_8 )
  {
    config |= _BV(EN_CRC);
  }
  else
  {
    config |= _BV(EN_CRC);
    config |= _BV( CRCO );
  }
  nRF24_write_register( CONFIG, config ) ;
}

/****************************************************************************/

rf24_crclength_e
 nRF24_getCRCLength(void)
{
  rf24_crclength_e result = RF24_CRC_DISABLED;
  
  uint8_t config = nRF24_read_register(CONFIG) & ( _BV(CRCO) | _BV(EN_CRC)) ;
  uint8_t AA = nRF24_read_register(EN_AA);
  
  if ( config & _BV(EN_CRC ) || AA)
  {
    if ( config & _BV(CRCO) )
      result = RF24_CRC_16;
    else
      result = RF24_CRC_8;
  }

  return result;
}

/****************************************************************************/

void
nRF24_disableCRC( void )
{
  uint8_t disable = nRF24_read_register(CONFIG) & ~_BV(EN_CRC) ;
  nRF24_write_register( CONFIG, disable ) ;
}

/****************************************************************************/
void
nRF24_setRetries(uint8_t delay, uint8_t count)
{
 nRF24_write_register(SETUP_RETR,(delay&0xf)<<ARD | (count&0xf)<<ARC);
}


/*---------------------------------------------------------------------------*/
int
nRF24_init(void)
{
#ifdef nRF24_CEPIN
  ce_pin = nRF24_CEPIN;
#else
  #error nRF24_CEPIN not defined. Define this at the 'plataform-conf.h' of your chosen plataform.
#endif
#ifdef nRF24_CSPIN
  csn_pin = nRF24_CSPIN;
#else
  #error nRF24_CSPIN not defined. Define this at the 'plataform-conf.h' of your chosen plataform.
#endif
//#ifdef nRF24_SPI_SPEED
//  spi_speed(_spi_speed);
//#else
//  #error nRF24_SPI_SPEED not defined. Define this at the 'plataform-conf.h' of your chosen plataform.
//#endif

#ifdef nRF24_PLUS_MODEL
  p_variant = nRF24_PLUS_MODEL;
#else
  p_variant = false;
  #warning nRF24_PLUS_MODEL not defined. Define this at the 'plataform-conf.h' of your chosen plataform. Using Default = false.
#endif
  
#ifdef nRF24_PAYLOAD 
  payload_size = nRF24_PAYLOAD;
#else
  payload_size = 32;
  #warning nRF24_PAYLOAD not defined. Define this at the 'plataform-conf.h' of your chosen plataform. Using Default = 32.
#endif
#ifdef nRF24_AUTO_PAYLOAD_SIZE
  dynamic_payloads_enabled = nRF24_AUTO_PAYLOAD_SIZE;
#else
  dynamic_payloads_enabled = false;
  #warning nRF24_AUTO_PAYLOAD_SIZE not defined. Define this at the 'plataform-conf.h' of your chosen plataform. Using Default = false.
#endif
#ifdef nRF24_ADRESS_SIZE
  addr_width = nRF24_ADRESS_SIZE;//,pipe0_reading_address(0)
#else
  addr_width = 5;//,pipe0_reading_address(0)
  #warning nRF24_ADRESS_SIZE not defined. Define this at the 'plataform-conf.h' of your chosen plataform. Using Default = 5.
#endif

    // Initialize pins
  pinMode(ce_pin,OUTPUT);
  IF_SERIAL_DEBUG(printf_p(SPRS("CE pin: %d CSN pin: %d"),ce_pin,csn_pin));
  
  pinMode(csn_pin,OUTPUT);
  
  spi_init();
  
  nRF24_ce(LOW);
	nRF24_csn(HIGH);

  // Must allow the radio time to settle else configuration bits will not necessarily stick.
  // This is actually only required following power up but some settling time also appears to
  // be required after resets too. For full coverage, we'll always assume the worst.
  // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
  // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
  // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
  clock_delay_msec( 5 ) ;
  // Reset CONFIG and enable 16-bit CRC.
  nRF24_write_register( CONFIG, 0b00001100 ) ;

  // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
  // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
  // sizes must never be used. See documentation for a more complete explanation.
  nRF24_setRetries(5,15);

  // Reset value is MAX
  //nRF24_setPALevel( RF24_PA_MAX ) ;

  // Determine if this is a p or non-p RF24 module and then
  // reset our data rate back to default value. This works
  // because a non-P variant won't allow the data rate to
  // be set to 250Kbps.
  if( nRF24_setDataRate( RF24_250KBPS ) )
  {
    p_variant = true ;
  }

  // Then set the data rate to the slowest (and most reliable) speed supported by all
  // hardware.
  nRF24_setDataRate( RF24_1MBPS ) ;

  // Initialize CRC and request 2-byte (16bit) CRC
  //nRF24_setCRCLength( RF24_CRC_16 ) ;

  // Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
  nRF24_toggle_features();
  nRF24_write_register(FEATURE,0 );
  nRF24_write_register(DYNPD,0);

  // Reset current status
  // Notice reset and flush is the last thing we do
  nRF24_write_register(STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  nRF24_setChannel(76);

  // Flush buffers
  nRF24_flush_rx();
  nRF24_flush_tx();

  nRF24_powerUp(); //Power up by default when begin() is called

  // Enable PTX, do not write CE high so radio will remain in standby I mode ( 130us max to transition to RX or TX instead of 1500us from powerUp )
  // PTX should use only 22uA of power
  nRF24_write_register(CONFIG, ( nRF24_read_register(CONFIG) ) & ~_BV(PRIM_RX) );
  return 0;
}
/*---------------------------------------------------------------------------*/
int
nRF24_prepare(const void *payload, unsigned short payload_len)
{
  nRF24_stopListening();
  
  //nRF24_write_payload( buf,len);
  return nRF24_write_payload( payload, (uint8_t)payload_len, W_TX_PAYLOAD ) ;
}
/*---------------------------------------------------------------------------*/
int
nRF24_transmit(unsigned short transmit_len)
{
  transmit_len=transmit_len;
  nRF24_ce(HIGH);
  
  //Wait until complete or failed
	#if defined (FAILURE_HANDLING)
		static struct timer t;							  //Get the time that the payload transmission started
    timer_set(&t, 1+(85)*CLOCK_SECOND/1000); //As we may have problem with this conversion, adding 1 will round it up.
	#endif 
	
	while( ! ( nRF24_get_status()  & ( _BV(TX_DS) | _BV(MAX_RT) ))) { 
		#if defined (FAILURE_HANDLING)
			if(timer_expired(&t)){			
				nRF24_errNotify();
				#if defined (FAILURE_HANDLING)
				  return 0;		
				#else
				  clock_delay_msec(100);
				#endif
			}
		#endif
	}
  
  nRF24_ce(LOW);

	uint8_t status = nRF24_write_register(STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  //Max retries exceeded
  if( status & _BV(MAX_RT)){
  	nRF24_flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
  	return RADIO_TX_ERR;
  }
	//TX OK 1 or 0
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
int
nRF24_send(const void *payload, unsigned short payload_len)
{
  //prepare(payload, payload_len);
  //transmit(payload_len);
  return nRF24_write(payload,(uint8_t)payload_len,1);
}
/*---------------------------------------------------------------------------*/
int
nRF24_read_contiki(void *buf, unsigned short buf_len)
{
  nRF24_read(buf,(uint8_t)buf_len);
  return 1;
}
/*---------------------------------------------------------------------------*/
int
nRF24_receiving_packet(void)
{
  nRF24_startListening();
  return 0;
}
/*---------------------------------------------------------------------------*/
int
nRF24_pending_packet(void)
{
  return nRF24_available(NULL);;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver nRF24_driver =
  {
    nRF24_init,
    nRF24_prepare,
    nRF24_transmit,
    nRF24_send,
    nRF24_read_contiki,
    nRF24_testRPD, //channel_clear
    nRF24_receiving_packet,
    nRF24_pending_packet,
    nRF24_powerUp,
    nRF24_powerDown,
  };
/*---------------------------------------------------------------------------*/
