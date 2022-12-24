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

/*
** Texel Tuning
** Mostly ripped from Andy Grant's Ethereal
** Credit to Ethereal, Algorythm-sxv, and dave7895
*/

#include "tuner.hpp"
#include "movegen.hpp"
#include <limits>
#include <utility>
#include <vector>

#define THREADS 8

namespace Yayo {

double sigmoid(double K, double E) {
    return 1.0 / (1.0 + exp(-K * E / 400.00));
}

TunerEntries::TunerEntries(const std::string &file) {
    entries = new TEntry[NUM_ENTRIES];

    std::ifstream games;
    games.open(file);

    if (!games.is_open()) {
        std::cerr << "ERROR: COULD NOT LOCATE TRAINING DATASET\n";
        return;
    }

    std::string line;
    std::vector<std::string> fens;
    for (int i = 0; i < NUM_ENTRIES; i++) {
        getline(games, line);
        fens.push_back(line);
    }

#pragma omp parallel for schedule(dynamic) num_threads(THREADS)
    for (int i = 0; i < NUM_ENTRIES; i++) {
        std::string line = fens[i];

        if (line.find("[1.0]") != std::string::npos)
            entries[i].result = 1.0;
        else if (line.find("[0.5]") != std::string::npos)
            entries[i].result = 0.5;
        else if (line.find("[0.0]") != std::string::npos)
            entries[i].result = 0.0;

        Board board;
        board.setFen(line);
        entries[i].init(board, std::move(line));

        if (!(i % 5000)) {
            std::cout << "initializing tuner entry #" << i << " of "
                      << NUM_ENTRIES << "\n";
        }
    }

    games.close();
}

void TEntry::init(Board &board, std::string fen) {
    Trace trace = Trace();
    Info info[1];
    info->timeGiven = false;

    Search search;
    search.setInfo(info);
    search._setFen(std::move(fen));
    search.probe = false;

    // turn = board.turn;
    search.negaMax(-INF, INF, 2, false);
    auto pvMoves = search.getPv();

    for (auto move : pvMoves) {
        make(board, move);
    }

    // staticEval = search.quiescent(-INF, INF);

    Eval<TRACE> eval(board, trace);
    staticEval = eval.eval();

    if (board.turn == BLACK) {
        staticEval *= -1;
    }

    // clang-format off
    phase = 4 * popcount(board.pieces(QUEEN)) +
            2 * popcount(board.pieces(ROOK)) +
            1 * popcount(board.pieces(BISHOP)) +
            1 * popcount(board.pieces(KNIGHT));
    // clang-format on

    mgPhase = phase;
    if (mgPhase > 24)
        mgPhase = 24;
    egPhase = 24 - mgPhase;

    int *TraceArray = (int *)&trace; // lol
    TTuple temp_tuples[NUM_FEATURES * 2];

    for (int i = 0, w = 0; w < NUM_FEATURES; i += 2, w++) {
        if (TraceArray[i] - TraceArray[i + 1]) {
            temp_tuples[nTuples].index = w;
            temp_tuples[nTuples].whiteScore = TraceArray[i];
            temp_tuples[nTuples].blackScore = TraceArray[i + 1];

            nTuples++;
        }
    }

    tuples = new TTuple[nTuples];
    for (int i = 0; i < nTuples; i++) {
        tuples[i] = temp_tuples[i];
    }
}

double TunerEntries::computeOptimalK() {
    double start = 0.0, end = 10, step = 1.0;
    double curr = start, err;
    double best = staticEvalErrors(start);

    for (int i = 0; i < 10; i++) {
        curr = start - step;

        while (curr < end) {
            curr = curr + step;
            err = staticEvalErrors(curr);
            if (err <= best) {
                best = err, start = curr;
            }
        }

        printf("Epoch [%d] K = [%.9f] E = [%.9f]\n", i, start, best);

        end = start + step;
        start = start - step;
        step = step / 10.0;
    }

    return start;
}

// clang-format off
double TunerEntries::staticEvalErrors(double K) {
    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NUM_ENTRIES / THREADS) reduction(+:total)
        for (int i = 0; i < NUM_ENTRIES; i++) {
            total += pow(entries[i].result - sigmoid(K, entries[i].staticEval), 2);
        }
    }
    return total / (double)NUM_ENTRIES;
}

