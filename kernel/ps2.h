#include <stdint.h>
#include "types.h"
#include "ioaccess.h"
#include "screen.h"

#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_CMD_PORT     0x64

// status bits
#define PS2_STATUS_OUTPUT_BUFFER_FULL   0x01  // output buffer full
#define PS2_STATUS_INPUT_BUFFER_FULL    0x02  // input buffer full

void ps2_wait_input_clear(void);

void ps2_wait_output_full(void);

// write controller command (port 0x64)
void ps2_send_command(uint8_t cmd);

// write data to device (port 0x60) after ensuring controller input buffer free
void ps2_send_data(uint8_t data);

// read data from device (assumes OBF is set)
uint8_t ps2_read_data(void);

void PS2_keyboard_init(void);