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
#define SDA_IN              PORTCbits.RC1
#define SCL_IN              PORTCbits.RC0
#define SDA_OUT             LATCbits.LATC1
#define SCL_OUT             LATCbits.LATC0
#define SDA_TRIS            TRISCbits.TRISC1
#define SCL_TRIS            TRISCbits.TRISC0
#define U1RX_IN             PORTCbits.RC5
#define U1RX_OUT            LATCbits.LATC5
#define U1RX_TRIS           TRISCbits.TRISC5
#define U1TX_IN             PORTCbits.RC4
#define U1TX_OUT            LATCbits.LATC4
#define U1TX_TRIS           TRISCbits.TRISC4
#define COMMAND_IN          /*TOTO:define pin*/ PORTCbits.RC3
#define COMMAND_OUT         /*TOTO:define pin*/ do { #error "COMMAND pin is input only!" } while(0)
#define COMMAND_TRIS        /*TOTO:define pin*/ TRISCbits.TRISC3
#define MOTOR_IN            /*TOTO:define pin*/ PORTCbits.RC2
#define MOTOR_OUT           /*TOTO:define pin*/ do { #error "MOTOR pin is input only!" } while(0)
#define MOTOR_TRIS          /*TOTO:define pin*/ TRISCbits.TRISC2
#define EEPROM_PWRPIN_IN    /*TOTO:define pin*/ PORTCbits.RC4
#define EEPROM_PWRPIN_OUT   /*TOTO:define pin*/ LATCbits.LATC4
#define EEPROM_PWRPIN_TRIS  /*TOTO:define pin*/ TRISCbits.TRISC4
#define configurePullups()  do { nWPUEN = 0; WPUA = 0xFF; } while(0)

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
#define COMMAND_isActive()  ( !COMMAND_IN )
#define MOTOR_isActive()    ( !MOTOR_IN )

#define EEPROM_powerOff()   do { EEPROM_PWRPIN_OUT = 0; EEPROM_PWRPIN_TRIS = OUTPUT_PIN; } while (0)
#define EEPROM_powerOn()    do { EEPROM_PWRPIN_OUT = 1; EEPROM_PWRPIN_TRIS = OUTPUT_PIN; } while (0)

#endif	/* PINOUT_H */

