#include <kernel/io/io.h>
#include <kernel/lib/lib.h>
#include <kernel/timer/timer.h>
#include <kernel/acpi/acpi.h>
#include <stddef.h>

struct FACP *facp;
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

unsigned int *acpi_check_rsd_ptr(unsigned int *ptr) {
    char *sig = "RSD PTR ";
    struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
    char *bptr;
    char check = 0;
    unsigned int i;

    if (memory_compare(sig, rsdp, 8) == 0) {
        bptr = (char *) ptr;
        for (i = 0; i < sizeof(struct RSDPtr); i++) {
            check += *bptr;
            bptr++;
        }

        if (check == 0) {
            return (unsigned int *) rsdp->rsdt_address;
        }
    }

    return NULL;
}

unsigned int *acpi_get_rsd_ptr() {
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

int acpi_check_header(unsigned int *ptr, char *sig) {
    if (memory_compare(ptr, sig, 4) == 0) {
        char *checkPtr = (char *) ptr;
        int len = *(ptr + 1);
        char check = 0;
        while (0 < len--) {
            check += *checkPtr;
            checkPtr++;
        }
        if (check == 0)
            return 0;
    }
    return -1;
}

int acpi_enable() {
    if ((receive_w((unsigned int) PM1a_CNT) & SCI_EN) == 0) {
        if (SMI_CMD != 0 && ACPI_ENABLE != 0) {
            send_b((unsigned int) SMI_CMD, ACPI_ENABLE); // Enable ACPI
            // Try 3s until ACPI is enabled
            int i;
            for (i = 0; i < 300; i++) {
                if ((receive_w((unsigned int) PM1a_CNT) & SCI_EN) == 1)
                    break;
                timer_wait(1);
            }
            if (PM1b_CNT != 0)
                for (; i < 300; i++) {
                    if ((receive_w((unsigned int) PM1b_CNT) & SCI_EN) == 1)
                        break;
                    timer_wait(1);
                }
            if (i < 300) {
                return 0; // Successfully enabled ACPI
            } else {
                return -1; // ACPI couldn't be enabled
            }
        } else {
            return -1; // ACPI is not supported
        }
    } else {
        return 0; // ACPI was already enabled
    }
}

int acpi_install() {
    unsigned int *ptr = acpi_get_rsd_ptr();

    if (ptr != NULL && acpi_check_header(ptr, "RSDT") == 0) {
        int entrys = *(ptr + 1);
        entrys = (entrys - 36) / 4;
        ptr += 36 / 4;

        while (0 < entrys--) {
            if (acpi_check_header((unsigned int *) *ptr, "FACP") == 0) {
                entrys = -2;
                facp = (struct FACP *) *ptr;
                if (acpi_check_header((unsigned int *) facp->DSDT, "DSDT") == 0) {
                    char *S5Addr = (char *) facp->DSDT + 36;
                    int dsdt_length = *(facp->DSDT + 1) - 36;
                    while (0 < dsdt_length--) {
                        if (memory_compare(S5Addr, "_S5_", 4) == 0)
                            break;
                        S5Addr++;
                    }
                    if (dsdt_length > 0) {
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

                            SMI_CMD = facp->SMI_CMD;

                            ACPI_ENABLE = facp->ACPI_ENABLE;
                            ACPI_DISABLE = facp->ACPI_DISABLE;

                            PM1a_CNT = facp->PM1a_CNT_BLK;
                            PM1b_CNT = facp->PM1b_CNT_BLK;

                            PM1_CNT_LEN = facp->PM1_CNT_LEN;

                            SLP_EN = 1 << 13;
                            SCI_EN = 1;

                            return 0;
                        } // Else: \_S5 parse error
                    } // Else: \_S5 not present
                } // Else: DSDT invalid
            }
            ptr++;
        } // Else: no valid FACP present
    } // Else: No ACPI available
    return -1;
}

void acpi_poweroff() {
    acpi_enable();

    if (SCI_EN == 0) {
        serial_write("ACPI shutdown is not supported\n");
        return;
    }

    // Send shutdown command
    send_w((unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN);
    if (PM1b_CNT != 0)
        send_w((unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN);
    else {
        send_w(0xB004, 0x2000); // Bochs
        send_w(0x604, 0x2000); // QEMU
        send_w(0x4004, 0x3400); // VirtualBox
    }

    serial_write("Shutdown failed\n");
}

void reboot() {
    asm volatile ("cli");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = receive_b(0x64);
    send_b(0x64, 0xFE);
    loop:
    asm volatile ("hlt");
    goto loop;
}
