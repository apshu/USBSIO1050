/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

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

#ifndef USART_H
#define USART_H

#include <stdbool.h>
#include <stdint.h>
#include "bsp/pinout.h"

#define CLOCK_FREQ 48000000L
#define GetSystemClock() CLOCK_FREQ

#define UART_ENABLE   RCSTAbits.SPEN

#define UART_TxRdy()    (PIR1bits.TXIF )
#define UART_baud600() do { SPBRG = 0x1F; SPBRGH = 0x4E; /* 48MHz -> 600 baud */ } while(0)
#define UART_baud19200() do { SPBRG = 0x70; SPBRGH = 0x02; /* 48MHz -> 19200 baud */ } while(0)
#define UART_baud38400() do { SPBRG = 0x38; SPBRGH = 0x01; /* 48MHz -> 38400 baud */ } while(0)
#define UART_baud57600() do { SPBRG = 0xCF; SPBRGH = 0x00; /* 48MHz -> 57600 baud */ } while(0)

bit UART_RxRdy(void);
void UART_poll(void);
/*********************************************************************
 * Function: void UART_init(void);
 * Overview: Initializes USART (RS-232 port)
 ********************************************************************/
void UART_init(void);
void UART_enable(void);
void UART_disable(void);

/******************************************************************************
 * Function:        void UART_putch(char c)
 * Input:           char c - character to print to the UART
 * Overview:        Sends one character to the UART
 *****************************************************************************/
void UART_putch(char);
void putch(char c);
/******************************************************************************
 * Function:        char UART_getch(void)
 * Output:          char c - character to received on the UART
 * Overview:        Get the input character from the UART
 *****************************************************************************/
char UART_getch(void);

/******************************************************************************
 * Function:        void UART_baudrateSet(void)
 * Overview:        Changes the serial port baudrate
 *****************************************************************************/
void UART_baudrateSet(uint32_t dwBaud);

#endif //UART_H
