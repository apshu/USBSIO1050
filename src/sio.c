#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "bsp/io_mapping.h"
#include "bsp/uart.h"
#include "bsp/eeprom.h"

#define SIO_IOBUFFER_LENGTH (128+5)

typedef enum {
    SIO_CMDACK = 'A',
    SIO_CMDNAK = 'N',
    SIO_CMDERROR = 'E',
    SIO_CMDCOMPLETE = 'C',
} SIO_replies_t;

typedef enum {
    SIODEV1050_CMD_formatAutomatic = 0x20,
    SIODEV1050_CMD_formatDisk = 0x21,
    SIODEV1050_CMD_formatEnhanced = 0x22,
    SIODEV1050_CMD_diagnostic = 0x23,
    SIODEV1050_CMD_getDiagResult = 0x24,
    SIODEV1050_CMD_getHighpeedIndex = 0x3F,
    SIODEV1050_CMD_readDriveConfig = 0x4E,
    SIODEV1050_CMD_writeDriveConfig = 0x4F,
    SIODEV1050_CMD_writeSector = 0x50,
    SIODEV1050_CMD_readSector = 0x52,
    SIODEV1050_CMD_getStatus = 0x53,
    SIODEV1050_CMD_writeSectorVerify = 0x57,
} SIODEV1050_commands_t;

typedef struct {
    uint8_t DDEVIC;
    uint8_t DCOMND;
    uint8_t DAUX1;
    uint8_t DAUX2;
} SIO_commandParams_t;

typedef struct {
    SIO_commandParams_t cmdParams;
    uint8_t chekSum;
} SIO_command_t;

typedef struct {
    uint8_t cmdComplete;
    uint8_t dataBytes[128];
} SIO_commandReply_t;

typedef struct {
    uint8_t cmdComplete;
    uint8_t drvStatus;
    uint8_t nWD2793Status;
    uint8_t formatTimeout;
    uint8_t WD2793Status;
    uint8_t CSUM;
} SIO_statusReply_t;

typedef struct {
    SIO_command_t SIOcmd;
    uint8_t dataBytes[128];
} SIO_indata128_t;

static union {
    uint8_t asBytes[SIO_IOBUFFER_LENGTH];
    SIO_command_t asSIOcmd;
    SIO_commandReply_t as_R_reply;
    SIO_statusReply_t as_R_status;
    SIO_indata128_t as_W_data;
    SIO_indata128_t as_W_sector;
} SIO_ioBuffer;

static enum {
    SIO_TASK_WAIT_IDLE,
    SIO_TASK_WAIT_COMMAND,
    SIO_TASK_RCV_COMMAND,
    SIO_TASK_EXEC_COMMAND,
    SIO_TASK_SEND_BUFFER,
    SIO_TASK_RECEIVE_BUFFER,
    SIO_TASK_WRITE_SECTOR,
    SIO_TASK_FORMAT_EEPROM,
} SIO_task_state = SIO_TASK_WAIT_IDLE;

static uint_fast8_t SIO_byteCounter;
static uint_fast8_t SIO_bytesToCommunicate;
static uint_fast8_t SIO_deviceID = '1';
static uint_fast16_t SIO_wordCounter;

/* Input parameter is SIO_byteCounter that tells how many bytes to add to CSUM 
   Input parameter is SIO_ioBuffer.asBytes 
   Output is SIO_ioBuffer.asBytes with appended CSUM to the right position 
   Return value is the checksum
   On return the SIO_byteCounter will be 0 */
#pragma warning push
#pragma warning disable 343 //It looks like explicit return, while it's not 

static uint8_t SIO_appendCSUM(void) {
    volatile uint8_t csum = 0;
    asm("\tMOVLW HIGH _SIO_ioBuffer");
    asm("\tMOVWF FSR0H");
    asm("\tMOVLW LOW _SIO_ioBuffer");
    asm("\tMOVWF FSR0L");
    asm("\tBANKSEL(_SIO_byteCounter)");
    asm("\tBCF STATUS, 0");
    asm("NEXT_BYTE:");
    asm("\tMOVIW FSR0++");
    asm("\tADDWFC SIO_appendCSUM@csum, f");
    asm("\tDECFSZ ((_SIO_byteCounter)&07Fh), f");
    asm("\tBRA NEXT_BYTE");
    asm("\tMOVLW 0");
    asm("\tADDWFC SIO_appendCSUM@csum, W");
    asm("\tMOVWI [0]FSR0");
}
#pragma warning pop
#pragma regsused SIO_appendCSUM wreg status fsr0

