/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

System Initialization

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
#include "system.h"
#include "bsp/system_config.h"
#include "usb/usb.h"
#include "fileio/fileio.h"
#include "bsp/uart.h"
#include "bsp/eeprom.h"

/*********************************************************************
 * Function: void SYSTEM_Initialize( SYSTEM_STATE state )
 *
 * Overview: Initializes the system.
 *
 * PreCondition: None
 *
 * Input:  SYSTEM_STATE - the state to initialize the system into
 *
 * Output: None
 *
 ********************************************************************/
void SYSTEM_init(void) {
    OSCCONbits.IRCF = 0xf; // 16 MHz HFINTOSC 3x PLL = 48MHz
    ACTCON = 0x90; // ACT enabled, FS-USB mode
    mConfigureIOpins();
    UART_init();
    EEPROM_init();
    USBDeviceInit(); //usb_device.c.  Initializes USB module SFRs and firmware
    //variables to known states.
}

#if defined(USB_INTERRUPT)
void interrupt SYS_InterruptHigh(void) {
    USBDeviceTasks();
}
#endif
