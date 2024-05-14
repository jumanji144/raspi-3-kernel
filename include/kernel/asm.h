#pragma once
#include <common.h>

#define stp_stack(reg1, reg2, stack) asm volatile("stp x" #reg1 ", x" #reg2 ", [sp, #" #stack "]")
#define ltp_stack(reg1, reg2, stack) asm volatile("ldp x" #reg1 ", x" #reg2 ", [sp, #" #stack "]")

static inline u64 mpidr_el1() {
    u64 mpidr;
    asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    return mpidr;
}

static inline u64 current_el() {
    u64 el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    return el;
}

static inline void mpidr_el1(u64 mpidr) {
    asm volatile("msr mpidr_el1, %0" : : "r"(mpidr));
}

static inline void scr_el3(u64 scr) {
    asm volatile("msr scr_el3, %0" : : "r"(scr));
}

static inline void sctlr_el3(u64 sctlr) {
    asm volatile("msr sctlr_el3, %0" : : "r"(sctlr));
}

static inline void elr_el3(u64 elr) {
    asm volatile("msr elr_el3, %0" : : "r"(elr));
}

static inline void cnthctl_el2(u64 cnthctl) {
    asm volatile("msr cnthctl_el2, %0" : : "r"(cnthctl));
}

static inline u64 cnthctl_el2() {
    u64 cnthctl;
    asm volatile("mrs %0, cnthctl_el2" : "=r"(cnthctl));
    return cnthctl;
}

static inline void cntvoff_el2(u64 cntvoff) {
    asm volatile("msr cntvoff_el2, %0" : : "r"(cntvoff));
}

static inline void hcr_el2(u64 hcr) {
    asm volatile("msr hcr_el2, %0" : : "r"(hcr));
}

static inline u64 hcr_el2() {
    u64 hcr;
    asm volatile("mrs %0, hcr_el2" : "=r"(hcr));
    return hcr;
}

static inline void elr_el2(u64 elr) {
    asm volatile("msr elr_el2, %0" : : "r"(elr));
}

static inline void spsr_el2(u64 spsr) {
    asm volatile("msr spsr_el2, %0" : : "r"(spsr));
}

static inline void sctlr_el1(u64 sctlr) {
    asm volatile("msr sctlr_el1, %0" : : "r"(sctlr));
}

static inline void vbar_el1(u64 vbar) {
    asm volatile("msr vbar_el1, %0" : : "r"(vbar));
}

static inline void sp_el1(u64 sp) {
    asm volatile("msr sp_el1, %0" : : "r"(sp));
}

static inline u64 cntfrq_el0() {
    u64 cntfrq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    return cntfrq;
}

static inline u64 cntpct_el0() {
    u64 cntpct;
    asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));
    return cntpct;
}

static inline void eret() {
    asm volatile("eret");
}

static inline void nop() {
    asm volatile("nop");
}

static inline void halt() {
    while (true) {
        asm volatile("wfi");
    }
}