void SIO_task(void) {
    if (!HOSTATR_isReady()) {
        //Host powered off
        SIO_task_state = SIO_TASK_WAIT_IDLE;
        UART_disable();
    }
    switch (SIO_task_state) {
        case SIO_TASK_WAIT_IDLE:
            UART_enable();
            if (!COMMAND_isActive()) {
                SIO_task_state = SIO_TASK_WAIT_COMMAND;
            }
            break;
        case SIO_TASK_WAIT_COMMAND:
            if (COMMAND_isActive()) {
                SIO_byteCounter = 0;
                SIO_task_state = SIO_TASK_RCV_COMMAND;
            }
            break;
        case SIO_TASK_RCV_COMMAND:
            if (!COMMAND_isActive()) {
                if (SIO_byteCounter == 5) {
                    SIO_bytesToCommunicate = SIO_ioBuffer.asBytes[--SIO_byteCounter]; //CSUM - Reusing variable SIO_bytesToCommunicate
                    if (SIO_bytesToCommunicate == SIO_appendCSUM()) {
                        //Correct command received, execute it
                        SIO_task_state = SIO_TASK_EXEC_COMMAND;
                        break;
                    }
                }
                putch(SIO_CMDNAK);
                SIO_task_state = SIO_TASK_WAIT_IDLE;
            } else {
                if (UART_RxRdy()) {
                    if (SIO_byteCounter <= 5) {
                        SIO_ioBuffer.asBytes[SIO_byteCounter++] = UART_getch();
                    }
                }
            }
            break;
        case SIO_TASK_EXEC_COMMAND:
            SIO_task_state = SIO_TASK_WAIT_IDLE;
            if (!COMMAND_isActive()) {
                if (SIO_ioBuffer.asSIOcmd.cmdParams.DDEVIC == SIO_deviceID) {
                    if (EEPROM_isDetected()) { //Reply only if EEPROM is available
                        SIO_ioBuffer.as_R_status.cmdComplete = SIO_CMDNAK;
                        SIO_task_state = SIO_TASK_SEND_BUFFER;
                        SIO_bytesToCommunicate = 1;
                        switch (SIO_ioBuffer.asSIOcmd.cmdParams.DCOMND) {
                            case SIODEV1050_CMD_getStatus:
                                putch(SIO_CMDACK); //Command acknowledged
                                SIO_ioBuffer.as_R_status.cmdComplete = SIO_CMDCOMPLETE;
                                SIO_ioBuffer.as_R_status.drvStatus = 0x80;
                                SIO_ioBuffer.as_R_status.nWD2793Status = 0xFF;
                                SIO_ioBuffer.as_R_status.formatTimeout = 0xE0;
                                SIO_ioBuffer.as_R_status.WD2793Status = 0x00;
                                SIO_ioBuffer.as_R_status.CSUM = 0x44;
                                SIO_bytesToCommunicate = 6;
                                SIO_byteCounter = 0;
                                break;
                            case SIODEV1050_CMD_readSector:
                                putch(SIO_CMDACK); //Command acknowledged
                                SIO_ioBuffer.as_R_reply.cmdComplete = EEPROM_read((uint_fast24_t) ((SIO_ioBuffer.asSIOcmd.cmdParams.DAUX1 + (SIO_ioBuffer.asSIOcmd.cmdParams.DAUX2 << 8)) << 7) + 16, SIO_ioBuffer.as_R_reply.dataBytes, 128) ? SIO_CMDCOMPLETE : SIO_CMDERROR;
                                SIO_bytesToCommunicate = 130;
                                SIO_byteCounter = 129;
                                SIO_appendCSUM(); //SIO_byteCounter = 0; // as side effect to SIO_appendCSUM() call
                                break;
                            case SIODEV1050_CMD_writeSector:
                                putch(SIO_CMDACK); //Command acknowledged
                                SIO_bytesToCommunicate = 129;
                                SIO_byteCounter = 0;
                                SIO_task_state = SIO_TASK_RECEIVE_BUFFER;
                                break;
                            case SIODEV1050_CMD_formatDisk:
                            case SIODEV1050_CMD_formatEnhanced:
                                putch(SIO_CMDACK); //Command acknowledged
                                memset(SIO_ioBuffer.as_R_reply.dataBytes, 0xFF, sizeof (SIO_ioBuffer.as_R_reply.dataBytes));
                                memset(SIO_ioBuffer.as_R_reply.dataBytes, 0, 16);
                                SIO_ioBuffer.as_R_reply.dataBytes[0] = 0x96;
                                SIO_ioBuffer.as_R_reply.dataBytes[1] = 0x02;
                                SIO_ioBuffer.as_R_reply.dataBytes[3] = 0x40;
                                SIO_ioBuffer.as_R_reply.dataBytes[4] = 0x80;
                                SIO_wordCounter = EEPROM_BYTE_SIZE / sizeof (SIO_ioBuffer.as_R_reply.dataBytes);
                                SIO_task_state = SIO_TASK_FORMAT_EEPROM;
                                break;
                        }
                    }
                }
            }
            break;
        case SIO_TASK_SEND_BUFFER:
            if (COMMAND_isActive()) {
                SIO_task_state = SIO_TASK_WAIT_IDLE;
                break;
            }

            if (UART_TxRdy()) {
                if (SIO_bytesToCommunicate--) {
                    UART_putch(SIO_ioBuffer.asBytes[SIO_byteCounter++]);
                } else {
                    SIO_task_state = SIO_TASK_WAIT_IDLE;
                }
            }
            break;
        case SIO_TASK_RECEIVE_BUFFER:
            if (COMMAND_isActive()) {
                SIO_task_state = SIO_TASK_WAIT_IDLE;
                break;
            }

            if (UART_RxRdy()) {
                if (SIO_bytesToCommunicate--) {
                    SIO_ioBuffer.as_W_data.dataBytes[SIO_byteCounter++] = UART_getch();
                } else {
                    SIO_bytesToCommunicate = SIO_ioBuffer.as_W_data.dataBytes[--SIO_byteCounter]; //CSUM - Reusing variable SIO_bytesToCommunicate
                    if (SIO_bytesToCommunicate == SIO_appendCSUM()) {
                        SIO_task_state = SIO_TASK_WRITE_SECTOR;
                    } else {
                        SIO_bytesToCommunicate = 1; //Byte counter == 0, as CSUM calculation side effect
                        SIO_ioBuffer.as_R_reply.cmdComplete = SIO_CMDERROR;
                        SIO_task_state = SIO_TASK_SEND_BUFFER;
                    }
                }
            }
            break;
        case SIO_TASK_WRITE_SECTOR:
            if (COMMAND_isActive()) {
                SIO_task_state = SIO_TASK_WAIT_IDLE;
                break;
            }
            SIO_bytesToCommunicate = 2;
            if (EEPROM_write((uint_fast24_t) ((SIO_ioBuffer.as_W_sector.SIOcmd.cmdParams.DAUX1 + (SIO_ioBuffer.as_W_sector.SIOcmd.cmdParams.DAUX2 << 8)) << 7) + 16, SIO_ioBuffer.as_W_sector.dataBytes, 128)) {
                putch(SIO_CMDCOMPLETE);
            } else {
                putch(SIO_CMDERROR);
            }
            SIO_task_state = SIO_TASK_WAIT_IDLE;
            break;
        case SIO_TASK_FORMAT_EEPROM:
            if (COMMAND_isActive()) {
                SIO_task_state = SIO_TASK_WAIT_IDLE;
                break;
            }
            if (SIO_wordCounter) {
                if (EEPROM_write(0, SIO_ioBuffer.as_R_reply.dataBytes, sizeof (SIO_ioBuffer.as_R_reply.dataBytes))) {
                    memset(SIO_ioBuffer.as_R_reply.dataBytes, 0xFF, sizeof (SIO_ioBuffer.as_R_reply.dataBytes));
                    SIO_wordCounter--;
                    break;
                }
                memset(SIO_ioBuffer.as_R_reply.dataBytes, 0, sizeof (SIO_ioBuffer.as_R_reply.dataBytes));
                SIO_ioBuffer.as_R_reply.cmdComplete = SIO_CMDERROR;
                SIO_bytesToCommunicate = 130;
                SIO_byteCounter = 129;
                SIO_appendCSUM();
                SIO_task_state = SIO_TASK_SEND_BUFFER;
            } else {
                SIO_ioBuffer.as_R_reply.cmdComplete = SIO_CMDCOMPLETE;
                SIO_bytesToCommunicate = 130;
                SIO_byteCounter = 129;
                SIO_appendCSUM();
                SIO_task_state = SIO_TASK_SEND_BUFFER;
            }
            break;
    }
}
