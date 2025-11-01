/***
    Courtesy of https://wiki.osdev.org/Inline_Assembly/Examples
*/
#ifndef _IOACCESS_H_
#define _IOACCESS_H_

#include <stdint.h>

void __outbyte(uint16_t port, uint8_t val);
uint8_t __inbyte(uint16_t port);

#endif // _IOACCESS_H_
