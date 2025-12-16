#include "sbi.h"

uint64_t sbi_call(uint64_t func_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret = 0;

    // 使用SBI ecall来设置timer
    register uint64_t a0 asm("a0") = arg0;
    register uint64_t a1 asm("a1") = arg1;
    register uint64_t a2 asm("a2") = arg2;
    register uint64_t a7 asm("a7") = func_id;

    asm volatile(
        "ecall"
        : "=r"(a0)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a7)
        : "memory"
    );

    ret = a0;
    return ret;
}
