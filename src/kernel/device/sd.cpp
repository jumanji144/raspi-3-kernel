#include <kernel/devices/sd.h>
#include "common/memory/mem.h"

bool dev::sd_dev::ioctl(dev::ioctl_cmd request, void *arg) {
    switch (request) {
        case ioctl_get_scr:
            mem::copy(arg, &dev.scr, 8);
            return true;
        default:
            return emmc_dev::ioctl(request, arg);
    }
}