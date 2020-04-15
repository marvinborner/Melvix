#include <kernel/system.h>
#include <kernel/io/io.h>
#include <kernel/fs/ata.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/alloc.h>
#include <kernel/pci/pci.h>
#include <kernel/interrupts/interrupts.h>

uint32_t ata_device;

ata_dev_t primary_master = { .slave = 0 };
ata_dev_t primary_slave = { .slave = 1 };
ata_dev_t secondary_master = { .slave = 0 };
ata_dev_t secondary_slave = { .slave = 1 };

void io_wait(ata_dev_t *dev)
{
	inb(dev->alt_status);
	inb(dev->alt_status);
	inb(dev->alt_status);
	inb(dev->alt_status);
}

void software_reset(ata_dev_t *dev)
{
	outb(dev->control, CONTROL_SOFTWARE_RESET);
	io_wait(dev);
	outb(dev->control, CONTROL_ZERO);
}

void ata_handler(struct regs *reg)
{
	inb(primary_master.status);
	inb(primary_master.BMR_STATUS);
	outb(primary_master.BMR_COMMAND, BMR_COMMAND_DMA_STOP);
	//irq_ack(14);
}

void ata_open(vfs_node_t *node, uint32_t flags)
{
	return;
}

void ata_close(vfs_node_t *node)
{
	return;
}

uint32_t ata_read(vfs_node_t *node, uint32_t offset, uint32_t size, char *buf)
{
	uint32_t start = offset / SECTOR_SIZE;
	uint32_t start_offset = offset % SECTOR_SIZE;

	uint32_t end = (offset + size - 1) / SECTOR_SIZE;
	uint32_t end_offset = (offset + size - 1) % SECTOR_SIZE;

	char *buf_curr = buf;
	uint32_t counter = start;
	uint32_t read_size;
	uint32_t off, total = 0;

	while (counter <= end) {
		off = 0;
		read_size = SECTOR_SIZE;

		char *ret = ata_read_sector((ata_dev_t *)node->device, counter);

		if (counter == start) {
			off = start_offset;
			read_size = SECTOR_SIZE - off;
		}
		if (counter == end)
			read_size = end_offset - off + 1;

		memcpy(buf_curr, ret + off, read_size);
		buf_curr = buf_curr + read_size;
		total = total + read_size;
		counter++;
	}
	return total;
}

uint32_t ata_write(vfs_node_t *node, uint32_t offset, uint32_t size, char *buf)
{
	uint32_t start = offset / SECTOR_SIZE;
	uint32_t start_offset = offset % SECTOR_SIZE;

	uint32_t end = (offset + size - 1) / SECTOR_SIZE;
	uint32_t end_offset = (offset + size - 1) % SECTOR_SIZE;

	char *buf_curr = buf;
	uint32_t counter = start;
	uint32_t write_size;
	uint32_t off, total = 0;

	while (counter <= end) {
		off = 0;
		write_size = SECTOR_SIZE;
		char *ret = ata_read_sector((ata_dev_t *)node->device, counter);
		if (counter == start) {
			off = start_offset;
			write_size = SECTOR_SIZE - off;
		}
		if (counter == end) {
			write_size = end_offset - off + 1;
		}
		memcpy(ret + off, buf_curr, write_size);
		ata_write_sector((ata_dev_t *)node->device, counter, ret);
		buf_curr = buf_curr + write_size;
		total = total + write_size;
		counter++;
	}
	return total;
}

void ata_write_sector(ata_dev_t *dev, uint32_t lba, char *buf)
{
	memcpy(dev->mem_buffer, buf, SECTOR_SIZE);

	outb(dev->BMR_COMMAND, 0);
	outl(dev->BMR_prdt, (uint32_t)dev->prdt_phys);
	outb(dev->drive, 0xe0 | dev->slave << 4 | (lba & 0x0f000000) >> 24);
	outb(dev->sector_count, 1);
	outb(dev->lba_lo, lba & 0x000000ff);
	outb(dev->lba_mid, (lba & 0x0000ff00) >> 8);
	outb(dev->lba_high, (lba & 0x00ff0000) >> 16);

	outb(dev->command, 0xCA);

	outb(dev->BMR_COMMAND, 0x1);

	while (1) {
		int status = inb(dev->BMR_STATUS);
		int dstatus = inb(dev->status);
		if (!(status & 0x04)) {
			continue;
		}
		if (!(dstatus & 0x80)) {
			break;
		}
	}
}

