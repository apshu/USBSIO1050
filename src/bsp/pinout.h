/*
 * File:   pinout.h
 *
 * abstract pinout assignements for XPRESS series
 */

#ifndef PINOUT_H
#define	PINOUT_H

#define _XTAL_FREQ          48000000L

#define INPUT_PIN           1
#define OUTPUT_PIN          0

//Net to pinlist definitions
 
#if (defined(_16F1454)) || defined(_16F1455))
#include "bsp/pinout_p16f1454_p16f1455.h"
#elif defined(_16F1459)
#include "bsp/pinout_p16f1459.h"
#else
#error "Processor not supported"
#endif

#define UART_TRISTx         U1TX_TRIS
#define UART_TRISRx         U1RX_TRIS
#define UART_Tx             U1TX_OUT
#define UART_Rx             U1RX_IN
#define UART_INVERTED       0
#define UART_DSR            MOTOR_IN
#define USB_CDC_DSR_ACTIVE_LEVEL 1
#define UART_CD             UART_DSR
#define USB_CDC_CD_ACTIVE_LEVEL USB_CDC_DSR_ACTIVE_LEVEL 
#define UART_RI             COMMAND_IN
#define USB_CDC_RI_ACTIVE_LEVEL 1
#define mInitDSRPin()       do {  } while (0)
#define mInitRIPin()        do {  } while (0)
#define mInitCDPin()        do {  } while (0)
#define COMMAND_isActive()  ( !COMMAND_IN )
#define MOTOR_isActive()    ( !MOTOR_IN )
#define HOSTATR_isReady()   ( READY_IN )

#define EEPROM_powerOff()   do { EEPROM_PWRPIN_OUT = 0; EEPROM_PWRPIN_TRIS = OUTPUT_PIN; } while (0)
#define EEPROM_powerOn()    do { EEPROM_PWRPIN_OUT = 1; EEPROM_PWRPIN_TRIS = OUTPUT_PIN; } while (0)
#define EEPROM_isPowered()  (EEPROM_PWRPIN_IN)

#endif	/* PINOUT_H */

