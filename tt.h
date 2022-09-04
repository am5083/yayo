#ifndef TT_H_
#define TT_H_
#include "board.h"
#include "util.h"
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
    int depth = 0;
    int flag  = 0;
    int move  = 0;
    int score = 0;
    std::string fen;
};

struct TPTable {
    TPHash t[TP_INIT_SIZE];
    unsigned int n;
    unsigned int overwrites;
    unsigned int collisions;

    constexpr int probeHash(int ply, std::uint64_t key, int *move, int depth, int alpha, int beta) {
        TPHash &p = t[key % TP_INIT_SIZE];

        if (p.key == key) {
            *move = p.move;
            if (p.depth >= depth) {
                const int mateScore = INF - MAX_PLY;
                if (p.score > mateScore)
                    p.score -= ply;
                else if (p.score < -mateScore)
                    p.score += ply;

                if (p.flag == TP_EXACT) {
                    return p.score;
                } else if (p.flag == TP_ALPHA && p.score <= alpha) {
                    return alpha;
                } else if (p.flag == TP_BETA && p.score >= beta) {
                    return beta;
                }
            }
        }

        return TP_UNKNOWN;
    };

    void recordHash(std::string fen, int ply, std::uint64_t key, int move, int depth, int score, unsigned char flag) {
        if (score == -ABORT_SCORE || score == ABORT_SCORE)
            return;

        TPHash &p = t[key % TP_INIT_SIZE];

        const int mateScore = INF - MAX_PLY;
        if (p.score > mateScore)
            p.score += ply;
        else if (p.score < -mateScore)
            p.score -= ply;

        if (p.key) {
            overwrites++;
        }

        n++;
        p.depth = depth;
        p.flag  = flag;
        p.key   = key;
        p.move  = move;
        p.score = score;
        p.fen   = fen;
    };

    constexpr int hashfull() const { return double(double(n - overwrites) / double(TP_INIT_SIZE)) * 1000; }
};

} // namespace Yayo

#endif // TT_H_
