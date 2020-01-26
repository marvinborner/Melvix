#include <kernel/io/io.h>
#include <kernel/pci/pci.h>
#include <kernel/system.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/kheap.h>

int rtl_irq = 0;
uint8_t mac[6];
uint8_t *rtl_rx_buffer;
uint32_t rtl_iobase = 0;
uint32_t rtl_device_pci = 0x00000000;

void find_rtl(uint32_t device, uint16_t vendor_id, uint16_t device_id, void *extra)
{
    if ((vendor_id == 0x10ec) && (device_id == 0x8139)) {
        *((uint32_t *) extra) = device;
    }
}

void rtl8139_irq_handler(struct regs *r)
{
    serial_printf("RTL INT!");
    uint16_t status = inw((uint16_t) (rtl_iobase + 0x3E));
    if (!status) return;
    outw((uint16_t) (rtl_iobase + 0x3E), status);

    if (status & 0x01 || status & 0x02) {
        while ((inw((uint16_t) (rtl_iobase + 0x37)) & 0x01) == 0) {
            serial_printf("RECEIVE");
            // RECEIVE
        }
    }
}

int rtl8139_init(void)
{
    if (rtl_device_pci) {
        uint16_t command_reg = (uint16_t) pci_read_field(rtl_device_pci, PCI_COMMAND, 4);

        if (command_reg & (1 << 2)) {
        } else {
            command_reg |= (1 << 2);
            pci_write_field(rtl_device_pci, PCI_COMMAND, 4, command_reg);
        }

        rtl_irq = pci_get_interrupt(rtl_device_pci);
        irq_install_handler(rtl_irq, rtl8139_irq_handler);

        uint32_t rtl_bar0 = pci_read_field(rtl_device_pci, PCI_BAR0, 4);
        // uint32_t rtl_bar1 = pci_read_field(rtl_device_pci, PCI_BAR1, 4);

        rtl_iobase = 0x00000000;

        if (rtl_bar0 & 0x00000001)
            rtl_iobase = rtl_bar0 & 0xFFFFFFFC;
        else
            warn("RTL8139 should be using an I/O BAR!");

        // Get mac address
        for (int i = 0; i < 6; ++i)
            mac[i] = inb((uint16_t) (rtl_iobase + 0x00 + i));
        debug("Mac address: %2x:%2x:%2x:%2x:%2x:%2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        // Activate (turn on)
        outb((uint16_t) (rtl_iobase + 0x52), 0x0);

        // Reset
        outb((uint16_t) (rtl_iobase + 0x37), 0x10);
        while ((inb((uint16_t) (rtl_iobase + 0x37)) & 0x10) != 0) {}

        // Set receive buffer
        rtl_rx_buffer = (uint8_t *) kmalloc(8192 + 16);
        outl((uint16_t) (rtl_iobase + 0x30), (uintptr_t) rtl_rx_buffer);

        // Enable ISR
        outw((uint16_t) (rtl_iobase + 0x3C), 0x0005);

        // Accept packets
        outl((uint16_t) (rtl_iobase + 0x44), 0xf | (1 << 7));

        // Enable receive and transmitter
        outb((uint16_t) (rtl_iobase + 0x37), 0x0C);
    } else {
        return -1;
    }
    return 0;
}

void rtl8139_install()
{
    pci_scan(&find_rtl, -1, &rtl_device_pci);
    if (!rtl_device_pci) {
        warn("No rtl8139 network card found");
        return;
    }

    if (rtl8139_init() == 0)
        info("Installed rtl8139 network driver");
}