#ifndef TUNER_H_
#define TUNER_H_

#include "eval.hpp"
#include "thread.hpp"
#include <fstream>

#define NUM_ENTRIES 2333770
#define MAX_EPOCHS 100000
#define BATCH_SIZE 2333770
#define LRRATE 1
#define LRDROPRATE 1.50
#define LRSTEPRATE 200
#define REPORTING 50
#define NUM_FEATURES 487

namespace Yayo {
double sigmoid(double K, double E);

struct TTuple {
    int index = 0;
    int whiteScore = 0;
    int blackScore = 0;
};

struct TEntry {
    int turn = 0;
    int phase = 0;
    int mgPhase = 0;
    int egPhase = 0;
    int staticEval = 0;
    int egEval;

    double result;

    EvalWeights w;
    TTuple *tuples;
    int nTuples = 0;

    void init(Board &board, std::string fen);
    double linearEval(double params[NUM_FEATURES][2]);
};

class TunerEntries {
  public:
    TunerEntries(std::string file);
    ~TunerEntries() { delete[] entries; }

    void runTuner();

  private:
    TEntry *entries;

    double staticEvalErrors(double K);
    double tunedEvalErrors(double params[NUM_FEATURES][2], double K);
    double computeOptimalK();

    void computeGradient(double gradient[NUM_FEATURES][2],
                         double params[NUM_FEATURES][2], double K, int batch);

    void updateSingleGradient(TEntry &entry, double gradient[NUM_FEATURES][2],
                              double params[NUM_FEATURES][2], double K);

    void initUntunedWeights(double params[NUM_FEATURES][2]);
};

} // namespace Yayo
#endif // TUNER_H_
