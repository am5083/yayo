#ifndef TUNER_H_
#define TUNER_H_

#include "eval.hpp"
#include "thread.hpp"
#include <fstream>

#ifndef NUM_ENTRIES
#define NUM_ENTRIES 500000
#endif
#ifndef MAX_EPOCHS
#define MAX_EPOCHS 100000
#endif
#ifndef BATCH_SIZE
#define BATCH_SIZE 500000
#endif
#ifndef LRRATE
#define LRRATE 0.10
#endif
#ifndef LRDROPRATE
#define LRDROPRATE 2.00
#endif
#ifndef LRSTEPRATE
#define LRSTEPRATE 500
#endif
#ifndef REPORTING
#define REPORTING 50
#endif
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
    explicit TunerEntries(const std::string& file);
    ~TunerEntries() { delete[] entries; }

    void runTuner();

  private:
    TEntry *entries;
    int numEntries = 0;

    double staticEvalErrors(double K);
    double tunedEvalErrors(double params[NUM_FEATURES][2], double K);
    double computeOptimalK();

    void computeGradient(double gradient[NUM_FEATURES][2],
                         double params[NUM_FEATURES][2], double K, int batch);

    static void updateSingleGradient(TEntry &entry, double gradient[NUM_FEATURES][2],
                              double params[NUM_FEATURES][2], double K);

    void initUntunedWeights(double params[NUM_FEATURES][2]);
};

} // namespace Yayo
#endif // TUNER_H_
