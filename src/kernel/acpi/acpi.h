#ifndef MELVIX_ACPI_H
#define MELVIX_ACPI_H

/**
 * Initialize the ACP interface
 * @return 0 if successful, otherwise -1
 */
int acpi_install();

/**
 * Activate a ACPI based device reboot
 */
void reboot();

/**
 * Activate a ACPI based device shutdown/poweroff
 */
void acpi_poweroff();

struct RSDPtr {
    char Signature[8];
    char CheckSum;
    char OemID[6];
    char Revision;
    uint32_t *rsdt_address;
};

struct FACP {
    char Signature[4];
    uint32_t Length;
    char unneded1[40 - 8];
    uint32_t *DSDT;
    char unneded2[48 - 44];
    uint32_t *SMI_CMD;
    char ACPI_ENABLE;
    char ACPI_DISABLE;
    char unneded3[64 - 54];
    uint32_t *PM1a_CNT_BLK;
    uint32_t *PM1b_CNT_BLK;
    char unneded4[89 - 72];
    char PM1_CNT_LEN;
    char unneeded5[18];
    char century;
};

extern struct FACP *facp;

#endif
