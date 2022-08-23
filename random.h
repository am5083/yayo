#ifndef RANDOM_H_
#define RANDOM_H_

typedef unsigned long long uint64_t;

inline uint64_t xorshift64_s(void) {
    static uint64_t x = 1804289383;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

constexpr uint64_t xorshift64__ct(int num) {
    uint64_t start = 0x25F4;

    for (int i = 0; i <= num; i++) {
        start ^= start >> 12;
        start ^= start << 25;
        start ^= start >> 27;
    }

    return start * 0x2545F4914F6CDD1DULL;
}

constexpr uint64_t xorshift64_ct(int num) {
    uint64_t start = 2915391149;

    for (int i = 0; i <= num; i++) {
        start ^= start >> 12;
        start ^= start << 25;
        start ^= start >> 27;
    }

    return start * 0x2545F4914F6CDD1DULL;
}

constexpr uint64_t random_u64_ct(int num) {
    uint64_t n1, n2, n3, n4;
    n1 = xorshift64_ct(num) & 0xFFFF;
    n2 = xorshift64_ct(num) & 0xFFFFF;
    n3 = xorshift64_ct(num) & 0xFFFF;
    n4 = xorshift64_ct(num) & 0xFFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
};

constexpr uint64_t random_u64__ct(int num) {
    uint64_t n1, n2, n3, n4;
    n1 = xorshift64__ct(num) & 0xFFFF;
    n2 = xorshift64__ct(num) & 0xFFFFF;
    n3 = xorshift64__ct(num) & 0xFFFF;
    n4 = xorshift64__ct(num) & 0xFFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
};

static inline uint64_t random_u64(void) {
    uint64_t n1, n2, n3, n4;
    n1 = xorshift64_s() & 0xFFFF;
    n2 = xorshift64_s() & 0xFFFFF;
    n3 = xorshift64_s() & 0xFFFF;
    n4 = xorshift64_s() & 0xFFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

static inline uint64_t xs_lowbits(void) { return xorshift64_s() & xorshift64_s() & xorshift64_s(); }

#endif // RANDOM_H_
