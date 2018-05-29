/* 
 * File:   pinout_p16f1459.h
 * Author: M91541
 * Processor specific pinout file
 *
 * Created on May 29, 2018, 1:08 PM
 */
#if !(defined(_16F1459))
#error "This include file is specific to PIC16F1459"
#endif

#ifndef PINOUT_P16F1459_H
#define	PINOUT_P16F1459_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SDA_IN              PORTBbits.RB4
#define SCL_IN              PORTBbits.RB6
#define SDA_OUT             LATBbits.LATB4
#define SCL_OUT             LATBbits.LATB6
#define SDA_TRIS            TRISBbits.TRISB4
#define SCL_TRIS            TRISBbits.TRISB6
#define U1RX_IN             PORTBbits.RB5
#define U1RX_OUT            do { #error "rx pin is input only!" } while(0)
#define U1RX_TRIS           TRISBbits.TRISB5
#define U1TX_IN             PORTBbits.RB7
#define U1TX_OUT            LATBbits.LATB7
#define U1TX_TRIS           TRISBbits.TRISB7
#define COMMAND_IN          PORTAbits.RA4
#define COMMAND_OUT         do { #error "COMMAND pin is input only!" } while(0)
#define COMMAND_TRIS        TRISAbits.TRISA4
#define MOTOR_IN            PORTAbits.RA5
#define MOTOR_OUT           do { #error "MOTOR pin is input only!" } while(0)
#define MOTOR_TRIS          TRISAbits.TRISA5
#define READY_IN            PORTCbits.RC3
#define READY_OUT           do { #error "MOTOR pin is input only!" } while(0)
#define READY_TRIS          TRISCbits.TRISC3
#define EEPROM_PWRPIN_IN    PORTCbits.RC2
#define EEPROM_PWRPIN_OUT   LATCbits.LATC2
#define EEPROM_PWRPIN_TRIS  TRISCbits.TRISC2
#define mConfigureIOpins()  do { ANSELA = ANSELB = ANSELC = 0; nWPUEN = 0; WPUA = 0xFF; EEPROM_PWRPIN_OUT = 0; EEPROM_PWRPIN_TRIS = OUTPUT_PIN;} while(0)


#ifdef	__cplusplus
}
#endif

#endif	/* PINOUT_P16F1459_H */

