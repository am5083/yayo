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
#include <limits>

namespace Yayo {

double sigmoid(double K, double E) {
    return 1.0 / (1.0 + exp(-K * E / 400.00));
}

int quiescent(Board &_board, int alpha, int beta) {
    int hashFlag = TP_ALPHA;
    int ply = _board.ply;

    bool pvNode = (beta - alpha) < 1;
    Eval eval(_board);
    int score = 0, best = eval.eval(), oldAlpha = alpha;
    int bestMove = 0;

    if (best >= beta)
        return beta;

    if (!_board.checkPcs && ((best + QUEEN_VAL) < alpha)) {
        return alpha;
    }

    if (alpha < best)
        alpha = best;

    moveList mList = {0};
    generateCaptures(_board, &mList);

    for (int i = 0; i < mList.nMoves; i++) {
        const int c_move = mList.moves[i].move;

        Square fromSq = getFrom(c_move), toSq = getTo(c_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getPcType(fromPc) > getPcType(toPc)) {
            int see = _board.see(toSq, toPc, fromSq, fromPc);
            if (see < 0) {
                mList.moves[i].score = (see / 1000) + 50;
            } else {
                mList.moves[i].score = see;
            }
        }
    }

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);

        make(_board, mList.moves[i].move);
        score = -quiescent(_board, -beta, -alpha);
        unmake(_board, mList.moves[i].move);

        if (score > best) {
            best = score;
            bestMove = mList.moves[i].move;

            if (score > alpha) {
                hashFlag = TP_EXACT;
                alpha = score;

                if (alpha >= beta) {
                    hashFlag = TP_BETA;
                    break;
                }
            }
        }
    }

    return alpha;
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

        if (!(i % 50000)) {
            std::cout << "initializing tuner entry #" << i << " of "
                      << NUM_ENTRIES << "\n";
        }
    }

    games.close();
}

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

    eval.eval();
    staticEval = quiescent(board, -INF + 1, INF - 1);

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
        #pragma omp for schedule(static, NUM_ENTRIES / 8) reduction(+:total)
        for (int i = 0; i < NUM_ENTRIES; i++) {
            int staticEval = entries[i].turn == WHITE ? entries[i].staticEval : -1 * entries[i].staticEval;
            total += pow(entries[i].result - sigmoid(K, staticEval), 2);
        }
    }
    return total / (double)NUM_ENTRIES;
}

double TunerEntries::tunedEvalErrors(double params[NUM_FEATURES][2], double K) {
    double total = 0.0;

    #pragma omp parallel shared(total)
    {
        #pragma omp for schedule(static, NUM_ENTRIES / 8) reduction(+:total)
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

    double linearEval = 0;
    for (int i = 0; i < nTuples; i++) {
        int Fi = tuples[i].whiteScore - tuples[i].blackScore;
        Score s = weights[tuples[i].index];

        double eval =
              Fi *
              ((double)(mgPhase * (MgScore(s) + params[tuples[i].index][0]) +
                        egPhase * (EgScore(s) + params[tuples[i].index][1])) /
               24);

        linearEval += eval;
    }

    return linearEval;
}

// clang-format off
void TunerEntries::computeGradient(double gradient[NUM_FEATURES][2], double params[NUM_FEATURES][2], double K, int batch) {
    double local[NUM_FEATURES][2] = {{0}};

    for (int i = batch * BATCH_SIZE; i < (batch + 1) * BATCH_SIZE; i++) {
        updateSingleGradient(entries[i], local, params, K);

        for (int i = 0; i < NUM_FEATURES; i++) {
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
    printf("constexpr Score pawnScore   = S(%4d, %4d)\n", (int)cparams[0][0] + (int)params[0][0], (int)cparams[0][1] + (int)params[0][1]);
    printf("constexpr Score knightScore = S(%4d, %4d)\n", (int)cparams[1][0] + (int)params[1][0], (int)cparams[1][1] + (int)params[1][1]);
    printf("constexpr Score bishopScore = S(%4d, %4d)\n", (int)cparams[2][0] + (int)params[2][0], (int)cparams[2][1] + (int)params[2][1]);
    printf("constexpr Score rookScore   = S(%4d, %4d)\n", (int)cparams[3][0] + (int)params[3][0], (int)cparams[3][1] + (int)params[3][1]);
    printf("constexpr Score queenScore  = S(%4d, %4d)\n\n", (int)cparams[4][0] + (int)params[4][0], (int)cparams[4][1] + (int)params[4][1]);

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

    printf("constexpr Score kingAttackersWeight[5] = {");
    for (int i = 0, start = 487; i < 5; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score trappedRookWeight = {");
    for (int i = 0, start = 492; i < 1; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score rookOnOpenFile[2] = {");
    for (int i = 0, start = 493; i < 2; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score openFileNextToKing[3] = {");
    for (int i = 0, start = 495; i < 3; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score pawnShieldStrength[4] = {");
    for (int i = 0, start = 498; i < 4; i++) {
        if (!(i % 4))
            printf("\n");
        printf("S(%4d, %4d), ", (int)cparams[start + i][0] + (int)params[start + i][0], (int)cparams[start + i][1] + (int)params[start + i][1]);
    }
    printf("\n};\n");

    printf("constexpr Score pushedPawnShieldStrength[4] = {");
    for (int i = 0, start = 502; i < 4; i++) {
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

    std::ofstream out("new_weights.txt");
    for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
        for (int batch = 0; batch < NUM_ENTRIES / BATCH_SIZE; batch++) {
            double gradient[NUM_FEATURES][2] = {0};
            computeGradient(gradient, params, K, batch);

            for (int i = 0; i < NUM_FEATURES; i++) {
                adagrad[i][0] += pow(2.0 * gradient[i][0] / BATCH_SIZE, 2.0);
                adagrad[i][1] += pow(2.0 * gradient[i][1] / BATCH_SIZE, 2.0);

                params[i][0] += (K * 2.0 / BATCH_SIZE) * gradient[i][0] *
                                (rate / sqrt(1e-8 + adagrad[i][0]));
                params[i][1] += (K * 2.0 / BATCH_SIZE) * gradient[i][1] *
                                (rate / sqrt(1e-8 + adagrad[i][1]));
            }
        }

        error = tunedEvalErrors(params, K);
        if (epoch && epoch % LRSTEPRATE == 0)
            rate = rate / LRDROPRATE;
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

        printf("Epoch  [%d]  Rate = [%g], ", epoch, rate);
        std::cout.precision(std::numeric_limits<double>::max_digits10);
        std::cout << "Error = [" << error << "];  "
                  << "ΔErr = [" << std::fixed << prev_err - error << "]"
                  << "\n";

        prev_err = error;
    }
}
} // namespace Yayo
