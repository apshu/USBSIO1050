// //////////////////////////////////////////////////////////
// Crc32Best.ino
// Copyright (c) 2011 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//
// Modified by Attila Kolinger to have smaller footprint to the cost of having less features

/// zlib's CRC32 polynomial

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define CRC32_POLYNOMIAL (0xEDB88320UL)

/// compute CRC32
uint32_t crc32_bitwise(void* data, uint16_t length, uint32_t previousCrc32) {
    uint8_t* current = (uint8_t*) data;

    while (length--) {
        previousCrc32 ^= *current++;
#ifndef CRC32_UNROLLED_LOOP
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t lowestBit = previousCrc32 & 1;
            previousCrc32 >>= 1;
            if (lowestBit)
                previousCrc32 ^= CRC32_POLYNOMIAL;
        }
#else
        // fully unrolled
        uint8_t lowestBit;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
        lowestBit = previousCrc32 & 1;
        previousCrc32 >>= 1;
        if (lowestBit)
            previousCrc32 ^= CRC32_POLYNOMIAL;
#endif
    }

    return previousCrc32; // same as crc ^ 0xFFFFFFFF
}

uint32_t crc32_finish(uint32_t previousCrc32) {
    return ~previousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
}    
