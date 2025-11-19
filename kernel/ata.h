#ifndef _ATA_H_
#define _ATA_H_

#include "types.h"

#define ATA_PRIMARY_IO       0x1F0
#define ATA_PRIMARY_CTRL     0x3F6

#define ATA_SECONDARY_IO     0x170
#define ATA_SECONDARY_CTRL   0x376

#define ATA_REG_DATA         0
#define ATA_REG_ERROR        1
#define ATA_REG_FEATURES     1
#define ATA_REG_SECCOUNT     2
#define ATA_REG_LBA0         3
#define ATA_REG_LBA1         4
#define ATA_REG_LBA2         5
#define ATA_REG_HDDEVSEL     6
#define ATA_REG_COMMAND      7
#define ATA_REG_STATUS       7

#define ATA_CMD_IDENTIFY     0xEC
#define ATA_CMD_READ_SECTORS 0x20

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF   0x20
#define ATA_SR_DSC  0x10
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

typedef struct {
    BOOL present;
    WORD io;
    WORD ctrl;
    BYTE slave;
} ata_device_t;

extern ata_device_t g_ata_devices[4];

void ata_init();
BOOL ata_read28(ata_device_t* dev, DWORD lba, BYTE* buf);

#endif
