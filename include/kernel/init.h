#pragma once
#include <common.h>
#include <kernel/exceptions.h>

#define jump(func) asm volatile("b " #func "\n")

const handler_func handlers[] = {
        exc_handler,
        exc_handler,
        exc_handler,
        exc_handler,
};

using exception_handler_vector = void (*)();

extern "C" [[maybe_unused]] u64 init_stack;
extern "C" u64 __data_start__;
extern "C" u64 __data_end__;
extern "C" u64 __data_load__;
extern "C" u32 __bss_start__;
extern "C" u32 __bss_end__;

extern "C" void init0();
extern "C" void init1();

[[gnu::section(".text.vectors")]] void sync_exception();
[[gnu::section(".text.vectors")]] void irq_exception();
[[gnu::section(".text.vectors")]] void fiq_exception();
[[gnu::section(".text.vectors")]] void serror_exception();

void common_exception(u8 type);