double TunerEntries::tunedEvalErrors(double params[NUM_FEATURES][2], double K) {
    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NUM_ENTRIES / THREADS) reduction(+:total)
        for (int i = 0; i < NUM_ENTRIES; i++) {
            total += pow(entries[i].result - sigmoid(K, entries[i].linearEval(params)), 2.0);
        }
    }

    return total / (double)NUM_ENTRIES;
}
// clang-format on

double TEntry::linearEval(double params[NUM_FEATURES][2]) {
    Score *weights = &((Score *)&w)[0];
    int mgScore = 0, egScore = 0;

    double linearEval = TEMPO;
    for (int i = 0; i < nTuples; i++) {
        int Fi = tuples[i].whiteScore - tuples[i].blackScore;
        Score s = weights[tuples[i].index];

        double eval =
              (Fi *
               (double)(mgPhase * (MgScore(s) + params[tuples[i].index][0]) +
                        egPhase * (EgScore(s) + params[tuples[i].index][1]))) /
              24;

        linearEval += eval;
    }

    return linearEval;
}

// clang-format off
void TunerEntries::computeGradient(double gradient[NUM_FEATURES][2], double params[NUM_FEATURES][2], double K, int batch) {
    double local[NUM_FEATURES][2] = {{0}};

    for (int i = batch * BATCH_SIZE; i < (batch + 1) * BATCH_SIZE; i++) {
        updateSingleGradient(entries[i], local, params, K);

        for (int i = 1; i < NUM_FEATURES; i++) {
            gradient[i][0] += local[i][0];
            gradient[i][1] += local[i][1];
        }
    }
}
// clang-format on

void TunerEntries::updateSingleGradient(TEntry &entry,
                                        double gradient[NUM_FEATURES][2],
                                        double params[NUM_FEATURES][2],
                                        double K) {
    double E = entry.linearEval(params);
    double S = sigmoid(K, E);
    double X = (entry.result - S) * S * (1 - S);

    double mgBase = X * entry.mgPhase, egBase = X * entry.egPhase;

    for (int i = 0; i < entry.nTuples; i++) {
        int idx = entry.tuples[i].index;
        int wcoeff = entry.tuples[i].whiteScore;
        int bcoeff = entry.tuples[i].blackScore;

        gradient[idx][0] =
              (wcoeff - bcoeff) * mgBase * ((double)entry.mgPhase / 24);
        gradient[idx][1] =
              (wcoeff - bcoeff) * egBase * ((double)entry.egPhase / 24);
    }
}

