/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

 CDC Device Implementation

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

/** INCLUDES *******************************************************/
#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "system.h"
#include "usb.h"
#include "usb_device_cdc.h"

#include "app_device_cdc.h"
#include "usb_config.h"
#include "bsp/uart.h"

/** VARIABLES ******************************************************/

//static bool buttonPressed;
//static char buttonMessage[] = "Button pressed.\r\n";
static uint8_t USB_Out_Buffer[CDC_DATA_OUT_EP_SIZE];
static uint8_t RS232_Out_Data[CDC_DATA_IN_EP_SIZE];

unsigned char NextUSBOut;
unsigned char LastRS232Out; // Number of characters in the buffer
unsigned char RS232cp; // current position within the buffer
unsigned char RS232_Out_Data_Rdy = 0;

/*********************************************************************
 * Function: void APP_DeviceCDCEmulatorInitialize(void);
 * Overview: Initializes the demo code
 ********************************************************************/
void APP_DeviceCDCEmulatorInitialize() {
    CDCInitEP();
    line_coding.bCharFormat = 0;
    line_coding.bDataBits = 8;
    line_coding.bParityType = 0;
    line_coding.dwDTERate = 19200UL;

    UART_init();

    // 	 Initialize the arrays !!!
    unsigned char i;
    for (i = 0; i<sizeof (USB_Out_Buffer); i++) {
        USB_Out_Buffer[i] = 0;
    }

    NextUSBOut = 0;
    LastRS232Out = 0;
}

/*********************************************************************
 * Function: void APP_DeviceCDCEmulatorTasks(void);
 *
 * Overview: Keeps the demo running.
 *
 * PreCondition: The demo should have been initialized and started via
 *   the APP_DeviceCDCEmulatorInitialize() and APP_DeviceCDCEmulatorStart() demos
 *   respectively.
 ********************************************************************/
void APP_DeviceCDCEmulatorTasks() {

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) return;

    if (RS232_Out_Data_Rdy == 0) // only check for new USB buffer if the old RS232 buffer is
    { // empty.  This will cause additional USB packets to be NAK'd
        LastRS232Out = getsUSBUSART(RS232_Out_Data, 64); // until the buffer is free.
        if (LastRS232Out > 0) {
            RS232_Out_Data_Rdy = 1; // signal buffer full
            RS232cp = 0; // Reset the current position
        }
    }

    //    Check if one or more bytes are waiting in the physical UART transmit
    //    queue.  If so, send it out the UART TX pin.
    if (RS232_Out_Data_Rdy && UART_TxRdy()) {
#if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
        //Make sure the receiving UART device is ready to receive data before
        //actually sending it.
        if (UART_CTS == USB_CDC_CTS_ACTIVE_LEVEL) {
            UART_putc(RS232_Out_Data[RS232cp]);
            ++RS232cp;
            if (RS232cp == LastRS232Out)
                RS232_Out_Data_Rdy = 0;
        }
#else
        //Hardware flow control not being used.  Just send the data.
        UART_putch(RS232_Out_Data[RS232cp]);
        ++RS232cp;
        if (RS232cp == LastRS232Out)
            RS232_Out_Data_Rdy = 0;
#endif
    }

    //Check if we received a character over the physical UART, and we need
    //to buffer it up for eventual transmission to the USB host.
    if (UART_RxRdy() && (NextUSBOut < (CDC_DATA_OUT_EP_SIZE - 1))) {
        USB_Out_Buffer[NextUSBOut] = UART_getch();
        ++NextUSBOut;
        USB_Out_Buffer[NextUSBOut] = 0;
    }

    //Check if any bytes are waiting in the queue to send to the USB host.
    //If any bytes are waiting, and the endpoint is available, prepare to
    //send the USB packet to the host.

    if (USBUSARTIsTxTrfReady()) {
        if (NextUSBOut > 0) {
            putUSBUSART(&USB_Out_Buffer[0], NextUSBOut);
            NextUSBOut = 0;
        } else {
            
        }
    }

    CDCTxService();
}

/******************************************************************************
 * Function:        void APP_mySetLineCodingHandler(void)
 * PreCondition:    USB_CDC_SET_LINE_CODING_HANDLER is defined
 * Overview:        This function gets called when a SetLineCoding command
 *                  is sent on the bus.  This function will evaluate the request
 *                  and determine if the application should update the baudrate
 *                  or not.
 *****************************************************************************/
#if defined(USB_CDC_SET_LINE_CODING_HANDLER)

void APP_SetLineCodingHandler(void) {
    //Update the baudrate info in the CDC driver
    CDCSetBaudRate(cdc_notice.GetLineCoding.dwDTERate);
    if (cdc_notice.GetLineCoding.dwDTERate <= 4800) {
        /* Force bootloader */
    } else {
        if (cdc_notice.GetLineCoding.dwDTERate <= 115200) {
            /* Reset analyzer device to force autodetection */
        }
    }
}
#endif
