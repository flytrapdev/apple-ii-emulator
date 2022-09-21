#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

#define REVERSE_BITS(a) ((((a) & 0x01) << 1) | (((a) & 0x02) >> 1))

#define MIN(a,b) ( ((a) > (b)) ? b : a )
#define MAX(a,b) ( ((a) > (b)) ? a : b )

typedef uint8_t byte;
typedef int8_t signed_byte;
typedef uint16_t word;
typedef uint32_t uint32;

#endif