// clang-format off
inline void printParams(double cparams[NUM_FEATURES][2], double params[NUM_FEATURES][2]) {
    printf("\n");
    printf("constexpr Score pawnScore   = S(%4d, %4d);\n", (int)cparams[0][0] + (int)params[0][0], (int)cparams[0][1] + (int)params[0][1]);
    printf("constexpr Score knightScore = S(%4d, %4d);\n", (int)cparams[1][0] + (int)params[1][0], (int)cparams[1][1] + (int)params[1][1]);
    printf("constexpr Score bishopScore = S(%4d, %4d);\n", (int)cparams[2][0] + (int)params[2][0], (int)cparams[2][1] + (int)params[2][1]);
    printf("constexpr Score rookScore   = S(%4d, %4d);\n", (int)cparams[3][0] + (int)params[3][0], (int)cparams[3][1] + (int)params[3][1]);
    printf("constexpr Score queenScore  = S(%4d, %4d);\n\n", (int)cparams[4][0] + (int)params[4][0], (int)cparams[4][1] + (int)params[4][1]);

    printf("constexpr Score taperedPawnPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 5; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score taperedKnightPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 69; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score taperedBishopPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 133; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score taperedRookPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 197; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score taperedQueenPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 261; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score taperedKingPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 325; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score passedPawnRankBonus[8] = {");
    for (int i = 0, start = 389; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score doubledPawnRankBonus[8] = {");
    for (int i = 0, start = 397; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score isolatedPawnRankBonus[8] = {");
    for (int i = 0, start = 405; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score backwardPawnRankBonus[8] = {");
    for (int i = 0, start = 413; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score KnightMobilityScore[9] = {");
    for (int i = 0, start = 421; i < 9; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score BishopMobilityScore[14] = {");
    for (int i = 0, start = 430; i < 14; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score RookMobilityScore[15] = {");
    for (int i = 0, start = 444; i < 15; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score QueenMobilityScore[28] = {");
    for (int i = 0, start = 459; i < 28; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");
}
// clang-format on

void TunerEntries::initUntunedWeights(double weights[NUM_FEATURES][2]) {
    Score *_w = &((Score *)&entries[0].w)[0];
    for (int i = 0; i < NUM_FEATURES; i++) {
        weights[i][0] = MgScore(_w[i]);
        weights[i][1] = EgScore(_w[i]);
    }
}

void TunerEntries::runTuner() {
    double params[NUM_FEATURES][2] = {0}, cparams[NUM_FEATURES][2] = {0},
           adagrad[NUM_FEATURES][2] = {0};

    double K, prev_err, error, rate = LRRATE;

    K = computeOptimalK();
    initUntunedWeights(cparams);

    prev_err = staticEvalErrors(K);
    double init_err = 0;

    int count = 0;

    std::ofstream out("new_weights.txt");
    for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
        for (int batch = 0; batch < NUM_ENTRIES / BATCH_SIZE; batch++) {
            double gradient[NUM_FEATURES][2] = {0};
            computeGradient(gradient, params, K, batch);

            for (int i = 1; i < NUM_FEATURES; i++) {
                adagrad[i][0] += pow(2.0 * gradient[i][0] / BATCH_SIZE, 2.0);
                adagrad[i][1] += pow(2.0 * gradient[i][1] / BATCH_SIZE, 2.0);

                params[i][0] += (K * 2.0 / BATCH_SIZE) * gradient[i][0] *
                                (rate / sqrt(1e-8 + adagrad[i][0]));
                params[i][1] += (K * 2.0 / BATCH_SIZE) * gradient[i][1] *
                                (rate / sqrt(1e-8 + adagrad[i][1]));
            }
        }

        error = tunedEvalErrors(params, K);

        if (init_err == 0)
            init_err = error;

        if (epoch && epoch % LRSTEPRATE == 0)
            rate = rate / LRDROPRATE;
        if (rate <= 0.1)
            rate = 0.1;

        if (prev_err - error < 0) {
            count++;

            if (count == 50) {
                break;
            }
        } else {
            count = 0;
        }

        if (epoch % REPORTING == 0) {
            printParams(cparams, params);

            for (int i = 0; i < NUM_FEATURES; i++) {
                out << "index: " << i << ", "
                    << "deltaMg: " << params[i][0]
                    << ", deltaEg: " << params[i][1] << std::endl;
            }

            out << "-----------------------" << std::endl;
            out << std::endl;
        }

        if (error - (init_err - 0.001) <= 0) {
            init_err = error;
        }

        printf("Epoch  [%d]  Rate = [%g], ", epoch, rate);
        std::cout.precision(std::numeric_limits<double>::max_digits10);
        std::cout << "Error = [" << error << "];  "
                  << "ΔErr = [" << std::fixed << prev_err - error << "]"
                  << ";   TT1 = [" << std::fixed
                  << (error - (init_err - 0.001)) / (prev_err - error) << "]"
                  << "\n";

        prev_err = error;
    }
}
} // namespace Yayo
