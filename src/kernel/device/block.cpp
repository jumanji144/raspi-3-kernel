#include <kernel/device/block.h>
#include <kernel/device/partition.h>

bool dev::partition_dev::ioctl(dev::ioctl_cmd request, void *arg) {
    switch (request) {
        case ioctl_get_partition_start:
            *reinterpret_cast<addr*>(arg) = part->start_sector;
            return true;
        case ioctl_get_partition_end:
            *reinterpret_cast<addr*>(arg) = part->end_sector;
            return true;
        case ioctl_get_partition_number:
            *reinterpret_cast<u32*>(arg) = number;
            return true;
        default:
            return device->ioctl(request, arg);
    }
}