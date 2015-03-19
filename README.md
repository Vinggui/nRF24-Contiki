nRF24 driver for Contiki-OS running on 328p
==============================
This repository is destined to provide a start for operating radios model **nRF24L01+** inside [Contiki-OS](http://www.contiki-os.org/index.html).  

These files are actually the full plataform for working with Contiki and runing on Atmega328p, either using external crystal 8-16MHz or internal RC clock at 4-8MHz. They were ported to "C" from the last updated repository of [RF24 library](https://github.com/tmrh20/RF24), but haven't still been fully used by me. So I hope this may be of use for someone, and I accept suggestions and fixes, of corse.

#How to use it
1. Download the [Arduino IDE](http://arduino.cc/en/Main/Software) inside the VM of contiki;  
2.   
3. Insert this folder inside your Contiki-OS folder (E.g. ~contiki-2.7/plataform/);  
4. Go to some Example (or use the "myFirstMSG-nRF24" example uploaded here).
  
Documentation
==============================
This driver does not yet have a documentation and better examples to be searched for, but as it is very similar to the original one (RF24-TMRh20 of arduino), you may just search for these informations at their [documentation](http://tmrh20.github.io/RF24/index.html) site.  

###Modifications
All functions now have this **"nRF24_"** prefix, this was used to prevent conflict with other functions inside Contiki.  

The "begin()" function, used to set some parameter and pins, is now done by calling the driver "init()" function, called: **nRF24_init()**  
or by using the driver:  
```
nRF24_driver.init();
```

That way, parameter are now configured by used defines inside the **"plataform-conf.h"**:  

```C
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
```

Besides, a few functions were modified to better work inside Contiki, that are:
* void nRF24_whatHappened(bool *tx_ok,bool *tx_fail,bool *rx_ready)  
Receives * instead of & for its parameter;  
* int nRF24_testRPD(void)  
  Return int instead of bool;  
* int nRF24_powerUP(void) and int nRF24_powerDown(void)  
  Return int instead of void;  

Using the existent SPI driver of Contiki would be possible, but for more compatibility with the original library, another SPI driver was used together with the radio driver (also downloadable inside the ~plataform/arduino-nRF24/dev).

How to flash the Contiki
==============================


Conclusion
==============================
This is yet an initial work ported from TMRh20. I will post more updates and fixes soon.  
Using this radio inside Contiki-OS will need more work around the drivers, so this is yet a start for someone wishing to use them, as i did.

Ackwoledgement
==============================
Thanks to [THMh20](https://github.com/tmrh20/RF24) and [Maniacbug](https://github.com/maniacbug) for these expetacular library for this radio.
