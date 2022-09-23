#include "tuner.hpp"

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

    nTuples = 0;
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
        std::cout << "eval: " << eval << std::endl;

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
} // namespace Yayo
