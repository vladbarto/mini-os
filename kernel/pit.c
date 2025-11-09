#include "pit.h"

void PIT_init(uint32_t freq_hz)
{
    if (freq_hz == 0) return;

    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / freq_hz);

    // Command byte: channel 0, access lobyte/hibyte, mode 2, binary
    __outbyte(PIT_COMMAND, 0x34);

    // Send divisor (low byte first)
    __outbyte(PIT_CHANNEL0, divisor & 0xFF);
    __outbyte(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    Log("PIT initialized for %u Hz (divisor=%u).", freq_hz, divisor);
}
