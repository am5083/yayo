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
#include <cassert>
#include <cstring>

namespace Yayo {

TTable tt;

void TTHash::age(int gen) {
    std::uint64_t x = hash ^ key;
    data.generation = gen;
    key = x ^ hash;
}

TTable::TTable() {
    maxEntries = 0;
    init(TP_INIT_SIZE);
}

TTable::~TTable() { delete[] table; }

void TTable::init(std::uint64_t len) {
    const std::uint64_t mbSize = len * 1024 * 1024;

    if (maxEntries)
        delete[] table;

    std::uint64_t n = mbSize / (sizeof(TTHash) * NUM_BUCKETS);

    if (n < sizeof(TTHash))
        n = NUM_BUCKETS;

    table = new (std::align_val_t(sizeof(TTHash)))
          TTHash[n * NUM_BUCKETS + NUM_BUCKETS];
    memset(table, 0, sizeof(TTHash) * (n * NUM_BUCKETS + NUM_BUCKETS));
    maxEntries = n;
}

void TTable::prefetch(std::uint64_t key) {
    int idx = (key % maxEntries) * NUM_BUCKETS;
    TTHash *bucket = table + idx;
    __builtin_prefetch(bucket);
}

bool TTable::probe(std::uint64_t key, TTHash &out) {
    int idx = (key % maxEntries) * NUM_BUCKETS;
    TTHash *bucket = table + idx;

    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (((bucket + i)->key ^ (bucket + i)->hash) == key) {
            (bucket + i)->age(age);
            out = *(bucket + i);
            return true;
        } else if ((bucket + i)->key == key) {
            out = *(bucket + i);
            ;
            return true;
        }
    }

    return false;
}

void TTable::record(std::uint64_t key, int ply, int move, int depth, int score,
                    unsigned char flag) {
    std::uint64_t index = (key % maxEntries) * NUM_BUCKETS;
    TTHash *bucket = table + index;

    if (score >= INF)
        score = TP_INF - ply;
    else if (score <= -INF)
        score = -TP_INF + ply;

    TTHash temp = {0};
    temp.data.depth = depth;
    temp.data.move = move;
    temp.data.score = score;
    temp.data.generation = age;
    temp.data.flag = flag;
    temp.key = key ^ temp.hash;

    if ((bucket->key ^ bucket->hash) == key) {
        if (flag == TP_EXACT || depth >= bucket->depth() - 3) {
            *bucket = temp;
        }
        return;
    }

    TTHash *rep = bucket;
    for (int i = 1; i < NUM_BUCKETS; i++) {
        if (((bucket + i)->key ^ (bucket + i)->hash) == key) {
            if (flag == TP_EXACT || depth >= (bucket + i)->depth() - 3) {
                *(bucket + i) = temp;
            } else if ((bucket + i)->generation() < rep->generation()) {
                rep = (bucket + i);
            }
        }
    }

    *rep = temp;
}

void TTable::reset() {
    for (int i = 0; i < maxEntries * NUM_BUCKETS; i++) {
        table[i].age(0);
    }
}

void TTable::increaseAge() {
    age++;
    if (age == 63) {
        age = 1;

        for (int i = 0; i < maxEntries * NUM_BUCKETS; i++) {
            table[i].age(0);
        }
    }
}

int TTable::percentFull() {
    int n = 0;
    for (int i = 0; i < 4000; i++) {
        if (table[i].generation() == age) {
            n++;
        }
    }

    return n / NUM_BUCKETS;
}

int Yayo::TPTable::probeHash(int ply, std::uint64_t key, int *move, int depth,
                             int alpha, int beta, bool qsearch) {
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
                return p.score;
            } else if (p.flag == TP_BETA && p.score >= beta) {
                return p.score;
            }
        }
    }

    return TP_UNKNOWN;
};

void Yayo::TPTable::recordHash(std::string fen, int ply, std::uint64_t key,
                               int move, int depth, int score,
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
    p.flag = flag;
    p.key = key;
    p.move = move;
    p.score = score;
};

int Yayo::TPTable::hashfull() const {
    return double(double(n - overwrites) / double(TP_INIT_SIZE)) * 1000;
}

void TPTable::clear() {
    n = 0;
    overwrites = 0;
    collisions = 0;
    memset(&t, 0, sizeof(t));
}
} // namespace Yayo
