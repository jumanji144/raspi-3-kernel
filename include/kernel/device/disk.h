#pragma once
#include <common.h>

#include <kernel/device/block.h>
#include <kernel/device/partition.h>
#include <kernel/block/part/table.h>

namespace dev {

    constexpr ioctl_cmd ioctl_rescan_partitions = def_ioctl("disk", 0);

    class disk_device : public blk_device {
    public:
        disk_device() = default;

        ssize_t read(addr offset, void* buffer, size_t size) override = 0;
        ssize_t write(addr offset, const void* buffer, size_t size) override = 0;

        ssize_t read(void* buffer, size_t size) override {
            return read(tell(), buffer, size);
        }
        ssize_t write(const void* buffer, size_t size) override {
            return write(tell(), buffer, size);
        }

        bool ioctl(ioctl_cmd request, void* arg) override;

        [[nodiscard]] addr tell() const override {
            return position;
        }
        void seek(addr offset) override {
            position = offset;
        };

        partition_dev& partition(u8 number) {
            return part_devs[number];
        }

        partition_dev& primary() {
            return partition(0);
        }
    private:
        void rescan_partitions();

        addr position = 0;
        blk::partition_table part_table {};
        util::list<partition_dev, blk::max_partitions> part_devs {};
    };

}