#include <kernel/main.h>
#include <kernel/peripheral/uart1.h>
#include <kernel/mailbox/board.h>
#include <kernel/gpu/buffer.h>
#include <kernel/peripheral/sd.h>

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
    using mailbox::command_type::set, mailbox::command_type::get;

    (void) dtb_ptr32;

    uart::init();

    uart::write("Hello, world!\n");

    gpu::buffer buffer = gpu::allocate_framebuffer(1024, 768, 32);

    buffer.clear({ 0, 0, 0 });

    buffer.puts("Hello, world!");

    emmc::sd sd;

    if (sd.init()) {
        uart::write("SD card initialized\n");
    } else {
        uart::write("Failed to initialize SD card\n");
        return;
    }

    u8 buf[512];

    if(sd.read(0, buf, 512)) {
        int i = 0;
        for (const auto &item: buf) {
            if (i++ % 16 == 0)
                uart::write("\n");
            uart::write("{:02x} ", item);
        }
    }


    return;
}