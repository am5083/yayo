#ifndef TUNER_H_
#define TUNER_H_

#include "eval.hpp"
#include "thread.hpp"
#include <fstream>

#define NUM_ENTRIES 1000000
#define MAX_EPOCHS 100000
#define BATCH_SIZE 20000
#define LRRATE 0.10
#define LRDROPRATE 1.00
#define LRSTEPRATE 250
#define REPORTING 50

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
    double linearEval(double params[487][2]);
};

struct TunerEntries {
    TEntry *entries;

    TunerEntries(std::string file);

    ~TunerEntries() { delete[] entries; }

    double staticEvalErrors(double K);
    double tunedEvalErrors(double params[487][2], double K) {
        double total = 0.0;
        for (int i = 0; i < NUM_ENTRIES; i++) {
            total += pow(entries[i].result - sigmoid(K, entries[i].linearEval(params)), 2.0);
        }
        return total / (double)NUM_ENTRIES;
    }
    double computeOptimalK();

    void computeGradient(double gradient[487][2], double params[487][2], double K, int batch) {
        double local[487][2] = {{0}};
        for (int i = batch * BATCH_SIZE; i < (batch + 1) * BATCH_SIZE; i++) {
            updateSingleGradient(&entries[i], local, params, K);

            for (int i = 0; i < 487; i++) {
                gradient[i][0] += local[i][0];
                gradient[i][1] += local[i][1];
            }
        }
    }

    void updateSingleGradient(TEntry *entry, double gradient[487][2], double params[487][2], double K) {
        double E = entry->linearEval(params);
        double S = sigmoid(K, E);
        double X = (entry->result - S) * S * (1 - S);

        double mgBase = X * entry->mgPhase, egBase = X * entry->egPhase;

        for (int i = 0; i < entry->nTuples; i++) {
            int idx    = entry->tuples[i].index;
            int wcoeff = entry->tuples[i].whiteScore;
            int bcoeff = entry->tuples[i].blackScore;

            gradient[idx][0] = (wcoeff - bcoeff) * (mgBase / 24.0);
            gradient[idx][1] = (wcoeff - bcoeff) * (egBase / 24.0);
        }
    }

    void runTuner() {
        double params[487][2] = {0}, cparams[487][2] = {0}, adagrad[487][2] = {0};

        double K, error, rate = LRRATE;

        K = computeOptimalK();

        for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
            for (int batch = 0; batch < NUM_ENTRIES / BATCH_SIZE; batch++) {
                double gradient[487][2] = {0};
                computeGradient(gradient, params, K, batch);

                for (int i = 0; i < 487; i++) {
                    adagrad[i][0] += pow(2.0 * gradient[i][0] / BATCH_SIZE, 2.0);
                    adagrad[i][1] += pow(2.0 * gradient[i][1] / BATCH_SIZE, 2.0);

                    params[i][0] += (K * 2.0 / BATCH_SIZE) * gradient[i][0] * (rate / sqrt(1e-8 + adagrad[i][0]));
                    params[i][1] += (K * 2.0 / BATCH_SIZE) * gradient[i][1] * (rate / sqrt(1e-8 + adagrad[i][1]));

                    if (epoch % REPORTING == 0) {
                        std::cout << "params[i][0]" << params[i][0] << std::endl;
                    }
                }
            }

            error = tunedEvalErrors(params, K);

            if (epoch && epoch % LRSTEPRATE == 0)
                rate = rate / LRDROPRATE;
            if (epoch % REPORTING == 0) {
                for (int i = 0; i < 487; i++) {
                    std::cout << "index: " << i << ", "
                              << "deltaMg: " << params[i][0] << ", deltaEg: " << params[i][1] << std::endl;
                }
            }

            printf("Epoch  [%d] Error = [%g], Rate = [%g]\n", epoch, error, rate);
        }
    }
};

} // namespace Yayo
#endif // TUNER_H_
