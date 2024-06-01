#pragma once
#include <common.h>

#include <kernel/peripheral/emmc.h>
#include <kernel/device/disk.h>

namespace dev {

    // ioctl commands
    constexpr dev::ioctl_cmd ioctl_get_cid = dev::def_ioctl("emmc", 0);
    constexpr dev::ioctl_cmd ioctl_get_status = dev::def_ioctl("emmc", 1);
    constexpr dev::ioctl_cmd ioctl_get_type = dev::def_ioctl("emmc", 2);
    constexpr dev::ioctl_cmd ioctl_get_csd = dev::def_ioctl("emmc", 3);

    class emmc_dev : public disk_device {
    public:
        explicit emmc_dev(emmc::device* device) : device(device) {}

        bool init() override;

        ssize_t read(addr offset, void* buffer, size_t size) override;
        ssize_t write(addr offset, const void* buffer, size_t size) override;

        bool ioctl(ioctl_cmd request, void* arg) override;

    protected:
        ssize_t common_io_op(u64 address, u8* buffer, size_t size, bool write);
        emmc::device* device;
    };

}