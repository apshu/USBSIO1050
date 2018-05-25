#include <xc.h>
#include <stdint.h>
#include "bsp/io_mapping.h"
#include "bsp/uart.h"

#define SIO_IOBUFFER_LENGTH (300)

typedef enum {
    SIO_CMDACK = 'A',
    SIO_CMDNAK = 'N',
    SIO_CMDERROR = 'E',
    SIO_CMDCOMPLETE = 'C',
} SIO_replies_t;

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

static union {
    uint8_t asBytes[128 + 5];
    SIO_command_t asSIOcmd;
} SIO_iobuffer;

static enum {
    SIO_TASK_WAIT_IDLE,
    SIO_TASK_WAIT_COMMAND,
    SIO_TASK_RCV_COMMAND,
    SIO_TASK_EXEC_COMMAND,
} SIO_task_state = SIO_TASK_WAIT_IDLE;

static uint_fast8_t SIO_byteCounter;
static uint_fast8_t SIO_deviceID = '1';

void SIO_task(void) {
    switch (SIO_task_state) {
        case SIO_TASK_WAIT_IDLE:
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
                    //Correct command received, execute it
                    SIO_task_state = SIO_TASK_EXEC_COMMAND;
                } else {
                    SIO_task_state = SIO_TASK_WAIT_IDLE;
                }
            } else {
                if ((SIO_byteCounter <= 5) && UART_RxRdy()) {
                    SIO_iobuffer.asBytes[SIO_byteCounter++] = UART_getch();
                }
            }
            break;
        case SIO_TASK_EXEC_COMMAND:
            if (SIO_iobuffer.asSIOcmd.cmdParams.DDEVIC == SIO_deviceID) {

            }
            UART_putch(SIO_CMDNAK);
            SIO_task_state = SIO_TASK_WAIT_IDLE;
            break;
    }
}