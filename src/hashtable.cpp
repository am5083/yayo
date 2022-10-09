#include "hashtable.hpp"
#include <new>
namespace Yayo {

HashTable::HashTable pawnHashTable;
HashTable::HashTable evalHashTable;
HashTable::HashTable mobilityHashTable;

namespace HashTable {
HashTable::HashTable() {
    numEntries = 0;
    init(16); // 16mb
}

HashTable::~HashTable() { delete[] table; }

void HashTable::init(std::uint64_t size) {

    const std::uint64_t mbSize = size * 1024 * 1024;

    if (numEntries)
        delete[] table;

    std::uint64_t n = mbSize / (sizeof(EvalHash) * NUM_BUCKETS);

    if (n < sizeof(EvalHash))
        n = NUM_BUCKETS;

    table = new (std::align_val_t(sizeof(EvalHash)))
          EvalHash[n * NUM_BUCKETS + NUM_BUCKETS];
    memset(table, 0, sizeof(EvalHash) * (n * NUM_BUCKETS + NUM_BUCKETS));
    numEntries = n;
}

void HashTable::prefetch(std::uint64_t key) {
    int idx = (key % numEntries) * NUM_BUCKETS;
    EvalHash *bucket = table + idx;
    __builtin_prefetch(bucket);
}

bool HashTable::probe(std::uint64_t key, EvalHash &out) {
    int idx = (key % numEntries) * NUM_BUCKETS;
    EvalHash *bucket = table + idx;

    for (int i = 0; i < NUM_BUCKETS; i++) {
        if (((bucket + i)->key ^ (bucket + i)->hash) == key) {
            (bucket + i)->age(gen);
            out = *(bucket + i);
            return true;
        }
    }

    return false;
}

void HashTable::record(std::uint64_t key, int eval) {
    std::uint64_t index = (key % numEntries) * NUM_BUCKETS;
    EvalHash *bucket = table + index;

    EvalHash temp = {0};
    temp.data.score = eval;
    temp.data.generation = gen;
    temp.key = key ^ temp.hash;

    if ((bucket->key ^ bucket->hash) == key) {
        *bucket = temp;
        return;
    }

    EvalHash *rep = bucket;
    for (int i = 1; i < NUM_BUCKETS; i++) {
        if (((bucket + i)->key ^ (bucket + i)->hash) == key) {
            *(bucket + i) = temp;
        } else if ((bucket + i)->generation() < rep->generation()) {
            rep = (bucket + i);
        }
    }

    *rep = temp;
}

void HashTable::age() {
    gen++;

    if (gen == 63) {
        gen = 1;

        for (int i = 0; i < numEntries * NUM_BUCKETS; i++) {
            table[i].age(0);
        }
    }
}
} // namespace HashTable
} // namespace Yayo
