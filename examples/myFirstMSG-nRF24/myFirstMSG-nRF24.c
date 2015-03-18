/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Vinicius Guimaraes <vinicius_galvao@msn.com>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "nRF24_driver.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
/* We declare the one processes */
PROCESS(myFirstMSG_nRF24_process, "first msg process");

/* We require the processes to be started automatically */
AUTOSTART_PROCESSES(&myFirstMSG_nRF24_process);
/*---------------------------------------------------------------------------*/
/* Implementation of the first process */
PROCESS_THREAD(myFirstMSG_nRF24_process, ev, data)
{
    // variables are declared static to ensure their values are kept
    // between kernel calls.
    static struct etimer timer;
    static int count = 0;
    uint8_t addresses[][6] = {"1Node","2Node"};

    // any process mustt start with this.
    PROCESS_BEGIN();            

    // set the etimer module to generate an event in one second.
    etimer_set(&timer, CLOCK_CONF_SECOND);
    while (1)
    {
        // wait here for an event to happen
        PROCESS_WAIT_EVENT();

        // if the event is the timer event as expected...
        if(ev == PROCESS_EVENT_TIMER)
        {
            // do the process work
            printf("Hello, world #%i\n", count);
            count ++;
            
            nRF24_driver.init();
            
            nRF24_openWritingPipe(addresses[1]);
            nRF24_openReadingPipe(1,addresses[0]);
    
            nRF24_startListening();
            
            nRF24_printDetails();
            
            // reset the timer so it will generate an other event
            // the exact same time after it expired (periodicity guaranteed)
            //etimer_reset(&timer);
        }

       // and loop
    }
    // any process must end with this, even if it is never reached.
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
