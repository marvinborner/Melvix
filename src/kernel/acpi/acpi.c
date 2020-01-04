// Important specification: https://uefi.org/sites/default/files/resources/ACPI_6_2.pdf
// HPET: https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf

#include <kernel/io/io.h>
#include <kernel/lib/lib.h>
#include <kernel/timer/timer.h>
#include <kernel/acpi/acpi.h>
#include <stddef.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>

struct FADT *fadt;
uint32_t *SMI_CMD;
char ACPI_ENABLE;
char ACPI_DISABLE;
uint32_t *PM1a_CNT;
uint32_t *PM1b_CNT;
int SLP_TYPa;
int SLP_TYPb;
int SLP_EN;
int SCI_EN;
char PM1_CNT_LEN;

unsigned int *acpi_check_rsd_ptr(unsigned int *ptr)
{
    char *sig = "RSD PTR ";
    struct RSD_ptr *rsdp = (struct RSD_ptr *) ptr;
    char *bptr;
    char check = 0;
    unsigned int i;

    if (memcmp(sig, rsdp, 8) == 0) {
        bptr = (char *) ptr;
        for (i = 0; i < sizeof(struct RSD_ptr); i++) {
            check += *bptr;
            bptr++;
        }

        if (check == 0) {
            return (unsigned int *) rsdp->rsdt_address;
        }
    }

    return NULL;
}

unsigned int *acpi_get_rsd_ptr()
{
    unsigned int *addr;
    unsigned int *rsdp;

    for (addr = (unsigned int *) 0x000E0000; (int) addr < 0x00100000; addr += 0x10 / sizeof(addr)) {
        rsdp = acpi_check_rsd_ptr(addr);
        if (rsdp != NULL)
            return rsdp;
    }

    int ebda = *((short *) 0x40E);
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (addr = (unsigned int *) ebda; (int) addr < ebda + 1024; addr += 0x10 / sizeof(addr)) {
        rsdp = acpi_check_rsd_ptr(addr);
        if (rsdp != NULL)
            return rsdp;
    }

    return NULL;
}

int acpi_enable()
{
    if ((inw((unsigned int) PM1a_CNT) & SCI_EN) == 0) {
        if (SMI_CMD != 0 && ACPI_ENABLE != 0) {
            outb((unsigned int) SMI_CMD, ACPI_ENABLE); // Enable ACPI
            // Try 3s until ACPI is enabled
            int i;
            for (i = 0; i < 300; i++) {
                if ((inw((unsigned int) PM1a_CNT) & SCI_EN) == 1)
                    break;
                timer_wait(1);
            }
            if (PM1b_CNT != 0)
                for (; i < 300; i++) {
                    if ((inw((unsigned int) PM1b_CNT) & SCI_EN) == 1)
                        break;
                    timer_wait(1);
                }
            if (i < 300) {
                return 0; // Successfully enabled ACPI
            } else {
                serial_printf("ACPI couldn't be enabled!");
                return -1; // ACPI couldn't be enabled
            }
        } else {
            serial_printf("ACPI is not supported!");
            return -1; // ACPI is not supported
        }
    } else {
        serial_printf("ACPI was already enabled!");
        return 0; // ACPI was already enabled
    }
}

int acpi_install()
{
    unsigned int *ptr = acpi_get_rsd_ptr();

    int success = 0;

    if (ptr != NULL && memcmp(ptr, "RSDT", 4) == 0) {
        int entries = *(ptr + 1);
        entries = (entries - 36) / 4;
        ptr += 36 / 4;

        while (0 < entries--) {
            if (memcmp((unsigned int *) *ptr, "FACP", 4) == 0) {
                fadt = (struct FADT *) *ptr; // TODO: Allocate ACPI tables after paging (page fault)!
                if (memcmp((unsigned int *) fadt->DSDT, "DSDT", 4) == 0) {
                    char *S5Addr = (char *) fadt->DSDT + 36;
                    int dsdt_length = *(fadt->DSDT + 1) - 36;
                    while (0 < dsdt_length--) {
                        if (memcmp(S5Addr, "_S5_", 4) == 0)
                            break;
                        S5Addr++;
                    }
                    if (dsdt_length > 0) {
                        // TODO: Implement device detection via DSDT ACPI (p199 -> AML)
                        if ((*(S5Addr - 1) == 0x08 || (*(S5Addr - 2) == 0x08 && *(S5Addr - 1) == '\\')) &&
                            *(S5Addr + 4) == 0x12) {
                            S5Addr += 5;
                            S5Addr += ((*S5Addr & 0xC0) >> 6) + 2;

                            if (*S5Addr == 0x0A)
                                S5Addr++;
                            SLP_TYPa = *(S5Addr) << 10;
                            S5Addr++;

                            if (*S5Addr == 0x0A)
                                S5Addr++;
                            SLP_TYPb = *(S5Addr) << 10;

                            SMI_CMD = fadt->SMI_CMD;

                            ACPI_ENABLE = fadt->ACPI_ENABLE;
                            ACPI_DISABLE = fadt->ACPI_DISABLE;

                            PM1a_CNT = fadt->PM1a_CNT_BLK;
                            PM1b_CNT = fadt->PM1b_CNT_BLK;

                            PM1_CNT_LEN = fadt->PM1_CNT_LEN;

                            SLP_EN = 1 << 13;
                            SCI_EN = 1;

                            acpi_enable();
                            vga_log("Installed ACPI");

                            success = 1;
                        } // Else: \_S5 parse error
                    } // Else: \_S5 not present
                } // Else: DSDT invalid
            }
            if (memcmp((unsigned int *) *ptr, "HPET", 4) == 0) {
                hpet = (struct HPET *) *ptr;
                // serial_printf("%d", hpet->base_address);
            }
            ptr++;
        } // Else: no valid FADT present
    } else {
        serial_printf("ACPI is not supported!");
    }

    return success == 1 ? 0 : -1;
}

void acpi_poweroff()
{
    asm ("cli");
    if (SCI_EN == 0) {
        warn("ACPI shutdown is not supported\n");
        return;
    }

    // Send shutdown command
    outw((unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN);
    if (PM1b_CNT != 0)
        outw((unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN);
    else {
        outw(0xB004, 0x2000); // Bochs
        outw(0x604, 0x2000); // QEMU
        outw(0x4004, 0x3400); // VirtualBox
    }
}

void reboot()
{
    asm ("cli");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    halt_loop();
}
