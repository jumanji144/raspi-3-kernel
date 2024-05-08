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

    board::serial_response response = mailbox::call_property<>(mailbox::property_message<board::serial_request>())
            .get_property<board::serial_response>().get_tag();

    // recombine, struct cannot contain u64, as it cannot be aligned to 8 bytes
    u64 full_serial = (u64) response.serial2 << 32 | response.serial1;

    u32 firmware_revision = mailbox::call_property(mailbox::property_message<board::firmware_revision_request>())
            .get_property<board::firmware_revision_response>().get_tag().revision;

    u32 board_model = mailbox::call_property(mailbox::property_message<board::board_model_request>())
            .get_property<board::board_model_response>().get_tag().model;

    u32 board_revision = mailbox::call_property(mailbox::property_message<board::board_revision_request>())
            .get_property<board::board_revision_response>().get_tag().revision;

    board::cpu_memory_response cpu_memory = mailbox::call_property(mailbox::property_message<board::cpu_memory_request>())
            .get_property<board::cpu_memory_response>().get_tag();

    board::video_memory_response video_memory = mailbox::call_property(mailbox::property_message<board::video_memory_request>())
            .get_property<board::video_memory_response>().get_tag();

    uart::write("Board information:\n");
    uart::write("Serial: ");
    uart::write(full_serial, 16, 16);
    uart::write("\n");

    uart::write("Firmware revision: ");
    uart::write(firmware_revision, 16, 8);
    uart::write("\n");

    uart::write("Board mode + revision: ");
    uart::write(board_model, 16, 8);
    uart::write("/");
    uart::write(board_revision, 16, 8);
    uart::write("\n");

    uart::write("CPU memory: ");
    uart::write((u64) cpu_memory.size / 1024 / 1024);
    uart::write("MB at ");
    uart::write(cpu_memory.base, 16, 8);
    uart::write("\n");

    uart::write("Video memory: ");
    uart::write((u64) video_memory.size / 1024 / 1024);
    uart::write("MB at ");
    uart::write(video_memory.base, 16, 8);

    while (true) {
        uart::write(uart::read());
    }
}