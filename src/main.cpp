#include "eval.hpp"
#include "thread.hpp"
#include "uci.hpp"
#include <fstream>

#define NUM_ENTRIES 1000000

double sigmoid(double K, double E) { return 1.0 / (1.0 + exp(-K * E / 400.00)); }

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

    TTuple *tuples;
    int nTuples = 0;

    void init(Board &board);

    double linearEval() {
        Score *weights = (Score *)&evalWeights;
        double mgScore = 0, egScore = 0;
        for (int i = 0; i < nTuples; i++) {
            std::cout << tuples[i].whiteScore - tuples[i].blackScore << std::endl;
        }

        const int linearEval = ((mgPhase * mgScore) + (egPhase * egScore)) / 24;

        std::cout << "linear eval: " << linearEval << ", " << staticEval << std::endl;
        return linearEval;
    }
};

struct TunerEntries {
    TEntry *entries;

    TunerEntries(std::string file) {
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

            if (!(i % 10000)) {
                std::cout << "Initializing Tuner Entries: " << i + 1 << " of " << NUM_ENTRIES << std::endl;
                entries[i].linearEval();
            }
        }

        games.close();
    }

    ~TunerEntries() { delete[] entries; }

    double staticEvalErrors(double K);
    double computeOptimalK();
};

void TEntry::init(Board &board) {
    Eval<TRACE> eval(board);
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

    int *TraceArray = &(((int *)&trace)[3]); // lol
    TTuple temp_tuples[1024];

    for (int i = 0, w = 0; w < 553; i += 2, w++) {
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

int main(int argc, char *argv[]) {
    using namespace Yayo;

    init_arrays();
    initMvvLva();

    std::unique_ptr<Search> searcher(new Search);
    UCI uci(*searcher.get());
    if (argc == 2) {
        if (strcmp(argv[1], "bench") == 0) {
            uci.Bench();
            return 0;
        }
    }

    Board board;
    board.setFen("r4k2/5ppp/p3p3/1p6/3P4/8/PP3PPP/R4RK1 w - - 0 2");

    Eval<TRACE> eval(board);
    int x = eval.eval();

    TunerEntries entries("lichess-big3-resolved.book");
    entries.computeOptimalK();

    // for (int i = 0; i < NUM_ENTRIES; i++) {
    //     if (!(i % 100000)) {
    //         std::cout << entries.entries[i].result << std::endl;
    //     }
    // }

    // uci.Main();

    return 0;
}
