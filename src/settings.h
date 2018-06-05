/* 
 * File:   settings.h
 * Author: M91541
 *
 * Created on June 5, 2018, 9:51 AM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef volatile struct {
        uint8_t SIO_driveID;
    } FLASH_settings_t;

    bit SETTINGS_isATRwriteable(void);
    uint8_t SETTINGS_getSIOaddress(void);
    bit SETTINGS_store(FLASH_settings_t *newSettings);
    
#ifdef	__cplusplus
}
#endif

#endif	/* SETTINGS_H */

