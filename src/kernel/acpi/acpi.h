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

#endif
