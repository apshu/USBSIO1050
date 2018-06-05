#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "settings.h"
#include "bsp/eeprom.h"
#include "usb/usb_device.h"

#define WRITE_FLASH_BLOCKSIZE 32

const FLASH_settings_t HEF_settings @ 0x1FE0 = {'1'}; /* HEF, last 64 blocks of FLASH */

bit SETTINGS_isATRwriteable(void) {
    return false;
}

uint8_t SETTINGS_getSIOaddress(void) {
    return HEF_settings.SIO_driveID;
}

bit SETTINGS_store(FLASH_settings_t *newSettings) {
    uint16_t_VAL writeAddr;
    writeAddr.Val = ((uint16_t) & HEF_settings) & 0x1FFF;
    uint8_t *dataArray = (uint8_t *) newSettings;

    /*************** ERASE routine */
    uint8_t GIEBitValue = INTCONbits.GIE; // Save interrupt enable

    INTCONbits.GIE = 0; // Disable interrupts
    // Load lower 8 bits of erase address boundary
    PMADRL = writeAddr.byte.LB;
    // Load upper 6 bits of erase address boundary
    PMADRH = writeAddr.byte.HB;

    // Block erase sequence
    PMCON1bits.CFGS = 0; // Deselect Configuration space
    PMCON1bits.FREE = 1; // Specify an erase operation
    PMCON1bits.WREN = 1; // Allows erase cycles

    // Start of required sequence to initiate erase
    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    PMCON1bits.WR = 1; // Set WR bit to begin erase
    NOP();
    NOP();

    //PMCON1bits.WREN = 0;       // Disable writes

    /********************* Write routine */
    uint8_t i;
    // Block write sequence
    PMCON1bits.LWLO = 1; // Only load write latches

    for (i = 0; i < WRITE_FLASH_BLOCKSIZE; i++) {
        // Load lower 8 bits of write address
        PMADRL = writeAddr.byte.LB;

        // Load upper 6 bits of write address
        PMADRH = writeAddr.byte.HB;

        // Load data in current address
        if (i < sizeof (FLASH_settings_t)) {
            PMDATL = dataArray[i];
            PMDATH = 0x34;
        } else {
            PMDAT = 0x3FFF;
        }

        if (i == (WRITE_FLASH_BLOCKSIZE - 1)) {
            // Start Flash program memory write
            PMCON1bits.LWLO = 0;
        }

        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        PMCON1bits.WR = 1;
        NOP();
        NOP();

        writeAddr.Val++;
    }

    /********************* Restore interrupts */
    PMCON1bits.WREN = 0; // Disable writes
    INTCONbits.GIE = GIEBitValue; // Restore interrupt enable

    return true;
}
