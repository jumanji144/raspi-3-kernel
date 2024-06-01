#pragma once
#include <common.h>
#include <kernel/device/device.h>

namespace dev {

    // block device generic ioctl requests
    constexpr ioctl_cmd ioctl_get_sector_count = def_ioctl("block", 0);
    constexpr ioctl_cmd ioctl_get_sector_size = def_ioctl("block", 1);

    class blk_device : public device {
    public:
        blk_device() = default;

        virtual ssize_t read(addr offset, void* buffer, size_t size) {
            return 0;
        }
        virtual ssize_t write(addr offset, const void* buffer, size_t size) {
            return 0;
        }

        ssize_t read(void* buffer, size_t size) override {
            return read(tell(), buffer, size);
        }
        ssize_t write(const void* buffer, size_t size) override {
            return write(tell(), buffer, size);
        }

        bool ioctl(ioctl_cmd request, void* arg) override {
            return false;
        }

        [[nodiscard]] virtual addr tell() const {
            return 0;
        }
        virtual void seek(addr offset) {};

        [[nodiscard]] u32 sector_size() {
            return sector_size_;
        }

    protected:
        u32 sector_size_ { 512 };
    };
}