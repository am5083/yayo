#include "tt.hpp"

int Yayo::TPTable::probeHash(int ply, std::uint64_t key, int *move, int depth, int alpha, int beta) {
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

void Yayo::TPTable::recordHash(std::string fen, int ply, std::uint64_t key, int move, int depth, int score,
                               unsigned char flag) {
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
};

int Yayo::TPTable::hashfull() const { return double(double(n - overwrites) / double(TP_INIT_SIZE)) * 1000; }
