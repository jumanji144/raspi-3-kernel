#include "kernel/mailbox/mailbox.h"

namespace mailbox {

    status get_status() {
        return peripheral::read<status>(mailbox::status_addr);
    }

    void send_message(message msg, int channel) {
        msg.channel = channel;
        // wait until we can talk to the VC
        while (get_status().full) { }

        peripheral::write<mailbox::message>(mailbox::write_addr, msg);
    }

    message read_message() {
        // wait until we can talk to the VC
        while (get_status().empty) { }

        return peripheral::read<mailbox::message>(mailbox::read_addr);
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