#include "eval.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "util.hpp"

namespace Yayo {

Trace trace;

int TracePeek::calculate() {
    int *trace     = &((int *)&t)[2]; // lol
    Score *weights = (Score *)&w;

    int mgPhase = ((int *)&t)[0];
    int egPhase = ((int *)&t)[1];

    std::cout << std::endl
              << "-------------\n"
              << "type pun test: \n";

    int score = TEMPO;
    for (int i = 0, w = 0; w < 394; i += 2, w++) {
        std::cout << trace[WHITE + i] - trace[BLACK + i] << std::endl;
        std::cout << "S(" << MgScore(weights[w]) << ", " << EgScore(weights[w]) << ")\n";

        int eval = (trace[WHITE + i] - trace[BLACK + i]) *
                   ((mgPhase * MgScore(weights[w]) + egPhase * EgScore(weights[w])) / 24);

        std::cout << "value: " << eval << std::endl;
        score += eval;
    }

    std::cout << "final eval: " << score << std::endl;

    return 0;
}

void TracePeek::print() {
    int numTerms = 0;

    std::cout << "MATERIAL BY COLOR:" << std::endl;
    std::cout << t.pawnScore[WHITE] << std::endl;
    std::cout << t.knightScore[WHITE] << std::endl;
    std::cout << t.bishopScore[WHITE] << std::endl;
    std::cout << t.rookScore[WHITE] << std::endl;
    std::cout << t.queenScore[WHITE] << std::endl;
    std::cout << "-----\n";
    std::cout << t.pawnScore[BLACK] << std::endl;
    std::cout << t.knightScore[BLACK] << std::endl;
    std::cout << t.bishopScore[BLACK] << std::endl;
    std::cout << t.rookScore[BLACK] << std::endl;
    std::cout << t.queenScore[BLACK] << std::endl;
    std::cout << "-----\n";

    std::cout << "MATERIAL: " << std::endl;
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
        if (t.pawnPcSq[WHITE][i] || t.pawnPcSq[BLACK][i])
            std::cout << t.pawnPcSq[WHITE][i] - t.pawnPcSq[BLACK][i] << std::endl;

        else if (t.knightPcSq[WHITE][i] || t.knightPcSq[BLACK][i])
            std::cout << t.knightPcSq[WHITE][i] - t.knightPcSq[BLACK][i] << std::endl;

        else if (t.bishopPcSq[WHITE][i] || t.bishopPcSq[BLACK][i])
            std::cout << t.bishopPcSq[WHITE][i] - t.bishopPcSq[BLACK][i] << std::endl;

        else if (t.rookPcSq[WHITE][i] || t.rookPcSq[BLACK][i])
            std::cout << t.rookPcSq[WHITE][i] - t.rookPcSq[BLACK][i] << std::endl;

        else if (t.queenPcSq[WHITE][i] || t.queenPcSq[BLACK][i])
            std::cout << t.queenPcSq[WHITE][i] - t.queenPcSq[BLACK][i] << std::endl;

        else if (t.kingPcSq[WHITE][i] || t.kingPcSq[BLACK][i])
            std::cout << t.kingPcSq[WHITE][i] - t.kingPcSq[BLACK][i] << std::endl;

        else
            std::cout << 0 << std::endl;
    }

    numTerms += 8;
    std::cout << "PASSED PAWN: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.passedPawn[WHITE][i] - t.passedPawn[BLACK][i] << std::endl;
    }

    numTerms += 8;
    std::cout << "DOUBLED PAWN" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.doubledPawns[WHITE] - t.doubledPawns[BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "ISOLATED PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.isolatedPawns[WHITE] - t.isolatedPawns[BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "BACKWARD PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << t.backwardPawns[WHITE] - t.backwardPawns[BLACK] << std::endl;
    }

    numTerms += 9;
    std::cout << "knightMobility:" << std::endl;
    for (int i = 0; i < 9; i++) {
        std::cout << t.knightMobility[WHITE][i] - t.knightMobility[BLACK][i] << std::endl;
    }

    numTerms += 14;
    std::cout << "bishopMobility:" << std::endl;
    for (int i = 0; i < 14; i++) {
        std::cout << t.bishopMobility[WHITE][i] - t.bishopMobility[BLACK][i] << std::endl;
    }

    numTerms += 15;
    std::cout << "rookMobility:" << std::endl;
    for (int i = 0; i < 15; i++) {
        std::cout << t.rookMobility[WHITE][i] - t.rookMobility[BLACK][i] << std::endl;
    }

    numTerms += 28;
    std::cout << "queenMobility:" << std::endl;
    for (int i = 0; i < 28; i++) {
        std::cout << t.queenMobility[WHITE][i] - t.queenMobility[BLACK][i] << std::endl;
    }

    std::cout << "total number of terms: " << numTerms << std::endl;
}

