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

struct RSD_ptr {
    char signature[8];
    char checksum;
    char oem_id[6];
    char revision;
    uint32_t *rsdt_address;
};

struct FADT {
    char signature[4];
    uint32_t length;
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

struct HPET {
    char signature[4];
    char unneeded[36];
    char base_address[12];
};

struct FADT *fadt;

struct HPET *hpet;

#endif
