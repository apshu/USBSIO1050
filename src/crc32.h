/* 
 * File:   crc32.h
 * Author: M91541
 *
 * Created on June 18, 2018, 9:39 AM
 */

#ifndef CRC32_H
#define	CRC32_H

#ifdef	__cplusplus
extern "C" {
#endif

    //#define CRC32_UNROLLED_LOOP
    uint32_t crc32_bitwise(void* data, uint16_t length, uint32_t previousCrc32);
    uint32_t crc32_finish(uint32_t previousCrc32);

#ifdef	__cplusplus
}
#endif

#endif	/* CRC32_H */

