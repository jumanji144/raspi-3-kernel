#include <kernel/devices/emmc.h>
#include "common/memory/mem.h"

ssize_t dev::emmc_dev::read(addr offset, void *buffer, size_t size) {
    return common_io_op(offset, static_cast<u8*>(buffer), size, false);
}

ssize_t dev::emmc_dev::write(addr offset, const void *buffer, size_t size) {
    return common_io_op(offset, const_cast<u8*>(static_cast<const u8*>(buffer)), size, true);
}

ssize_t dev::emmc_dev::common_io_op(u64 address, u8 *buffer, size_t size, bool write) {
    u32 overflow[512 / sizeof(u32)]; // buffer for unaligned read / writes
    u32* buf = reinterpret_cast<u32*>(buffer);

    size_t overflow_size = size % device->block_size;
    size_t blocks = size / device->block_size;

    u32 lba = address;
    if (!device->byte_access_mode)
        lba /= device->block_size;

    // execute io operation
    if (!device->transfer_block(lba, buf, blocks, write)) {
        return -1;
    }

    // transfer overflow block
    if (overflow_size) {
        if (write)
            mem::copy(overflow, buffer + (size - overflow_size), overflow_size);

        if (!device->transfer_block(lba + blocks, overflow, 1, write))
            return -1;

        if (!write)
            mem::copy(buffer + (size - overflow_size), overflow, overflow_size);
    }

    return size;
}

bool dev::emmc_dev::ioctl(dev::ioctl_cmd request, void *arg) {
    switch (request) {
        // blk ioctl
        case dev::ioctl_get_sector_count:
            *reinterpret_cast<u32*>(arg) = device->block_count;
            return true;
        case dev::ioctl_get_sector_size:
            *reinterpret_cast<u32*>(arg) = device->block_size;
            return true;

        // emmc ioctl
        case ioctl_get_cid:
            mem::copy(arg, device->cid, 4);
            return true;
        case ioctl_get_status:
            *reinterpret_cast<u32*>(arg) = emmc::bus->status;
            return true;
        case ioctl_get_type:
            *reinterpret_cast<u32*>(arg) = device->host_version;
            return true;
        case ioctl_get_csd:
            mem::copy(arg, device->csd, 16);
            return true;
        default:
            return false;
    }
}

addr dev::emmc_dev::tell() const {
    return position;
}

void dev::emmc_dev::seek(addr offset) {
    position = offset;
}