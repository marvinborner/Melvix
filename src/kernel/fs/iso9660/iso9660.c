#include <stdint.h>
#include <kernel/lib/lib.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/fs/iso9660/iso9660.h>
#include <mlibc/stdlib.h>

struct iso9660_entity *ISO9660_get(char **dirs, uint8_t dirs_sz)
{
    ATAPI_read(1, 0x10);
    uint32_t last_len = *(uint32_t *) (
            ATAPI_PIO_BUFFER +
            ISO9660_ROOT_RECORD_OFFSET +
            ISO9660_DIR_EAR_LENGTH
    );
    uint32_t last_lba = *(uint32_t *) (
            ATAPI_PIO_BUFFER +
            ISO9660_ROOT_RECORD_OFFSET +
            ISO9660_DIR_EAR_LBA
    );

    for (uint8_t dirs_i = 0; dirs_i < dirs_sz; dirs_i++) {
        ATAPI_read((last_len % 2048 != 0) + (last_len / 2048), last_lba);

        uint8_t found = 0;
        for (uint32_t i = 0; i < last_len && !found;) {
            if (!*(uint8_t *) (ATAPI_PIO_BUFFER + i + ISO9660_DIR_RECORD_LENGTH))
                break;

            char *filename = (char *) (ATAPI_PIO_BUFFER + i + ISO9660_DIR_FILENAME);

            for (uint32_t j = 0; j < ISO9660_DIR_FILENAME_LENGTH; j++) {
                if (filename[j] == ';') {
                    filename[j] = 0;
                    break;
                }
            }

            if (strcmp(dirs[dirs_i], filename) == 0) {
                found = 1;
                last_lba = *(uint32_t *) (ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LBA);
                last_len = *(uint32_t *) (ATAPI_PIO_BUFFER + i + ISO9660_DIR_EAR_LENGTH);
            } else {
                i += *(uint8_t *) (ATAPI_PIO_BUFFER + i + ISO9660_DIR_RECORD_LENGTH);
            }
        }

        if (!found) {
            return (struct iso9660_entity *) 0;
        }
    }

    struct iso9660_entity *ret = (struct iso9660_entity *) kmalloc(sizeof(struct iso9660_entity));
    ret->lba = last_lba;
    ret->length = last_len;
    return ret;
}

uint8_t *ISO9660_read(struct iso9660_entity *entity)
{
    ATAPI_read((entity->length % 2048 != 0) + (entity->length / 2048), entity->lba);
    return (uint8_t *) ATAPI_PIO_BUFFER;
}