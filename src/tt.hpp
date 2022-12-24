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

#include "util.hpp"

namespace Yayo {

/*
** 0000 0000 0000 0000
**    gen
*/

struct TTHash {
    std::uint64_t key = 0;

    union {
        struct {
            std::int16_t eval;
            std::uint16_t info;
            std::uint16_t move;
            std::int16_t score;
        } data;

        std::uint64_t hash = 0;
    };

    [[nodiscard]] int score(int ply) const {
        if (data.score >= CHECKMATE) {
            return data.score - ply;
        } else if (data.score <= -CHECKMATE) {
            return data.score + ply;
        }

        return data.score;
    }

    void age(int gen);
    [[nodiscard]] unsigned flag() const { return data.info & 3u; }
    [[nodiscard]] unsigned depth() const { return (data.info >> 2u) & 255u; }
    int generation() const { return data.info >> 10u; }
    unsigned move() const { return data.move; }
    int eval() const { return data.eval; }
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
    void record(std::uint64_t key, int ply, unsigned short move, int depth,
                int eval, int score, bool pvNode, unsigned char flag);

    void reset();
    void increaseAge();
    int percentFull();
};

extern TTable tt;

} // namespace Yayo

#endif // TT_H_