// template <Tracing T = NO_TRACE> int eval(Board &board) {
// Eval evaluation(board);

// const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
// const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
// const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
// const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
// const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

// const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
// const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
// const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
// const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
// const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

// const int wMaterial =
//     (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) + (ROOK_VAL *
//     whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

// const int bMaterial =
//     (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) + (ROOK_VAL *
//     blackRookScore) + (QUEEN_VAL * blackQueenScore);

// trace.pawnScore[WHITE] = whitePawnScore;
// trace.pawnScore[BLACK] = blackPawnScore;

// trace.knightScore[WHITE] = whiteKnightScore;
// trace.knightScore[BLACK] = blackKnightScore;

// trace.bishopScore[WHITE] = whiteBishopScore;
// trace.bishopScore[BLACK] = blackBishopScore;

// trace.rookScore[WHITE] = whiteRookScore;
// trace.rookScore[BLACK] = blackRookScore;

// trace.queenScore[WHITE] = whiteQueenScore;
// trace.queenScore[BLACK] = blackQueenScore;

// const auto color = (board.turn == WHITE) ? 1 : -1;

// int eval = TEMPO;
// const auto pcSqEval = pieceSquare<TRACE, WHITE>(board) - pieceSquare<TRACE, BLACK>(board);
// const auto passed = passedPawnScore<TRACE, WHITE>(board) - passedPawnScore<TRACE, BLACK>(board);
// const auto doubledPenalty = doubledPawnPenalty<TRACE, WHITE>(board) - doubledPawnPenalty<TRACE, BLACK>(board);
// const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
// const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
// const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
// const auto backwardPenalty = backwardPawnScore<TRACE, WHITE>(board) - backwardPawnScore<TRACE, BLACK>(board);
// const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
// const auto mobility = mobilityScore<TRACE, WHITE>(board) - mobilityScore<TRACE, BLACK>(board);

// trace.isolatedPawns[WHITE] = whiteIsolatedPawnCount;
// trace.isolatedPawns[BLACK] = blackIsolatedPawnCount;

// eval += (mobility / 2);
// eval += (wMaterial - bMaterial);
// eval += (1.2 * pcSqEval);
// eval += (0.3 * pawnStructureScore);

// return  eval * color;
//     return 0;
// }

// template <> int eval<NO_TRACE>(Board &board) {
//     // const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
//     // const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
//     // const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
//     // const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
//     // const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

//     // const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
//     // const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
//     // const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
//     // const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
//     // const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

//     // const int wMaterial =
//     //     (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) +
//     (ROOK_VAL * whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

//     // const int bMaterial =
//     //     (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) +
//     (ROOK_VAL * blackRookScore) + (QUEEN_VAL * blackQueenScore);

//     // const auto color = (board.turn == WHITE) ? 1 : -1;

//     // int eval = TEMPO;
//     // const auto pcSqEval = pieceSquare<NO_TRACE, WHITE>(board) - pieceSquare<NO_TRACE, BLACK>(board);
//     // const auto passed = passedPawnScore<NO_TRACE, WHITE>(board) - passedPawnScore<NO_TRACE, BLACK>(board);
//     // const auto doubledPenalty = doubledPawnPenalty<NO_TRACE, WHITE>(board) - doubledPawnPenalty<NO_TRACE,
//     BLACK>(board);
//     // const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
//     // const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
//     // const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
//     // const auto backwardPenalty = backwardPawnScore<NO_TRACE, WHITE>(board) - backwardPawnScore<NO_TRACE,
//     BLACK>(board);
//     // const auto pawnStructureScore = passed + doubledPenalty + isolatedPenalty + backwardPenalty;
//     // const auto mobility = mobilityScore<NO_TRACE, WHITE>(board) - mobilityScore<NO_TRACE, BLACK>(board);

//     // eval += mobility;
//     // eval += (wMaterial - bMaterial);
//     // eval += pcSqEval;
//     // eval += pawnStructureScore;

//     // return  eval * color;
//     return 0;
// }

} // namespace Yayo
