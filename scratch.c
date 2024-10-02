#include "stdio.h"

typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;
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

int main() {

    printf("Auto padded   - Size: %d, Align: %d\n", sizeof(Test), _Alignof(Test));
    printf("Manual padded - Size: %d, Align: %d\n", sizeof(TestPadded), _Alignof(TestPadded));

    return 0;
}