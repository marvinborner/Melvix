#include <kernel/io/io.h>
#include <mlibc/stdlib.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/interrupts/interrupts.h>

struct ATA_INTERFACE *new_ATA(uint8_t master, uint16_t portBase) {
    struct ATA_INTERFACE *ret = kmalloc(sizeof(struct ATA_INTERFACE));

    ret->master = master;
    ret->dataPort = portBase;
    ret->errorPort = portBase + 0x1;
    ret->sectorCountPort = portBase + 0x2;
    ret->lbaLowPort = portBase + 0x3;
    ret->lbaMidPort = portBase + 0x4;
    ret->lbaHiPort = portBase + 0x5;
    ret->devicePort = portBase + 0x6;
    ret->commandPort = portBase + 0x7;
    ret->controlPort = portBase + 0x206;

    // isr_ignore(0x06);
    // isr_ignore(0x2E);
    // isr_ignore(0x2F);

    return ret;
}

uint8_t ATA_identify(struct ATA_INTERFACE *iface, uint16_t *retdata) {
    send_b(iface->devicePort, iface->master ? 0xA0 : 0xB0);
    send_b(iface->controlPort, 0);

    send_b(iface->devicePort, 0xA0);
    uint8_t status = receive_b(iface->commandPort);
    if (status == 0xFF) return 1;

    send_b(iface->devicePort, iface->master ? 0xA0 : 0xB0);
    send_b(iface->sectorCountPort, 0);
    send_b(iface->lbaLowPort, 0);
    send_b(iface->lbaMidPort, 0);
    send_b(iface->lbaHiPort, 0);
    send_b(iface->commandPort, 0xEC); // Identify command

    status = receive_b(iface->commandPort);
    if (!status) return 1;

    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
        status = receive_b(iface->commandPort);
    }

    if (status & 0x01) return 1;

    for (int i = 0; i < 256; i++) retdata[i] = receive_w(iface->dataPort);
    return 0;
}

uint8_t *ATA_read28(struct ATA_INTERFACE *iface, uint32_t sector) {
    if (sector > 0x0FFFFFFF) return 0;

    send_b(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

    uint8_t status;
    for (int i = 0; i < 5; i++) status = receive_b(iface->commandPort);
    if (status == 0xFF) return 0;

    send_b(iface->errorPort, 0);
    send_b(iface->sectorCountPort, 1);
    send_b(iface->lbaLowPort, sector & 0x000000FF);
    send_b(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
    send_b(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
    send_b(iface->commandPort, 0x20); // Read command

    status = receive_b(iface->commandPort);
    while ((status & 0x80) && !(status & 0x01)) status = receive_b(iface->commandPort);

    uint8_t *ret = kmalloc(BYTES_PER_SECTOR);
    for (int i = 0; i < BYTES_PER_SECTOR; i += 2) {
        uint16_t data = receive_w(iface->dataPort);
        ret[i] = data & 0xFF;
        ret[i + 1] = (data >> 8) & 0xFF;
    }
    return ret;
}

uint8_t ATA_write28(struct ATA_INTERFACE *iface, uint32_t sector, uint8_t *contents) {
    if (sector > 0x0FFFFFFF) return 1;
    asm volatile ("cli");

    send_b(iface->devicePort, (iface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

    uint8_t status;
    for (int i = 0; i < 5; i++) status = receive_b(iface->commandPort);
    if (status == 0xFF) return 1;

    send_b(iface->errorPort, 0);
    send_b(iface->sectorCountPort, 1);
    send_b(iface->lbaLowPort, sector & 0x000000FF);
    send_b(iface->lbaMidPort, (sector & 0x0000FF00) >> 8);
    send_b(iface->lbaHiPort, (sector & 0x00FF0000) >> 16);
    send_b(iface->commandPort, 0x30); // Write command

    while ((status & 0x80) || !(status & 0x08)) status = receive_b(iface->commandPort);

    if (status & (0x01 || 0x20)) return 2;

    for (int i = 0; i < BYTES_PER_SECTOR; i += 2) {
        uint16_t data = contents[i];
        data |= ((uint16_t) contents[i + 1]) << 8;
        send_w(iface->dataPort, data);
    }

    send_b(iface->commandPort, 0xE7); // Flush command

    for (int i = 0; i < 5; i++) status = receive_b(iface->commandPort);
    if (!status) return 3;

    while ((status & 0x80) && !(status & 0x01)) {
        status = receive_b(iface->commandPort);
    }

    return 0;
}

uint8_t ATA_clear28(struct ATA_INTERFACE *iface, uint32_t sector) {
    uint8_t emptysector[512] = {0};
    return ATA_write28(iface, sector, emptysector);
}