char *ata_read_sector(ata_dev_t *dev, uint32_t lba)
{
	char *buf = kmalloc(SECTOR_SIZE);

	outb(dev->BMR_COMMAND, 0);
	outl(dev->BMR_prdt, (uint32_t)dev->prdt_phys);
	outb(dev->drive, 0xe0 | dev->slave << 4 | (lba & 0x0f000000) >> 24);
	outb(dev->sector_count, 1);
	outb(dev->lba_lo, lba & 0x000000ff);
	outb(dev->lba_mid, (lba & 0x0000ff00) >> 8);
	outb(dev->lba_high, (lba & 0x00ff0000) >> 16);

	outb(dev->command, 0xC8);

	outb(dev->BMR_COMMAND, 0x8 | 0x1);

	while (1) {
		int status = inb(dev->BMR_STATUS);
		int dstatus = inb(dev->status);
		if (!(status & 0x04)) {
			continue;
		}
		if (!(dstatus & 0x80)) {
			break;
		}
	}

	memcpy(buf, dev->mem_buffer, SECTOR_SIZE);
	return buf;
}

vfs_node_t *create_ata_device(ata_dev_t *dev)
{
	vfs_node_t *t = kcalloc(sizeof(vfs_node_t), 1);
	strcpy(t->name, "ata device ");
	t->name[strlen(t->name)] = dev->mountpoint[strlen(dev->mountpoint) - 1];
	t->device = dev;
	t->flags = FS_BLOCKDEVICE;
	t->read = ata_read;
	t->write = ata_write;
	t->open = ata_open;
	t->close = ata_close;
	return t;
}

void ata_device_init(ata_dev_t *dev, int primary)
{
	dev->prdt = (void *)kmalloc(sizeof(prdt_t));
	memset(dev->prdt, 0, sizeof(prdt_t));
	dev->prdt_phys = (uint8_t *)paging_get_phys((uint32_t)dev->prdt);
	dev->mem_buffer = (void *)kmalloc(4096);
	memset(dev->mem_buffer, 0, 4096);

	dev->prdt[0].buffer_phys = (uint32_t)paging_get_phys((uint32_t)dev->mem_buffer);
	dev->prdt[0].transfer_size = SECTOR_SIZE;
	dev->prdt[0].mark_end = MARK_END;

	uint16_t base_addr = primary ? (0x1F0) : (0x170);
	uint16_t alt_status = primary ? (0x3F6) : (0x376);

	dev->data = base_addr;
	dev->error = base_addr + 1;
	dev->sector_count = base_addr + 2;
	dev->lba_lo = base_addr + 3;
	dev->lba_mid = base_addr + 4;
	dev->lba_high = base_addr + 5;
	dev->drive = base_addr + 6;
	dev->command = base_addr + 7;
	dev->alt_status = alt_status;

	dev->bar4 = pci_read_field(ata_device, PCI_BAR4, 4);
	if (dev->bar4 & 0x1) {
		dev->bar4 = dev->bar4 & 0xfffffffc;
	}
	dev->BMR_COMMAND = dev->bar4;
	dev->BMR_STATUS = dev->bar4 + 2;
	dev->BMR_prdt = dev->bar4 + 4;

	memset(dev->mountpoint, 0, 32);
	strcpy(dev->mountpoint, "/dev/hd");
	dev->mountpoint[strlen(dev->mountpoint)] = 'a' + (((!primary) << 1) | dev->slave);
}

void ata_device_detect(ata_dev_t *dev, int primary)
{
	ata_device_init(dev, primary);

	software_reset(dev);
	io_wait(dev);
	outb(dev->drive, (0xA + dev->slave) << 4);
	outb(dev->sector_count, 0);
	outb(dev->lba_lo, 0);
	outb(dev->lba_mid, 0);
	outb(dev->lba_high, 0);

	outb(dev->command, COMMAND_IDENTIFY);
	if (!inb(dev->status)) {
		log("Device does not exist");
		return;
	}

	uint8_t lba_lo = inb(dev->lba_lo);
	uint8_t lba_hi = inb(dev->lba_high);
	if (lba_lo != 0 || lba_hi != 0) {
		log("Device is not ata-compatible");
		return;
	}
	uint8_t drq = 0, err = 0;
	while (!drq && !err) {
		drq = inb(dev->status) & STATUS_DRQ;
		err = inb(dev->status) & STATUS_ERR;
	}
	if (err) {
		log("Error while polling");
		return;
	}

	for (int i = 0; i < 256; i++)
		inw(dev->data);

	uint32_t pci_command_reg = pci_read_field(ata_device, PCI_COMMAND, 2);
	if (!(pci_command_reg & (1 << 2))) {
		pci_command_reg |= (1 << 2);
		pci_write_field(ata_device, PCI_COMMAND, pci_command_reg);
	}

	log("Detected drive: %d", dev->drive);
	vfs_mount(dev->mountpoint, create_ata_device(dev));
}

void ata_find(uint32_t device, uint16_t vendor_id, uint16_t device_id, void *extra)
{
	if ((vendor_id == ATA_VENDOR_ID) && (device_id == ATA_DEVICE_ID))
		*((uint32_t *)extra) = device;
}

void ata_init()
{
	pci_scan(&ata_find, -1, &ata_device);

	irq_install_handler(32 + 14, ata_handler);

	ata_device_detect(&primary_master, 1);
	ata_device_detect(&primary_slave, 1);
	ata_device_detect(&secondary_master, 0);
	ata_device_detect(&secondary_slave, 0);
}
