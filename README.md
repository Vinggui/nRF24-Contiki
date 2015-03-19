nRF24 driver for Contiki-OS running on 328p
==============================
This repository is destined to provide a start for operating radios model **nRF24L01+** inside [Contiki-OS](http://www.contiki-os.org/index.html).  

These files are actually the full plataform for working with Contiki and runing on Atmega328p, either using external crystal 8-16MHz or internal RC clock at 4-8MHz. They were ported to "C" from the last updated repository of [RF24 library](https://github.com/tmrh20/RF24), but haven't still been fully used by me. So I hope this may be of use for someone, and I accept suggestions and fixes, of course.

How to use it
==============================
1. Download the [Arduino IDE](http://arduino.cc/en/Main/Software) inside the VM of contiki;  
2. Go to the main directory of ubuntu running on VM and create the folder "jenkins/jenkins/jobs/toolchain-avr-3.4.3-linux32/workspace/objdir/etc";  
3. Copy the **avrdude.conf** file from the downloaded Arduino IDE "arduino/hardware/tools/avr/etc/avrdude.conf" to the jenkins folder created before;
4. Insert the new arduino-nRF24 plataform folder inside your Contiki-OS folder (E.g. ~contiki-2.7/plataform/);  
5. Go to some Example (or use the "myFirstMSG-nRF24" example uploaded here).  

* Don't forget to check and flash the [Avr FUSE](http://www.engbedded.com/fusecalc/) to the desired cpu clock, or things may not work.

###How to flash Contiki using bootloader
First, check if you are using the correct [bootloader](https://github.com/VGH05T/optiboot) for the intended clock, then just go to your example folder, and run:
```
$ make TARGET=arduino-nRF24 ARDUINO_MODEL=Uno savetarget savearduinotarget
$ make myFirstMSG-nRF24.u
```
You may change the arduino_model to UnoIntern8M, UnoIntern4M or UnoIntern1M, depending on your desired clock.  

PS: If you didn't do the step 2 above correctly, you may face some dificulties or have to use "-C /home/user/Arduino ..." together in your make line, like explained [here](http://iot-neuron-networks.blogspot.com.br/2013/08/install-contiki-os-with-arduino-uno.html).  

###How to flash Contiki using USBasp
Like the bootloader, using USBasp may save you some KBytes of data. So this is another option, although sometimes slower.  
To use it, just do the following code (you may need to add path to the system or go inside the folder of Arduino IDE):  
* To generate a hex file:  
```
$ make TARGET=arduino-nRF24 ARDUINO_MODEL=Uno savetarget savearduinotarget
$ make myFirstMSG-nRF24
$ avr-objcopy myFirstMSG-nRF24.arduino-nRF24 -j .text -j .data -O ihex myFirstMSG-nRF24.hex
```
* Connect the board with PC and copy hex file to the board:
```
$ avrdude -D -p atmega328p -c usbasp -P /dev/ttyUSB1 -b 57600  -U flash:w:myFirstMSG-nRF24.hex:i -v
```

At the end, you may see your output running the Arduino IDE->terminal or using:
```
$ ttylog -V -d /dev/ttyUSB1 -b 9600
```
PS: Remeber to change the port "/dev/ttyUSB1" in cases where it is not correct. just edit the file "Makefile.arduino-nRF24" inside the platform folder.

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

Conclusion
==============================
This is yet an initial work ported from TMRh20. I will post more updates and fixes soon.  
Using this radio inside Contiki-OS will need more work around the drivers, so this is yet a start for someone wishing to use them, as i did.

Ackwoledgement
==============================
Thanks to [THMh20](https://github.com/tmrh20/RF24) and [Maniacbug](https://github.com/maniacbug) for these spectacular library for this radio.
