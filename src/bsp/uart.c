/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

 bsp/uart.c

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
#include <stdbool.h>
#include "usb/usb_config.h"

#include "uart.h"

static uint8_t UART_chin;
static bit UART_isNewChar;

/******************************************************************************
 * Function:        void UART_init(void)
 * Overview:        This routine initializes the UART
 *****************************************************************************/
void UART_init(void) {
    UART_TRISRx = INPUT_PIN;
    UART_TRISTx = OUTPUT_PIN;
    TXSTA = 0x24; // TX enable BRGH=1
    RCSTA = 0x90; // RX enable
    SPBRG = 0xCF;
    SPBRGH = 0x00; // 48MHz -> 19200 baud
#if defined (UART_INVERTED) && UART_INVERTED == 1
    BAUDCON = 0x18; // BRG16 = 1, SCKP = 1 
#else
    BAUDCON = 0x08; // BRG16 = 1
#endif
    char c = RCREG; // read
    UART_enable();
}

void UART_disable(void) {
    RCSTAbits.SPEN = 0; // disable UART, control I/Os
    RCIE = 0;
}

void UART_enable(void) {
    RCSTAbits.SPEN = 1; // enable UART, control I/Os
}

/******************************************************************************
 * Function:        void UART_putch(char c)
 * Input:           char c - character to print to the UART
 * Output:          None
 * Overview:        Print the input character to the UART
 *****************************************************************************/
void UART_putch(char c) {
    while (!UART_TxRdy());
    TXREG = c;
}

void putch(char c) {
    UART_putch(c);
}

/******************************************************************************
 * Function:        void UART_baudrateSet(void)
 * Overview:        Changes the serial port baudrate
 *****************************************************************************/
void UART_baudrateSet(uint32_t dwBaud) {
    uint32_t dwDivider = ((GetSystemClock() / 4) / dwBaud) - 1;
    SPBRG = (uint8_t) dwDivider;
    SPBRGH = (uint8_t) ((uint16_t) (dwDivider >> 8));
}

/******************************************************************************
 * Function:        char UART_getch(void)
 * Output:          unsigned char c - character to received on the UART
 * Overview:        Get a character from the UART
 *****************************************************************************/
char UART_getch(void) {
    return UART_chin;
}

bit UART_RxRdy(void) {
    return UART_isNewChar;
}

void UART_poll(void) {
    if ((UART_isNewChar = PIR1bits.RCIF)) {
        if (RCSTAbits.OERR) // in case of overrun error
        { // we should never see an overrun error, but if we do,
            RCSTAbits.CREN = 0; // reset the port
            UART_chin = RCREG;
            RCSTAbits.CREN = 1; // and keep going.
        } else {
            UART_chin = RCREG;
        }
    }
}

