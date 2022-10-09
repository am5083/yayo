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

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <cstdint>
#include <cstring>
#define NUM_BUCKETS 4

namespace Yayo {
namespace HashTable {
struct EvalHash {
    std::uint64_t key;

    union {
        struct {
            std::uint32_t generation;
            std::uint32_t score;
        } data;

        std::uint64_t hash;
    };

    void age(int gen) {
        std::uint64_t x = hash ^ key;
        data.generation = gen;
        key = x ^ hash;
    }

    int score() { return data.score; }
    int generation() { return data.generation; }
};

class HashTable {
  public:
    EvalHash *table;

    std::uint64_t numEntries;
    int gen = 1;

    HashTable();
    ~HashTable();

    void init(std::uint64_t size);
    void prefetch(std::uint64_t key);
    bool probe(std::uint64_t key, EvalHash &res);
    void record(std::uint64_t key, int eval);
    void reset();
    void age();
};

} // namespace HashTable

extern HashTable::HashTable pawnHashTable;
extern HashTable::HashTable evalHashTable;
extern HashTable::HashTable mobilityHashTable;

} // namespace Yayo

#endif // HASHTABLE_H_
