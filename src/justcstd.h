#pragma once

#define STD_EXIT_SUCCESS 0
#define STD_EXIT_FAILURE 1

typedef unsigned long long size_t;

// stdlib.h

void std_exit(int _Code);
void* std_malloc(size_t _Size);
void* std_realloc(void *_Memory, size_t _NewSize);
void std_free(void *_Memory);

// string.h

void* std_memcpy(void *__restrict__ _Dst, const void *__restrict__ _Src, size_t _Size);

// stdio.h

int std_snprintf(char *__restrict__ __stream, size_t __n, const char *__restrict__ __format, ...);
