#ifndef TUNER_H_
#define TUNER_H_

#include "eval.hpp"
#include "thread.hpp"
#include <fstream>

#define NUM_ENTRIES 1000000
namespace Yayo {
double sigmoid(double K, double E);

struct TTuple {
    int index      = 0;
    int whiteScore = 0;
    int blackScore = 0;
};

struct TEntry {
    int turn       = 0;
    int phase      = 0;
    int mgPhase    = 0;
    int egPhase    = 0;
    int staticEval = 0;
    int egEval;

    double result;

    EvalWeights w;
    TTuple *tuples;
    int nTuples = 0;

    void init(Board &board);
    double linearEval();
};

struct TunerEntries {
    TEntry *entries;

    TunerEntries(std::string file);

    ~TunerEntries() { delete[] entries; }

    double staticEvalErrors(double K);
    double computeOptimalK();
};

} // namespace Yayo
#endif // TUNER_H_
