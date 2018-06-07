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
#include "bsp/eeprom.h"
#include "settings.h"

/** VARIABLES ******************************************************/

//static bool buttonPressed;
//static char buttonMessage[] = "Button pressed.\r\n";
static uint8_t USB_Out_Buffer[CDC_DATA_OUT_EP_SIZE];
static uint8_t RS232_Out_Data[CDC_DATA_IN_EP_SIZE];

unsigned char NextUSBOut;
unsigned char LastRS232Out; // Number of characters in the buffer
unsigned char RS232cp; // current position within the buffer
unsigned char RS232_Out_Data_Rdy = 0;

static bit APP_configModeActive;
static bit APP_configShowConfig;

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
    APP_configModeActive = 0;
    APP_configShowConfig = 0;
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
#define CONFIG_REPORT_STR ("Status:*#*\r\n")

void APP_DeviceCDCEmulatorTasks() {

    if ((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1)) return;

    if (APP_configModeActive) {
        LastRS232Out = getsUSBUSART(RS232_Out_Data, 64); // until the buffer is free.
        while (LastRS232Out) {
            APP_configShowConfig = 1;
            uint8_t cmd = RS232_Out_Data[--LastRS232Out];
            if ((cmd >= '0') && (cmd <= '9') || (cmd == 8)) {
                //Command for changing drive letter
                if (cmd < '1') {
                    cmd = 0;
                }
                FLASH_settings_t settings;
                settings.SIO_driveID = cmd;
                SETTINGS_store(&settings);
            } else {
                if ((cmd == 'u') || (cmd == 'U') || (cmd == ' ')) {
                    EEPROM_start5msTimer();
                    while (!EEPROM_is5msTimerExpired()) {
                        continue;
                    }
                    if ((cmd == ' ') && !EEPROM_isPowered()) {
                        /* Power-up EEPROM */
                        EEPROM_powerOn();
                    } else {
                        /* Stop EEPROM, now all pending writes should be finished */
                        EEPROM_powerOff();
                    }
                } else {
                    if ((cmd == 'b') || (cmd == 'B')) {
                        /* Stop USART */
                        UART_disable();
                        /* Detach USB device */
                        USBModuleDisable();
                        /* Wait for detach detection */
                        __delay_ms(2000);
                        /* Stop EEPROM, now all pending writes should be finished */
                        EEPROM_powerOff();
                        /* Goto bootloader */
                        STATUSbits.nTO = 0; //Signal the bootloader 
                        RESET();
                    } else {
                        if ((cmd == 'w') || (cmd == 'W')) {
                            // Set drive parameters to R/W
                            cmd = 0;
                        } else {
                            if ((cmd == 'r') || (cmd == 'R')) {
                                // Set drive parameters to ReadOnly
                                cmd = 1;
                            } else {
                                //Unknown command
                                continue;
                            }
                        }
                        EEPROM_write(15, &cmd, 1); //Write new Readonly flag to the EEPROM
                    }
                }
            }
        }
        if (USBUSARTIsTxTrfReady() && APP_configShowConfig) {
            APP_configShowConfig = 0;
            memcpy(USB_Out_Buffer, CONFIG_REPORT_STR, sizeof (CONFIG_REPORT_STR) - 1);
            if (SETTINGS_getSIOaddress()) {
                USB_Out_Buffer[7] = SETTINGS_getSIOaddress();
            }
            if (!EEPROM_isPowered()) {
                USB_Out_Buffer[8] = '_';
            }
            if (EEPROM_read(0, RS232_Out_Data, 16)) {
                USB_Out_Buffer[9] = ((RS232_Out_Data[15] & 1) == 1) ? 'R' : 'W';
            }
            putUSBUSART(USB_Out_Buffer, sizeof (CONFIG_REPORT_STR) - 1);
        }
    } else {
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
    APP_configModeActive = 0; /* Assume we do not activate the config mode */
    //Update the baudrate info in the CDC driver
    CDCSetBaudRate(cdc_notice.GetLineCoding.dwDTERate);
    if (cdc_notice.GetLineCoding.dwDTERate <= 1200) {
        /* Set tape speed = 600 baud */
        UART_baud600();
    } else {
        if (cdc_notice.GetLineCoding.dwDTERate <= 19200) {
            /* Set UART to 19200 1x speed */
            UART_baud19200();
        } else {
            if (cdc_notice.GetLineCoding.dwDTERate <= 38400) {
                /* Set UART to 38400 2x speed */
                UART_baud38400();
            } else {
                if (cdc_notice.GetLineCoding.dwDTERate <= 57600) {
                    /* Set UART to 57600 3x speed */
                    UART_baud57600();
                } else {
                    if (cdc_notice.GetLineCoding.dwDTERate >= 115200) {
                        /* Config mode active */
                        APP_configModeActive = 1;
                        APP_configShowConfig = 1; // request displaying current config
                    } else {
                        /* Reset UART to 19200 1x speed */
                        UART_init();
                    }
                }
            }
        }
    }
}
#endif
