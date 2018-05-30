/* 
 * File:   eeprom.h
 * Author: M91541
 *
 * Created on May 23, 2018, 2:00 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif

#define EEPROM_I2C_ADDRESS  0xA0 
#define EEPROM_PAGE_BYTES   (256)
    
    void EEPROM_init(void);
    bool EEPROM_read(uint_fast24_t dataAddress, uint8_t *buf, uint_fast16_t numBytes);
    bool EEPROM_write(uint_fast24_t dataAddress, uint8_t *buf, uint_fast16_t numBytes);
    static bool inline EEPROM_isDetected() {return EEPROM_write(0,(void*)0,0);}

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

