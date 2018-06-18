/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

 MSD Loader and CDC Interface for the MPLAB XPRESS Evaluation Board

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

 *******************************************************************************/
#include <xc.h>
#include "system.h"     // find release MAJOR/MINOR and DATE in here!!!
#include "bsp/system_config.h"

#include "usb/usb.h"
#include "usb/usb_device_msd.h"
#include "usb/usb_device_cdc.h"

#include "usb/app_device_msd.h"
#include "usb/app_device_cdc.h"
#include "fileio/direct_msd.h"
#include "sio.h"
#include "bsp/eeprom.h"
#include "bsp/uart.h"

/********************************************************************
 * Function:        void main(void)
 *******************************************************************/
MAIN_RETURN main(void) {
    SYSTEM_init();
    while (1) {
        SYSTEM_Tasks();
        UART_poll();
        SIO_task();

#if defined(USB_POLLING)
        USBDeviceTasks();
#endif
        /* If the USB device isn't configured yet, we can't really do anything
         * else since we don't have a host to talk to.  So jump back to the
         * top of the while loop. */
        if (USBGetDeviceState() < CONFIGURED_STATE) { /* USB connection not available or not yet complete */
            // implement nMCLR button
            continue; /* Jump back to the top of the while loop. */
        }

        /* If we are currently suspended, then we need to see if we need to
         * issue a remote wakeup.  In either case, we shouldn't process any
         * keyboard commands since we aren't currently communicating to the host
         * thus just continue back to the start of the while loop. */
        if (USBIsDeviceSuspended() == true) {
            continue; // jump back to the top of the loop
        }

        //Application specific tasks
        MSDTasks();
        APP_DeviceCDCEmulatorTasks();

    }//end while
}//end main

/*******************************************************************************
 End of File
 */

