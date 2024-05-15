#pragma once
#include "common.h"
#include "common/memory/mem.h"
#include "common/string.h"
#include <kernel/peripheral/peripheral.h>
#include <kernel/peripheral/uart1.h>
#include <common/types/type_traits.h>

namespace mailbox {

    struct status {
        u32 reserved : 30;
        u8 empty : 1;
        u8 full : 1;
    };

    enum response : u32 {
        request = 0,
        success = 0x80000000,
        error = 0x80000001
    };

    enum device : u16 {
        video_core = 0,
        board = 1,
        power = 2,
        clock = 3,
        framebuffer = 4,
        config = 5,
        resources = 6
    };

    enum command_type : u8 {
        get = 0,
        test = 4,
        set = 8
    };

    union command {
        struct {
            u16 cmd : 12;
            command_type type : 4;
            device dev : 16;
        };
        u32 raw;

        constexpr command(device dev, command_type type, u16 cmd) : cmd(cmd), type(type), dev(dev) {}
    };

    struct [[gnu::packed]] tag {
        u32 id;
        u32 size;
        u32 code;

        constexpr tag(command cmd, u32 size, u32 code) : id(cmd.raw), size(size), code(code) {}
    };

    template <typename T>
    concept tag_type = type::is_base_of_v<tag, T>;

    enum clock_id : u8 {
        emmc = 1,
        uart = 2,
        arm = 3,
        core = 4,
        v3d = 5,
        h264 = 6,
        isp = 7,
        sdram = 8,
        pixel = 9,
        pwm = 10,
        hevc = 11,
        emmc2 = 12,
        m2mc = 13,
        pixel_bv = 14
    };

    template <command_type type>
    struct clock_rate : tag {
        clock_id cid;
        u32 rate;
        u32 skip_turbo;

        constexpr clock_rate(clock_id id, u32 rate, u32 skip_turbo = 0)
            : tag(command { device::clock, type, 2 }, 12, 8),
            cid(id), rate(rate), skip_turbo(skip_turbo) {}

        constexpr clock_rate() : clock_rate(clock_id::emmc, 0) {}
    };

    template<tag_type... Tags>
    struct [[gnu::aligned(16)]] property_message {
        constexpr static size_t total_tag_size = (0 + ... + align_up(sizeof(Tags), sizeof(u32)));
        constexpr static size_t buf_size = align_up(2 * sizeof(u32) + total_tag_size, 1 << 4);
        constexpr static size_t tags_size = buf_size - 8;

        u32 buffer_size = buf_size;
        u32 code = response::request;
        u32 tags[tags_size / sizeof(u32)] {};

        // constructor to construct tag
        explicit property_message(Tags... tags) {
            u32* ptr = this->tags;
            ((mem::copy(ptr, &tags, align_up(sizeof(Tags), sizeof(u32))),
                    ptr += (align_up(sizeof(Tags), sizeof(u32)) / sizeof(u32))) , ...);
        }

        template<tag_type Tag>
        Tag get_tag() const {
            static_assert(type::is_present_v<Tag, Tags...>, "Tag not found");
            size_t size = 0;
            ((!type::is_same_v<Tag, Tags>
                    && (size += (align_up(sizeof(Tags), sizeof(u32)) / sizeof(u32)), true)) && ...);
            // bit cast
            return mem::bit_cast<Tag>(tags[size]);
        }
    };

    struct message {
        u8 channel : 4;
        u32 data : 28;

        u32* get_data() const {
            return (u32*)(addr)(data << 4);
        }

        template<typename T>
        property_message<T> get_property() const {
            return *(property_message<T>*)get_data();
        }
    } __attribute__((packed));

    constexpr addr read_addr = peripheral::mailbox + 0x00;
    constexpr addr status_addr = peripheral::mailbox + 0x18;
    constexpr addr write_addr = peripheral::mailbox + 0x20;

    enum channel : u8 {
        power_management = 0,
        virtual_uart = 2,
        vchiq = 3,
        leds = 4,
        buttons = 5,
        touchscreen = 6,
        property_tags = 8
    };

    //static constexpr size_t max_message_size = 128;
    //static u8 __attribute__((aligned(16))) read_buffer[max_message_size];

    template<typename... T>
    message create_property_message(property_message<T...>& msg) {
        return message { channel::property_tags, (u32)(addr)&msg >> 4 };
    }
    message read_message();
    status get_status();
    void send_message(message msg, int channel);
    bool call(message msg, int channel);

    template<typename... T>
    message call_property(property_message<T...>& msg) {
        auto msg_ = create_property_message(msg);
        send_message(msg_, channel::property_tags);
        return read_message();
    }

}