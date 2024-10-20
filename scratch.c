#include "stdlib.h"
#include "stdio.h"

typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;
typedef     uint64                  usize;
typedef     unsigned char           byte;

typedef struct {
    uint32 a;       // 4
    uint64 b;       // 8
    byte c;         // 1
    uint32 d[9];    // 4*9 = 36 => (4*2)*4 + 4
} Test;

typedef struct {
    uint32 a;       // 4
    byte pad1[4];       // 4
    uint64 b;       // 8
    byte c;         // 1
    byte pad2[3];       // 3
    uint32 d0[1];   // 4
    uint32 d[8];    // 32 = 4*8
} TestPadded;

usize addr_align_up(usize addr, usize align) {
    return (addr + align - 1) & ~(align - 1);
}

void* ptr_align_up(void* ptr, usize align) {
    return (void*) addr_align_up((usize) ptr, align);
}

int main() {

    printf("Auto padded   - Size: %d, Align: %d\n", sizeof(Test), _Alignof(Test));
    printf("Manual padded - Size: %d, Align: %d\n", sizeof(TestPadded), _Alignof(TestPadded));

    uint32* p = (uint32*)20;
    uint32* pa = ptr_align_up(p, 4);
    usize pa2 = addr_align_up((usize)p, 4);
    printf("pointer: %p\naddr usize: %u\naddr: %u\n", p, (usize)p, p);
    printf("---\n");
    printf("aligned: %p, %u\n", pa, pa);
    printf("aligned: %p, %u\n", pa2, pa2);

    return 0;
}