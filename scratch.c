#include "stdio.h"

#define SIZE 100

typedef struct {
    int a;
    int b;
    unsigned char base[SIZE];
} ArenaRegion;

typedef struct {
    int a;
    int b;
} ArenaRegion2;

int main() {
    ArenaRegion ar = {0};

    printf("Size: %d + %d\n", sizeof(ArenaRegion), SIZE);
    printf("Size 2: %d\n", sizeof(ArenaRegion2));

    printf("%p -> %p - %p - %p\n", &ar, ar.base, &ar.base, &ar.base[0]);

    return 0;
}