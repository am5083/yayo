#include "eval.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "util.hpp"

namespace Yayo {

const EvalWeights evalWeights;

int TracePeek::calculate(std::tuple<int, int, int> info) {
    int *trace     = (int *)&t; // lol
    Score *weights = &((Score *)&w)[0];

    const int color   = std::get<0>(info);
    const int mgPhase = std::get<1>(info);
    const int egPhase = std::get<2>(info);

    int score     = TEMPO;
    int nFeatures = 0;
    for (int i = 0, w = 0; w < 487; i += 2, w++) {
        if (trace[i] - trace[i + 1]) {
            const int eval =
                (trace[i] - trace[i + 1]) * ((mgPhase * MgScore(weights[w]) + egPhase * EgScore(weights[w])) / 24);

            std::cout << "eval: " << eval << std::endl;

            score += eval;
            nFeatures++;
        }
    }

    std::cout << "\\sum{features * weights} = " << score * color << std::endl;
    std::cout << "nFeatures: " << nFeatures << std::endl;

    return 0;
}

void TracePeek::print() {
    int numTerms = 0;

    std::cout << "\nMATERIAL: " << std::endl;
    std::cout << t.pawnScore[WHITE] - t.pawnScore[BLACK] << std::endl;
    numTerms++;
    std::cout << t.knightScore[WHITE] - t.knightScore[BLACK] << std::endl;
    numTerms++;
    std::cout << t.bishopScore[WHITE] - t.bishopScore[BLACK] << std::endl;
    numTerms++;
    std::cout << t.rookScore[WHITE] - t.rookScore[BLACK] << std::endl;
    numTerms++;
    std::cout << t.queenScore[WHITE] - t.queenScore[BLACK] << std::endl;
    numTerms++;
    std::cout << "-----\n";

    numTerms += 6 * 64;
    std::cout << "PIECE SQUARE: " << std::endl;
    for (int i = 0; i < 64; i++) {
        if (i && !(i % 8)) {
            std::cout << std::endl;
        }

        if (t.pawnPcSq[i][WHITE] || t.pawnPcSq[i][BLACK])
            std::cout << t.pawnPcSq[i][WHITE] - t.pawnPcSq[i][BLACK];

        else if (t.knightPcSq[i][WHITE] || t.knightPcSq[i][BLACK])
            std::cout << t.knightPcSq[i][WHITE] - t.knightPcSq[i][BLACK];

        else if (t.bishopPcSq[i][WHITE] || t.bishopPcSq[i][BLACK])
            std::cout << t.bishopPcSq[i][WHITE] - t.bishopPcSq[i][BLACK];

        else if (t.rookPcSq[i][WHITE] || t.rookPcSq[i][BLACK])
            std::cout << t.rookPcSq[i][WHITE] - t.rookPcSq[i][BLACK];

        else if (t.queenPcSq[i][WHITE] || t.queenPcSq[i][BLACK])
            std::cout << t.queenPcSq[i][WHITE] - t.queenPcSq[i][BLACK];

        else if (t.kingPcSq[i][WHITE] || t.kingPcSq[i][BLACK])
            std::cout << t.kingPcSq[i][WHITE] - t.kingPcSq[i][BLACK];

        else
            std::cout << 0;
    }

    std::cout << "\n-----------\n";

    numTerms += 8;
    std::cout << "PASSED PAWN: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.passedPawn[i][WHITE] - t.passedPawn[i][BLACK] << std::endl;
    }

    std::cout << "\n-----------\n";

    numTerms += 8;
    std::cout << "DOUBLED PAWN" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.doubledPawns[i][WHITE] - t.doubledPawns[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "ISOLATED PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.isolatedPawns[i][WHITE] - t.isolatedPawns[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "BACKWARD PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.backwardPawns[i][WHITE] - t.backwardPawns[i][BLACK] << std::endl;
    }

    numTerms += 18;
    std::cout << "knightMobility:" << std::endl;
    for (int i = 0; i < 9; i++) {
        std::cout << t.knightMobility[i][WHITE] - t.knightMobility[i][BLACK] << std::endl;
    }

    numTerms += 28;
    std::cout << "bishopMobility:" << std::endl;
    for (int i = 0; i < 14; i++) {
        std::cout << t.bishopMobility[i][WHITE] - t.bishopMobility[i][BLACK] << std::endl;
    }

    numTerms += 30;
    std::cout << "rookMobility:" << std::endl;
    for (int i = 0; i < 15; i++) {
        std::cout << t.rookMobility[i][WHITE] - t.rookMobility[i][BLACK] << std::endl;
    }

    numTerms += 56;
    std::cout << "queenMobility:" << std::endl;
    for (int i = 0; i < 28; i++) {
        std::cout << t.queenMobility[i][WHITE] - t.queenMobility[i][BLACK] << std::endl;
    }

    std::cout << "\ntotal number of terms: " << numTerms << std::endl;
}
} // namespace Yayo
