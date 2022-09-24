#include "tuner.hpp"
#include <limits>

namespace Yayo {

double sigmoid(double K, double E) { return 1.0 / (1.0 + exp(-K * E / 400.00)); }

void TEntry::init(Board &board) {
    Trace trace = Trace();
    Eval<TRACE> eval(board, trace);
    turn = board.turn;

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

    staticEval = eval.eval();

    int *TraceArray = (int *)&trace; // lol
    TTuple temp_tuples[974];

    for (int i = 0, w = 0; w < 487; i += 2, w++) {
        if (TraceArray[i] - TraceArray[i + 1]) {
            temp_tuples[nTuples].index      = w;
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
            err  = staticEvalErrors(curr);
            if (err <= best) {
                best = err, start = curr;
            }
        }

        printf("Epoch [%d] K = [%.9f] E = [%.9f]\n", i, start, best);

        end   = start + step;
        start = start - step;
        step  = step / 10.0;
    }

    return start;
}

double TunerEntries::staticEvalErrors(double K) {
    double total = 0.0;
    for (int i = 0; i < NUM_ENTRIES; i++) {
        total += pow(entries[i].result - sigmoid(K, entries[i].staticEval), 2);
    }
    return total / (double)NUM_ENTRIES;
}

double TEntry::linearEval(double params[487][2]) {
    Score *weights = &((Score *)&w)[0];
    int mgScore = 0, egScore = 0;

    double linearEval = 0;
    for (int i = 0; i < nTuples; i++) {
        int Fi  = tuples[i].whiteScore - tuples[i].blackScore;
        Score s = weights[tuples[i].index];

        double eval = Fi * ((double)(mgPhase * (MgScore(s) + params[tuples[i].index][0]) +
                                     egPhase * (EgScore(s) + params[tuples[i].index][1])) /
                            24);

        linearEval += eval;
    }

    int color = (turn == WHITE) ? 1 : -1;
    return linearEval * color;
}

TunerEntries::TunerEntries(std::string file) {
    entries = new TEntry[NUM_ENTRIES];

    Board board;
    std::ifstream games;
    games.open(file);

    if (!games.is_open()) {
        std::cerr << "ERROR: COULD NOT LOCATE TRAINING DATASET\n";
        return;
    }

    std::string line;
    for (int i = 0; i < NUM_ENTRIES; i++) {
        getline(games, line);

        if (line.find("[1.0]") != std::string::npos)
            entries[i].result = 1.0;
        else if (line.find("[0.5]") != std::string::npos)
            entries[i].result = 0.5;
        else if (line.find("[0.0]") != std::string::npos)
            entries[i].result = 0.0;

        board.setFen(line);
        entries[i].init(board);
    }

    games.close();
}

double TunerEntries::tunedEvalErrors(double params[487][2], double K) {
    double total = 0.0;
    for (int i = 0; i < NUM_ENTRIES; i++) {
        total += pow(entries[i].result - sigmoid(K, entries[i].linearEval(params)), 2.0);
    }
    return total / (double)NUM_ENTRIES;
}

void TunerEntries::computeGradient(double gradient[487][2], double params[487][2], double K, int batch) {
    double local[487][2] = {{0}};
    for (int i = batch * BATCH_SIZE; i < (batch + 1) * BATCH_SIZE; i++) {
        updateSingleGradient(entries[i], local, params, K);

        for (int i = 0; i < 487; i++) {
            gradient[i][0] += local[i][0];
            gradient[i][1] += local[i][1];
        }
    }
}

void TunerEntries::updateSingleGradient(TEntry &entry, double gradient[487][2], double params[487][2], double K) {
    double E = entry.linearEval(params);
    double S = sigmoid(K, E);
    double X = (entry.result - S) * S * (1 - S);

    double mgBase = X * entry.mgPhase, egBase = X * entry.egPhase;

    for (int i = 0; i < entry.nTuples; i++) {
        int idx    = entry.tuples[i].index;
        int wcoeff = entry.tuples[i].whiteScore;
        int bcoeff = entry.tuples[i].blackScore;

        gradient[idx][0] = (wcoeff - bcoeff) * (mgBase / 24.0);
        gradient[idx][1] = (wcoeff - bcoeff) * (egBase / 24.0);
    }
}

// clang-format off
inline void printParams(double params[487][2]) {
    printf("\n");
    printf("pawnScore   = S(%4d, %4d)\n", (int)params[0][0], (int)params[0][1]);
    printf("knightScore = S(%4d, %4d)\n", (int)params[1][0], (int)params[1][1]);
    printf("bishopScore = S(%4d, %4d)\n", (int)params[2][0], (int)params[2][1]);
    printf("rookScore   = S(%4d, %4d)\n", (int)params[3][0], (int)params[3][1]);
    printf("queenScore  = S(%4d, %4d)\n\n", (int)params[4][0], (int)params[4][1]);

    printf("taperedPawnPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 5; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("taperedKnightPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 69; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("taperedBishopPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 133; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("taperedRookPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 197; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("taperedQueenPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 261; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("taperedKingPcSq[SQUARE_CT] = {");
    for (int i = 0, start = 325; i < 64; i++) {
        if (!(i % 8))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("passedPawnRankBonus[8] = {");
    for (int i = 0, start = 389; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("doubledPawnRankBonus[8] = {");
    for (int i = 0, start = 397; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("isolatedPawnRankBonus[8] = {");
    for (int i = 0, start = 405; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("backwardPawnRankBonus[8] = {");
    for (int i = 0, start = 413; i < 8; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("KnightMobilityScore[9] = {");
    for (int i = 0, start = 421; i < 9; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("BishopMobilityScore[14] = {");
    for (int i = 0, start = 430; i < 14; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("RookMobilityScore[15] = {");
    for (int i = 0, start = 444; i < 15; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("QueenMobilityScore[28] = {");
    for (int i = 0, start = 459; i < 28; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)params[start + i][0], (int)params[start + i][1]);
    }
    printf("\n};\n");
}
// clang-format on

void TunerEntries::runTuner() {
    double params[487][2] = {0}, cparams[487][2] = {0}, adagrad[487][2] = {0};

    double K, error, rate = LRRATE;

    K = computeOptimalK();

    std::ofstream out("new_weights.txt");
    for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
        for (int batch = 0; batch < NUM_ENTRIES / BATCH_SIZE; batch++) {
            double gradient[487][2] = {0};
            computeGradient(gradient, params, K, batch);

            for (int i = 0; i < 487; i++) {
                adagrad[i][0] += pow(2.0 * gradient[i][0] / BATCH_SIZE, 2.0);
                adagrad[i][1] += pow(2.0 * gradient[i][1] / BATCH_SIZE, 2.0);

                params[i][0] += (K * 2.0 / BATCH_SIZE) * gradient[i][0] * (rate / sqrt(1e-8 + adagrad[i][0]));
                params[i][1] += (K * 2.0 / BATCH_SIZE) * gradient[i][1] * (rate / sqrt(1e-8 + adagrad[i][1]));
            }
        }

        error = tunedEvalErrors(params, K);
        if (epoch && epoch % LRSTEPRATE == 0)
            rate = rate / LRDROPRATE;
        if (epoch % REPORTING == 0) {
            printParams(params);

            for (int i = 0; i < 487; i++) {
                out << "index: " << i << ", "
                    << "deltaMg: " << params[i][0] << ", deltaEg: " << params[i][1] << std::endl;
            }

            out << "-----------------------" << std::endl;
            out << std::endl;
        }

        printf("Epoch  [%d]  Rate = [%g], ", epoch, rate);
        std::cout.precision(std::numeric_limits<double>::max_digits10);
        std::cout << "Error = [" << error << "]\n";
    }
}

} // namespace Yayo

// 2:42 - 233
