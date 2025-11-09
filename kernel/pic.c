#include "pic.h"

void PIC_sendEOI(uint8_t irq)
{
	if(irq >= 8)
		__outbyte(PIC2_COMMAND,PIC_EOI);
	
	__outbyte(PIC1_COMMAND,PIC_EOI);
}

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
	__outbyte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	__outbyte(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	__outbyte(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	__outbyte(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	__outbyte(PIC1_DATA, 1 << CASCADE_IRQ);        // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	io_wait();
	__outbyte(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	__outbyte(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	__outbyte(PIC2_DATA, ICW4_8086);
	io_wait();

	// Unmask both PICs.
	__outbyte(PIC1_DATA, 0);
	__outbyte(PIC2_DATA, 0);
}

void PIC_disable(void) {
    __outbyte(PIC1_DATA, 0xff);
    __outbyte(PIC2_DATA, 0xff);
}

void PIC_unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t value = __inbyte(port) & ~(1 << (irq % 8));
    __outbyte(port, value);
}