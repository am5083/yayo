#ifndef ZOBRIST_H_
#define ZOBRIST_H_
#include "random.h"
#include <cstdint>
#include <array>

namespace Yayo {
constexpr auto zobristBlackToMove = random_u64_ct(1);

constexpr auto zobristCastleRights{[]() consteval {
    std::array<uint64_t, 16> cr {};
    for (int i = 0; i < 16; i++) {
        int z = i+1;
        cr[i] = random_u64_ct(1+z);
    }
    return cr;
}()};

constexpr auto zobristEpFile{[]() consteval {
    std::array<uint64_t, 8> ep {};
    for (int i = 0; i < 8; i++) {
        int z = i+1;
        ep[i] = random_u64_ct((17*2) + z);
    }
    return ep;
}()};

constexpr auto ps1{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*1);
    }
    return t;
}()};

constexpr auto ps2{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*2);
    }
    return t;
}()};

constexpr auto ps3{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*3);
    }
    return t;
}()};

constexpr auto ps4{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*4);
    }
    return t;
}()};

constexpr auto ps5{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*5);
    }
    return t;
}()};

constexpr auto ps6{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*6);
    }
    return t;
}()};

constexpr auto ps7{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*7);
    }
    return t;
}()};

constexpr auto ps8{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*8);
    }
    return t;
}()};

constexpr auto ps9{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*9);
    }
    return t;
}()};

constexpr auto ps10{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*10);
    }
    return t;
}()};

constexpr auto ps11{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*11);
    }
    return t;
}()};

constexpr auto ps12{[]() consteval {
    std::array<uint64_t, 64> t = {0};
    for (int j = 0; j < 64; j++) {
        int z = j + 1;
        t[j] = random_u64__ct((z + j)*12);
    }
    return t;
}()};

constexpr std::array<std::array<uint64_t, 64>, 12> joinArrays() {
    std::array<std::array<uint64_t, 64>, 12> ps = {};

    ps[0] = ps1;
    ps[1] = ps2;
    ps[2] = ps3;
    ps[3] = ps4;
    ps[4] = ps5;
    ps[5] = ps6;
    ps[6] = ps7;
    ps[7] = ps8;
    ps[8] = ps9;
    ps[9] = ps10;
    ps[10] = ps11;
    ps[11] = ps12;

    return ps;
}


constexpr auto zobristPieceSq = joinArrays();

}
#endif // ZOBRIST_H_
