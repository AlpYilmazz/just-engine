#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "justcstd.h"

// stdlib.h

void std_exit(int _Code) {
    exit(_Code);
}

void* std_malloc(size_t _Size) {
    return malloc(_Size);
}

void* std_realloc(void *_Memory, size_t _NewSize) {
    return realloc(_Memory, _NewSize);
}

void std_free(void *_Memory) {
    free(_Memory);
}

// string.h

void* std_memcpy(void *__restrict__ _Dst, const void *__restrict__ _Src, size_t _Size) {
    return memcpy(_Dst, _Src, _Size);
}

// stdio.h

int std_snprintf(char *__restrict__ __stream, size_t __n, const char *__restrict__ __format, ...) {
    va_list args;
    va_start(args, __format);
    int count = vsnprintf(__stream, __n, __format, args);
    va_end(args);
    return count;
}
