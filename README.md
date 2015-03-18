# nRF24 driver for Contiki-OS running on 328p
This repository is destined to provide a start for operating radios model nRF24L01+ inside Contiki-OS (http://www.contiki-os.org/index.html).
These files are actually the full plataform for working with Contiki runing on Atmega328p, either using external crystal 8-16MHz or internal RC clock at 4-8MHz. They were ported to "C" from the last updated repository of RF24 library (https://github.com/tmrh20/RF24), but haven't still being fully used by me. So I hope this may be of use for someone, and I accept suggestions and fixes, of corse.

#How to use it
  1- Download the Arduino IDE inside the VM of contiki (http://arduino.cc/en/Main/Software);
  2- 
  3- Insert this folder inside your Contiki-os folder (~contiki-2.7/plataform/);
  4- Go to some Example (or use the "myFirstMSG-nRF24" example uploaded here)
  
#Documentation
  Now just search for the funcions you may want to use and program in your Contiki. This driver does not yet have a documentation and better examples to be searched for, but as it is very similar to the original one (RF24-TMRh20 of arduino), you may just search for these informations at http://tmrh20.github.io/RF24/index.html.
  A few modification on functions were made, that are:
    - void nRF24_whatHappened(bool *tx_ok,bool *tx_fail,bool *rx_ready)
      Receives * instead of & for its parameter;
    - int nRF24_testRPD(void)
      Return int instead of bool;
    -int nRF24_powerUP(void) and int nRF24_powerDown(void)
      Return int instead of void;

#How to flash the Contiki

#Conclusion
This is yet an initial work ported from TMRh20. I will post more updates and fixes soon.
Using this radio inside Contiki-OS will need more work around the drivers, so this is yet a start for someone wishing to use them, as i did.

#Ackwoledgement
Thanks to THMh20 and Maniacbug for providing such an expetacular library for this radio.
