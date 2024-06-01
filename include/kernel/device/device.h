#pragma once
#include <common.h>
#include <common/string.h>

namespace dev {

    using ioctl_cmd = u64;

    class device {
    public:
        device() = default;

        virtual bool init() {
            return false;
        }

        virtual ssize_t read(void* buffer, size_t size) {
            return 0;
        }
        virtual ssize_t write(const void* buffer, size_t size) {
            return 0;
        }

        virtual bool ioctl(ioctl_cmd request, void* arg) {
            return false;
        }

    };

    // ioctl generating function
    consteval ioctl_cmd def_ioctl(const char* name, u16 request) {
        size_t len = str::strlen(name);
        if (len > 6 || name == nullptr) unreachable();
        ioctl_cmd cmd = 0;
        ioctl_cmd tmp;
        // pack name into top bits and the request into the bottom bits
        for (int i = 0; i < len; i++) {
            tmp = name[i];
            cmd |= tmp << (i * 8);
        }
        tmp = request;
        cmd |= tmp << 48;
        return cmd;
    }

}