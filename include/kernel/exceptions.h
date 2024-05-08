#pragma once
#include <common.h>

struct exception_vector {
    u64 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
    u64 x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
    u64 type;
    u64 esr;
    u64 elr;
    u64 spsr;
    u64 far;
};

extern "C" void exc_handler(u64 type, u64 esr, u64 elr, u64 spsr, u64 far);