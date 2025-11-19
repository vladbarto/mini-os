#include "ata.h"
#include "screen.h"

ata_device_t g_ata_devices[4];

static BYTE io_inb(WORD base, BYTE reg) {
    return __inbyte(base + reg);
}

static void io_outb(WORD base, BYTE reg, BYTE val) {
    __outbyte(base + reg, val);
}

static void ata_select(ata_device_t* d) {
    io_outb(d->io, ATA_REG_HDDEVSEL, 0xA0 | (d->slave << 4));
}

static BOOL ata_wait(ata_device_t* d) {
    BYTE s;
    do { s = io_inb(d->io, ATA_REG_STATUS); }
    while (s & ATA_SR_BSY);

    if (s & ATA_SR_ERR) return false;
    return true;
}

void ata_init() {
    WORD bases[2]  = { ATA_PRIMARY_IO, ATA_SECONDARY_IO };
    WORD ctrls[2]  = { ATA_PRIMARY_CTRL, ATA_SECONDARY_CTRL };

    int idx = 0;

    for (int bus = 0; bus < 2; bus++) {
        for (int slave = 0; slave < 2; slave++) {

            ata_device_t* d = &g_ata_devices[idx++];
            d->io   = bases[bus];
            d->ctrl = ctrls[bus];
            d->slave = slave;
            d->present = false;

            ata_select(d);

            io_outb(d->io, ATA_REG_SECCOUNT, 0);
            io_outb(d->io, ATA_REG_LBA0, 0);
            io_outb(d->io, ATA_REG_LBA1, 0);
            io_outb(d->io, ATA_REG_LBA2, 0);

            io_outb(d->io, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            BYTE status = io_inb(d->io, ATA_REG_STATUS);

            if (status == 0)
                continue; // no device

            // wait BSY clear
            while (status & ATA_SR_BSY)
                status = io_inb(d->io, ATA_REG_STATUS);

            // ATAPI?
            if (io_inb(d->io, ATA_REG_LBA1) == 0x14 &&
                io_inb(d->io, ATA_REG_LBA2) == 0xEB)
                continue;

            // wait for DRQ
            while (!(status & ATA_SR_DRQ) && !(status & ATA_SR_ERR))
                status = io_inb(d->io, ATA_REG_STATUS);

            if (status & ATA_SR_ERR)
                continue;

            // read IDENTIFY data (ignore)
            WORD junk;
            for (int i = 0; i < 256; i++)
                junk = __inword(d->io + ATA_REG_DATA);

            d->present = true;
            ScreenDisplay("ATA device detected\n", GREEN_BRIGHT_8BIT);
        }
    }
}

BOOL ata_read28(ata_device_t* d, DWORD lba, BYTE* buf) {
    if (!d->present) return false;

    ata_select(d);

    io_outb(d->io, ATA_REG_SECCOUNT, 1);
    io_outb(d->io, ATA_REG_LBA0, (BYTE)(lba));
    io_outb(d->io, ATA_REG_LBA1, (BYTE)(lba >> 8));
    io_outb(d->io, ATA_REG_LBA2, (BYTE)(lba >> 16));
    io_outb(d->io, ATA_REG_HDDEVSEL,
        0xE0 | (d->slave << 4) | ((lba >> 24) & 0x0F));

    io_outb(d->io, ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    if (!ata_wait(d))
        return false;

    if (!(io_inb(d->io, ATA_REG_STATUS) & ATA_SR_DRQ))
        return false;

    // read 512 bytes
    WORD* dst = (WORD*)buf;
    for (int i = 0; i < 256; i++)
        dst[i] = __inword(d->io + ATA_REG_DATA);

    return true;
}
