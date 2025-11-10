#include "ioaccess.h"
#include "logging.h"
#include <stdint.h>

#define PIT_CHANNEL0    0x40
#define PIT_COMMAND     0x43
#define PIT_FREQUENCY   1193181

void PIT_init(uint32_t freq_hz);