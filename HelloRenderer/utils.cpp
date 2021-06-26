#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int b32;

#define MAX_U32 UINT32_MAX
#define MIN_S16 INT16_MIN
#define MAX_S16 INT16_MAX

#define global_variable static
#define internal static

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

global_variable bool running = true;
