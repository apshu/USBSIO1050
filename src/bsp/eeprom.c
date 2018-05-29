
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bsp/io_mapping.h"
#include "bsp/eeprom.h"

static void EEPROM_stop(void) {
    SSP1CON2bits.PEN = 1;
}

static bit EEPROM_sendAbyte(uint_fast8_t data) {
    SSP1BUF = data & 0xFF;
    SSP1IF = 0;
    while (!SSP1IF) {
        continue;
    }
    if (SSP1CON2bits.ACKSTAT) {
        //NAK received from slave
        EEPROM_stop();
        return false;
    }
    return true;
}

void EEPROM_init(void) {
    EEPROM_powerOff();
    SSP1CON1 = 0; //Disable MSSP
    SSP1ADD = 16;
    SSP1CON2 = SSP1CON3 = SSP1STAT = 0;
    __delay_ms(10);
    EEPROM_powerOn();
    __delay_ms(10);
    SSP1CON1 = 0x28; //Enable MSSP I2C Master
}

//Return false if something went wrong

static bool EEPROM_writeNoStop(uint_fast24_t dataAddress, uint8_t *buf, uint_fast16_t numBytes) {
    if (SSP1CON1 == 0x28) {
        SSP1CON2bits.SEN = 1;
        dataAddress &= (uint_fast24_t)((1UL << 18UL) - 1UL);
        uint_fast8_t dout = (dataAddress >> 16) & 0xFF;
        dout <<= 1;
        dout |= EEPROM_I2C_ADDRESS & 0xFF; //Device address for Write
        while (SSP1CON2bits.SEN) {
            continue;
        }
        if (EEPROM_sendAbyte(dout)) {
            if (EEPROM_sendAbyte(dataAddress >> 8)) {
                if (EEPROM_sendAbyte(dataAddress)) {
                    //Address set correctly
                    if (buf && numBytes) {
                        //We want to send something
                        if (numBytes + (dataAddress & (EEPROM_PAGE_BYTES - 1)) <= EEPROM_PAGE_BYTES) {
                            //Not crossing page boundary
                            for (; numBytes; --numBytes) {
                                if (!EEPROM_sendAbyte(*buf++)) {
                                    //ACK error, STOP already sent, write might happen
                                    __delay_ms(5);
                                    return false;
                                }
                            }
                            //Written
                            return true;
                        }
                    } else {
                        //Nothing to send, address correctly received by the device
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool EEPROM_read(uint_fast24_t dataAddress, uint8_t *buf, uint_fast16_t numBytes) {
    if (EEPROM_writeNoStop(dataAddress, NULL, 0)) {
        //Read address set
        SSP1CON2bits.RSEN = 1;
        uint_fast8_t dout = (dataAddress >> 15) & 0xF;
        dout |= EEPROM_I2C_ADDRESS | 1;
        while (SSP1CON2bits.RSEN) {
            continue;
        }
        if (EEPROM_sendAbyte(dout)) {
            //EEPROM addressed for read
            if (buf) {
                while (numBytes) {
                    SSPCON2bits.RCEN = 1;
                    SSP1IF = 0;
                    while (!SSP1IF) {
                        continue;
                    }
                    *buf++ = SSP1BUF;
                    --numBytes;
                    SSP1CON2bits.ACKDT = !numBytes;
                    SSP1CON2bits.ACKEN = 1;
                    while (SSP1CON2bits.ACKEN) {
                        continue;
                    }
                }
            }
            EEPROM_stop();
            return true;
        }
    }
    EEPROM_stop();
    return false;
}

bool EEPROM_write(uint_fast24_t dataAddress, uint8_t *buf, uint_fast16_t numBytes) {
    if (EEPROM_writeNoStop(dataAddress, buf, numBytes)) {
        //Bytes written, stop condition will start a write
        EEPROM_stop();
        __delay_ms(5);
        return true;
    }
    return false;
}

