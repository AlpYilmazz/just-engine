#pragma once

typedef     unsigned char           uint8;
typedef     unsigned short          uint16;
typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;

typedef     char                    int8;
typedef     short                   int16;
typedef     int                     int32;
typedef     long long               int64;

typedef     float                   float32;
typedef     double                  float64;

typedef     unsigned char           byte;
// typedef     uint8                   bool;

#define STRUCT_ZERO_INIT {0}
#define LAZY_INIT {0}
#define LATER_INIT {0}
#define UNINIT {0}

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#define SIGNOF(a) ( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) )

typedef struct {
    uint32 width;
    uint32 height;
} ScreenSize;
