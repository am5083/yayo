#ifndef RANDOM_H_
#define RANDOM_H_
#include <cstdint>

static inline std::uint64_t xorshift64_s(void) {
    static std::uint64_t x = 1804289383;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

static inline std::uint64_t random_u64(void) {
    std::uint64_t n1, n2, n3, n4;
    n1 = xorshift64_s() & 0xFFFF;
    n2 = xorshift64_s() & 0xFFFFF;
    n3 = xorshift64_s() & 0xFFFF;
    n4 = xorshift64_s() & 0xFFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

static inline std::uint64_t xs_lowbits(void) { return xorshift64_s() & xorshift64_s() & xorshift64_s(); }

#endif // RANDOM_H_
