#pragma once
#include <common.h>

#include <kernel/device/block.h>
#include <kernel/block/part/partition.h>

namespace dev {

    constexpr ioctl_cmd ioctl_get_partition_start = def_ioctl("partit", 0);
    constexpr ioctl_cmd ioctl_get_partition_end = def_ioctl("partit", 1);
    constexpr ioctl_cmd ioctl_get_partition_number = def_ioctl("partit", 2);

    /// delegating class for partition devices
    class partition_dev : public blk_device {
    public:
        explicit partition_dev(blk_device* parent, const blk::partition* part, u8 number) :
                device(parent), part(part), number(number) {
            start = part->start_sector * device->sector_size();
            end = part->end_sector * device->sector_size();
        }

        partition_dev() = default;

        ssize_t read(addr offset, void *buffer, size_t size) override {
            return device->read(offset + start, buffer, size);
        }

        ssize_t write(addr offset, const void *buffer, size_t size) override {
            return device->write(offset + start, buffer, size);
        }

        bool ioctl(ioctl_cmd request, void *arg) override;

        [[nodiscard]] addr tell() const override {
            return position;
        }

        void seek(addr offset) override {
            position = offset;
        }

    private:
        addr position = 0;

        // partition information
        addr start = 0; // in bytes
        addr end = 0; // in bytes

        blk_device* device {};
        const blk::partition* part {};
        u8 number = 0;
    };

}