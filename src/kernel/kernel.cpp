#include <kernel/main.h>
#include <kernel/peripheral/uart1.h>
#include <kernel/mailbox/board.h>

void print_board_info() {
    mailbox::property_message<board::serial_request, board::firmware_revision_request, board::board_model_request,
            board::board_revision_request, board::cpu_memory_request, board::video_memory_request> message
                { {}, {}, {}, {}, {}, {} };

    mailbox::call_property(message);

    uart::write("Board information:\n");
    mailbox::tag_type auto serial = message.get_tag<board::serial_request>();
    uart::write("Serial: {:016x}\n", (u64) serial.serial2 << 32 | serial.serial1);

    uart::write("Firmware revision: {:08x}\n", message.get_tag<board::firmware_revision_request>().revision);

    uart::write("Board mode/revision: {:08x}/{:08x}\n",
                message.get_tag<board::board_model_request>().model,
                message.get_tag<board::board_revision_request>().revision);

    mailbox::tag_type auto cpu_memory = message.get_tag<board::cpu_memory_request>();
    uart::write("CPU memory: {}MB at {:08x}\n", cpu_memory.size / 1024 / 1024, cpu_memory.base);

    mailbox::tag_type auto video_memory = message.get_tag<board::video_memory_request>();
    uart::write("Video memory: {}MB at {:08x}\n", video_memory.size / 1024 / 1024, video_memory.base);
}
extern "C" void kernel_main(u64 dtb_ptr32)
{

    (void) dtb_ptr32;

    uart::init();

    print_board_info();

    while (true) {
        uart::write(uart::read());
    }
}