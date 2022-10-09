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
#define TP_INIT_SIZE 128

#define TP_INF 30000
#define NUM_BUCKETS 4

namespace Yayo {

/*
** 0000 0000 0000 0000
**    gen
*/

struct TTHash {
    std::uint64_t key;
    union {
        struct {
            std::uint8_t old;
            std::uint8_t flag;
            std::uint8_t generation;
            std::uint8_t depth;
            std::uint16_t move;
            std::int16_t score;
        } data;

        std::uint64_t hash = 0;
    };

    int score(int ply) const {
        if (data.score >= TP_INF) {
            return INF - ply;
        } else if (data.score <= -TP_INF) {
            return -INF + ply;
        }

        return data.score;
    }

    void age(int gen);
    int flag() const { return data.flag; }
    int depth() const { return data.depth; }
    int move() const { return data.move; }
    int generation() const { return data.generation; }
};

class TTable {
  public:
    TTHash *table;

    std::uint64_t maxEntries;
    int age = 1;

    TTable();
    ~TTable();

    void init(std::uint64_t len);

    void prefetch(std::uint64_t key);

    bool probe(std::uint64_t key, TTHash &out);
    void record(std::uint64_t key, int ply, int move, int depth, int score,
                unsigned char flag);

    void reset();
    void increaseAge();
    int percentFull();
};

struct TPHash {
    std::uint64_t key;
    short depth = 0;
    short move = 0;
    int flag = 0;
    int score = 0;
};

struct TPTable {
    TPHash t[TP_INIT_SIZE];
    unsigned int n;
    unsigned int overwrites;
    unsigned int collisions;

    void clear();
    int probeHash(int ply, std::uint64_t key, int *move, int depth, int alpha,
                  int beta, bool qsearch = false);
    void recordHash(std::string fen, int ply, std::uint64_t key, int move,
                    int depth, int score, unsigned char flag);
    int hashfull() const;
};

extern TTable tt;

} // namespace Yayo

#endif // TT_H_
