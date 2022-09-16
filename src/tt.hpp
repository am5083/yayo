#ifndef TT_H_
#define TT_H_
#include "board.hpp"
#include "util.hpp"
#include <cstdint>
#include <iostream>

#define TP_EXACT 0
#define TP_ALPHA 1
#define TP_BETA 2
#define TP_UNKNOWN 9999999
#define TP_INIT_SIZE 16000000

namespace Yayo {

struct TPHash {
    std::uint64_t key;
    short depth = 0;
    short move  = 0;
    int flag    = 0;
    int score   = 0;
};

struct TPTable {
    TPHash t[TP_INIT_SIZE];
    unsigned int n;
    unsigned int overwrites;
    unsigned int collisions;

    int probeHash(int ply, std::uint64_t key, int *move, int depth, int alpha, int beta);
    void recordHash(std::string fen, int ply, std::uint64_t key, int move, int depth, int score, unsigned char flag);
    int hashfull() const;
};

} // namespace Yayo

#endif // TT_H_
