#ifndef MELVIX_ISO9660_H
#define MELVIX_ISO9660_H

#define ISO9660_ROOT_RECORD_OFFSET 156
#define ISO9660_DIR_RECORD_LENGTH 0
#define ISO9660_DIR_EAR_LBA 2
#define ISO9660_DIR_EAR_LENGTH 10
#define ISO9660_DIR_FILENAME_LENGTH 32
#define ISO9660_DIR_FILENAME 33

#include <stdint.h>

struct iso9660_entity {
	uint32_t lba;
	uint32_t length;
};

struct iso9660_entity *ISO9660_get(char **dirs, uint8_t dirs_sz);

uint8_t *ISO9660_read(struct iso9660_entity *entity);

#endif
