#include <kernel/peripheral/uart1.h>
#include <kernel/exceptions.h>

const char * const exception_classes[65] = {
        "Unknown", "Trapped WFI or WFE instruction execution", "Unknown", "Trapped MCR or MRC access",
        "Trapped MCRR or MRRC access", "Trapped MCR or MRC access (64-bit)", "Trapped LDC or STC access",
        "Trapped access to SVE, Advanced SIMD, or floating-point functionality", "Unknown", "Unknown",
        "Trapped access to LD64B, ST64B, LDST128, or PRFM functionality", "Unknown", "Trapped MRRC access (64-bit)",
        "Branch target exception", "Illegal execution state", "Unknown", "Unknown", "Syscall (32-bit)",
        "Syscall (64-bit)", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown",
        "Trapped MSR, MRS, or System instruction execution", "Trapped SVE, Advanced SIMD, or floating-point instruction execution",
        "Unknown", "Unknown", "Pointer authentication failure", "Unknown", "Unknown", "Unknown",
        "Instruction abort from a lower Exception level", "Instruction abort from the same Exception level",
        "PC alignment fault", "Unknown", "Data abort from a lower Exception level", "Data abort from the same Exception level",
        "Stack alignment check", "Unknown", "Floating-point exception (32-bit)", "Unknown",
        "Unknown", "Unknown", "Floating-point exception (64-bit)", "Unknown", "Unknown",
        "SError interrupt", "Breakpoint from a lower Exception level", "Breakpoint from the same Exception level",
        "Software step from a lower Exception level", "Software step from the same Exception level",
        "Watchpoint from a lower Exception level", "Watchpoint from the same Exception level", "Unknown",
        "Unknown", "BKPT instruction execution (32-bit)", "Unknown", "Unknown", "Unknown",
        "BKPT instruction execution (64-bit)", "Unknown", "Unknown", "Unknown", "Unknown",
};
extern "C" void exc_handler(u64 type, u64 esr, u64 elr, u64 spsr, u64 far) {
    uart::write("Exception: ");
    switch (type) {
        case 0: uart::write("Synchronous"); break;
        case 1: uart::write("IRQ"); break;
        case 2: uart::write("FIQ"); break;
        case 3: uart::write("SError"); break;
    }
    uart::write("\n");

    u32 ec = esr >> 26;
    uart::write("Exception class: ");
    if (ec < 65) {
        uart::write(exception_classes[ec]);
    } else {
        uart::write("Unknown");
    }
    if(ec == 0b100100 || ec == 0b100101) {
        uart::write(", Fault cause: ");
        switch ((esr >> 2 ) & 3) {
            case 0: uart::write("Address size fault"); break;
            case 1: uart::write("Translation fault"); break;
            case 2: uart::write("Access flag fault"); break;
            case 3: uart::write("Permission fault"); break;
        }
        switch (esr & 3) {
            case 0: uart::write(" at level 0"); break;
            case 1: uart::write(" at level 1"); break;
            case 2: uart::write(" at level 2"); break;
            case 3: uart::write(" at level 3"); break;
        }
    }
    // dump registers
    uart::write("\nESR_EL1: ");
    uart::write(esr >> 32, 16, 8);
    uart::write(esr, 16, 8);
    uart::write("\nELR_EL1: ");
    uart::write(elr >> 32, 16, 8);
    uart::write(elr, 16, 8);
    uart::write("\nSPSR_EL1: ");
    uart::write(spsr >> 32, 16, 8);
    uart::write(spsr, 16, 8);
    uart::write("\nFAR_EL1: ");
    uart::write(far >> 32, 16, 8);
    uart::write(far, 16, 8);

    while (true) { }
}