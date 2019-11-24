#include <kernel/io/io.h>
#include <mlibc/stdlib.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/interrupts/interrupts.h>

struct ata_interface *new_ata(uint8_t master, uint16_t port_base)
{
    struct ata_interface *ret = kmalloc(sizeof(struct ata_interface));

    ret->master = master;
    ret->data_port = port_base;
    ret->error_port = port_base + 0x1;
    ret->sector_count_port = port_base + 0x2;
    ret->lba_low_port = port_base + 0x3;
    ret->lba_mid_port = port_base + 0x4;
    ret->lba_high_port = port_base + 0x5;
    ret->device_port = port_base + 0x6;
    ret->command_port = port_base + 0x7;
    ret->control_port = port_base + 0x206;

    // isr_ignore(0x2E);
    // isr_ignore(0x2F);

    return ret;
}

uint8_t ata_identify(struct ata_interface *interface, uint16_t *ret_data)
{
    outb(interface->device_port, interface->master ? 0xA0 : 0xB0);
    outb(interface->control_port, 0);

    outb(interface->device_port, 0xA0);
    uint8_t status = inb(interface->command_port);
    if (status == 0xFF) return 1;

    outb(interface->device_port, interface->master ? 0xA0 : 0xB0);
    outb(interface->sector_count_port, 0);
    outb(interface->lba_low_port, 0);
    outb(interface->lba_mid_port, 0);
    outb(interface->lba_high_port, 0);
    outb(interface->command_port, 0xEC); // Identify command

    status = inb(interface->command_port);
    if (!status) return 1;

    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) {
        status = inb(interface->command_port);
    }

    if (status & 0x01) return 1;

    for (int i = 0; i < 256; i++) ret_data[i] = inw(interface->data_port);
    return 0;
}

uint8_t *ata_read28(struct ata_interface *interface, uint32_t sector)
{
    if (sector > 0x0FFFFFFF) return 0;

    outb(interface->device_port, (interface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

    uint8_t status = 0;
    for (int i = 0; i < 5; i++) status = inb(interface->command_port);
    if (status == 0xFF) return 0;

    outb(interface->error_port, 0);
    outb(interface->sector_count_port, 1);
    outb(interface->lba_low_port, sector & 0x000000FF);
    outb(interface->lba_mid_port, (sector & 0x0000FF00) >> 8);
    outb(interface->lba_high_port, (sector & 0x00FF0000) >> 16);
    outb(interface->command_port, 0x20); // Read command

    status = inb(interface->command_port);
    while ((status & 0x80) && !(status & 0x01)) status = inb(interface->command_port);

    uint8_t *ret = kmalloc(BYTES_PER_SECTOR);
    for (int i = 0; i < BYTES_PER_SECTOR; i += 2) {
        uint16_t data = inw(interface->data_port);
        ret[i] = data & 0xFF;
        ret[i + 1] = (data >> 8) & 0xFF;
    }
    return ret;
}

uint8_t ata_write28(struct ata_interface *interface, uint32_t sector, const uint8_t *contents)
{
    if (sector > 0x0FFFFFFF) return 1;
    asm ("cli");

    outb(interface->device_port, (interface->master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

    uint8_t status = 0;
    for (int i = 0; i < 5; i++) status = inb(interface->command_port);
    if (status == 0xFF) return 1;

    outb(interface->error_port, 0);
    outb(interface->sector_count_port, 1);
    outb(interface->lba_low_port, sector & 0x000000FF);
    outb(interface->lba_mid_port, (sector & 0x0000FF00) >> 8);
    outb(interface->lba_high_port, (sector & 0x00FF0000) >> 16);
    outb(interface->command_port, 0x30); // Write command

    while ((status & 0x80) || !(status & 0x08)) status = inb(interface->command_port);

    if (status & (0x01 || 0x20)) return 2;

    for (int i = 0; i < BYTES_PER_SECTOR; i += 2) {
        uint16_t data = contents[i];
        data |= ((uint16_t) contents[i + 1]) << 8;
        outw(interface->data_port, data);
    }

    outb(interface->command_port, 0xE7); // Flush command

    for (int i = 0; i < 5; i++) status = inb(interface->command_port);
    if (!status) return 3;

    while ((status & 0x80) && !(status & 0x01)) {
        status = inb(interface->command_port);
    }

    return 0;
}

uint8_t ata_clear28(struct ata_interface *interface, uint32_t sector)
{
    uint8_t empty_sector[512] = {0};
    return ata_write28(interface, sector, empty_sector);
}