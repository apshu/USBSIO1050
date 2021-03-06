#ifndef XPRJ_WITH_DFU

#include "xc.h"

#if !(defined(_16F1454) || defined(_16F1455) || defined(_16F1459))
#error "Please use this CONFIG FUSE file with supported MCU"
#endif

/** CONFIGURATION Bits **********************************************/
#if __DEBUG
#pragma config FOSC = HS        // Oscillator Selection Bits 
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config PLLMULT = 4x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)
#else
#pragma config FOSC = INTOSC    // Oscillator Selection Bits 16Mhz External clock
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT disabled)
#pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected) 16MHz
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)
#endif
#pragma config WDTE = SWDTEN    // Watchdog Timer Enable (WDT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is IOPIN)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)

// CONFIG2
#pragma config WRT = BOOT        // Flash Memory Self-Write Protection (Write protection off)
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection Bit (NO CPU system divide)
#pragma config USBLSCLK = 48MHz // USB Low SPeed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
#pragma config PLLEN = ENABLED  // PLL Enable Bit (3x or 4x PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#else
#error "This file should be excluded from configuration when DFU is included "
#endif