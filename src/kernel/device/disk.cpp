#include <kernel/device/disk.h>

static blk::partition_table_parser parsers[] = {
        blk::parse_mbr
};

void dev::disk_device::rescan_partitions() {
    // clear the partition table
    this->part_table.partitions.clear();

    this->part_table = blk::partition_table{};
    bool found = false;
    for (const auto &item: parsers) {
        if (!item)
            continue;
        if (item(this->part_table, this)) {
            found = true;
            break;
        }
    }

    if (!found) {
        // no partition table found, add the whole disk as a single partition
        u32 sectors = 0;
        if (!this->ioctl(ioctl_get_sector_count, &sectors)) {
            return;
        }

        blk::partition part = {
                .start_sector = 0,
                .end_sector = sectors,
                .type = 0,
        };
        this->part_table.partitions.add(part);
    }

    // create partition devices
    this->part_devs.clear();
    for (size_t i = 0; i < this->part_table.partitions.size(); i++) {
        this->part_devs.add(partition_dev(this, &this->part_table.partitions[i], i));
    }
}

bool dev::disk_device::ioctl(dev::ioctl_cmd request, void *arg) {
    switch (request) {
        case ioctl_rescan_partitions:
            rescan_partitions();
            return true;
        default:
            return blk_device::ioctl(request, arg);
    }
}