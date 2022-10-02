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

#include "tt.hpp"
#include <cstring>

namespace Yayo {

int Yayo::TPTable::probeHash(int ply, std::uint64_t key, int *move, int depth, int alpha, int beta, bool qsearch) {
    TPHash &p = t[key % TP_INIT_SIZE];

    if (p.key == key) {
        *move = p.move;
        if (p.depth >= depth) {
            const int mateScore = INF - MAX_PLY;
            if (p.score > mateScore) {
                p.score -= ply;
            } else if (p.score < -mateScore) {
                p.score += ply;
            }

            if (p.flag == TP_EXACT) {
                if (qsearch)
                    return p.score;
                else
                    return TP_UNKNOWN;
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

void TPTable::clear() {
    n          = 0;
    overwrites = 0;
    collisions = 0;
    memset(&t, 0, sizeof(t));
}
} // namespace Yayo
