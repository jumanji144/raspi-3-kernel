#include "kernel/mailbox/mailbox.h"

namespace mailbox {

    status get_status() {
        u32 status = peripheral::read<u32>(mailbox::status_addr);
        return reinterpret_cast<mailbox::status&>(status);
    }

    void send_message(message msg, int channel) {
        msg.channel = channel;
        // wait until we can talk to the VC
        while (get_status().full) { }

        peripheral::write<u32>(mailbox::write_addr, reinterpret_cast<u32&>(msg));
    }

    message read_message() {
        // wait until we can talk to the VC
        while (get_status().empty) { }

        u32 msg = peripheral::read<u32>(mailbox::read_addr);
        return reinterpret_cast<message&>(msg);
    }

    bool call(message msg, int channel) {
        constexpr u32 max_retry = 1000000;
        send_message(msg, channel);

        for (u32 i = 0; i < max_retry; i++) {
            auto resp = read_message();
            if (resp.data == msg.data) {
                return resp.get_data()[1] == 0x80000000;
            }
        }
        return false;
    }
    

}