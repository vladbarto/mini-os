#include "ps2.h"

void ps2_wait_input_clear(void) {
    // wait until input buffer free (bit1 == 0)
    while (__inbyte(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL) { /* spin */ }
}

void ps2_wait_output_full(void) {
    // wait until output buffer full (bit0 == 1)
    while (!(__inbyte(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL)) { /* spin */ }
}

// write controller command (port 0x64)
void ps2_send_command(uint8_t cmd) {
    ps2_wait_input_clear();
    __outbyte(PS2_CMD_PORT, cmd);
}

// write data to device (port 0x60) after ensuring controller input buffer free
void ps2_send_data(uint8_t data) {
    ps2_wait_input_clear();
    __outbyte(PS2_DATA_PORT, data);
}

// read data from device (assumes OBF is set)
uint8_t ps2_read_data(void) {
    return __inbyte(PS2_DATA_PORT);
}

void PS2_keyboard_init(void)
{
    uint8_t cmdbyte;

    // Enable first PS/2 port (keyboard)
    ps2_send_command(0xAE); // enable first port. Waits IBF internally.

    // Read controller command byte
    ps2_send_command(0x20); // request read
    ps2_wait_output_full();
    cmdbyte = ps2_read_data();

    // Enable IRQ for first port (bit 0). Keep other bits as-is.
    cmdbyte |= 0x01; // bit0 = enable first port interrupt
    // Write back command byte: send 0x60, then the data byte to port 0x60
    ps2_send_command(0x60);
    ps2_send_data(cmdbyte);

    // Wnable scanning on keyboard device (device command 0xF4)
    // Send device command directly: write 0xF4 to data port (first port)
    // Controller will forward it to keyboard; wait for ACK 0xFA.
    ps2_send_data(0xF4);
    ps2_wait_output_full();
    uint8_t ack = ps2_read_data();
    (void)ack; // accept 0xFA, ignore otherwise for now

    FormattedLog("PS/2 keyboard initialized, controller cmdbyte=0x%X, ack=0x%X\n", cmdbyte, ack);
}
