#include <kernel/main.h>
#include <kernel/peripheral/uart1.h>
#include <kernel/mailbox/board.h>
#include <kernel/gpu/buffer.h>

struct test {
    u64 a;
};
extern "C" void kernel_main(u64 dtb_ptr32)
{

    (void) dtb_ptr32;
	uart::init();

    uart::write("Hello, kernel World!\n");

    u16 pad = 10;
    u8 pad3 = 34;

    uart::write((u64) pad);

    static_assert(type::alignment_of_v<mailbox::tag> == 1);

    mailbox::property_message<board::serial_request, board::firmware_revision_request, board::board_model_request,
            board::board_revision_request, board::cpu_memory_request, board::video_memory_request> message
                { {}, {}, {}, {}, {}, {} };

    mailbox::call_property(message);

    uart::write("Board information:\n");
    uart::write("Serial: ");
    mailbox::tag_type auto serial = message.get_tag<board::serial_request>();
    uart::write((u64) serial.serial2 << 32 | serial.serial1, 16, 16);
    uart::write("\n");

    uart::write("Firmware revision: ");
    uart::write(message.get_tag<board::firmware_revision_request>().revision, 16, 8);
    uart::write("\n");

    uart::write("Board mode + revision: ");
    uart::write(message.get_tag<board::board_model_request>().model, 16, 8);
    uart::write("/");
    uart::write(message.get_tag<board::board_revision_request>().revision, 16, 8);
    uart::write("\n");

    uart::write("CPU memory: ");
    auto cpu_memory = message.get_tag<board::cpu_memory_request>();
    uart::write((u64) cpu_memory.size / 1024 / 1024);
    uart::write("MB at ");
    uart::write(cpu_memory.base, 16, 8);
    uart::write("\n");

    uart::write("Video memory: ");
    auto video_memory = message.get_tag<board::video_memory_request>();
    uart::write((u64) video_memory.size / 1024 / 1024);
    uart::write("MB at ");
    uart::write(video_memory.base, 16, 8);

    while (true) {
        uart::write(uart::read());
    }
}