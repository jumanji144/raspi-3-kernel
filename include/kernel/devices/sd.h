#pragma once
#include <common.h>

#include <kernel/peripheral/sd.h>
#include <kernel/devices/emmc.h>

namespace dev {

    constexpr dev::ioctl_cmd ioctl_get_scr = dev::def_ioctl("sd", 0);

    class sd_dev : public dev::emmc_dev {
    public:
        explicit sd_dev(emmc::sd& device) : dev(device), dev::emmc_dev(nullptr) {
            this->device = &dev;
        }

        bool ioctl(dev::ioctl_cmd request, void *arg) override;

    protected:
        emmc::sd dev;
    };

}