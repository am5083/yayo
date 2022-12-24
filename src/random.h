/*
**    Yayo is a UCI chess engine written by am5083 (am@kvasm.us)
**    Copyright (C) 2022 Ahmed Mohamed (am@kvasm.us)
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RANDOM_H_
#define RANDOM_H_
#include <cstdint>

static inline std::uint64_t xorshift64_s() {
    static std::uint64_t x = 1804289383;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

static inline std::uint64_t random_u64() {
    std::uint64_t n1, n2, n3, n4;
    n1 = xorshift64_s() & 0xFFFF;
    n2 = xorshift64_s() & 0xFFFFF;
    n3 = xorshift64_s() & 0xFFFF;
    n4 = xorshift64_s() & 0xFFFFF;
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

static inline std::uint64_t xs_lowbits() { return xorshift64_s() & xorshift64_s() & xorshift64_s(); }

#endif // RANDOM_H_
