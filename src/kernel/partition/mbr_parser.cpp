#include <kernel/block/part/table.h>
#include "common/memory/unaligned.h"

struct [[gnu::packed]] chs {
    u8 head : 8;
    u8 sector : 6;
    u16 cylinder : 10;
};

struct [[gnu::packed]] partition_entry {
    u8 status;
    chs start_chs;
    u8 type;
    chs end_chs;
    u32 lba_start;
    u32 sectors;
};

struct [[gnu::packed]] mbr {
    u8 boot_code[446];
    partition_entry partitions[4];
    u16 signature;
};

bool blk::parse_mbr(blk::partition_table &table, dev::blk_device *device) {
    // read the MBR
    mbr mbr{};
    u32 sector_size = 0;
    device->ioctl(dev::ioctl_get_sector_size, &sector_size);

    if (device->read(0, &mbr, sizeof(mbr)) != sizeof(mbr)) {
        return false;
    }

    if (mbr.signature != 0xAA55) {
        return false;
    }

    for (const auto& item : mbr.partitions) {
        u32 nsecs = mem::get_unaligned_le32(&item.sectors);
        u32 start = mem::get_unaligned_le32(&item.lba_start);

        if (item.type == 0 || nsecs == 0) {
            continue;
        }

        partition part = {
                .start_sector = start,
                .end_sector = start + nsecs,
                .type = item.type,
        };

        table.partitions.add(part);
    }

    return true;
}