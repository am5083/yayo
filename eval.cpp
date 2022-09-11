#include "eval.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "util.hpp"

namespace Yayo {

Trace trace;

// clang-format off
template <> int eval<TRACE>(Board &board, moveList &mList) {
    const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
    const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
    const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
    const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
    const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

    const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
    const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
    const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
    const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
    const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

    const int wMaterial =
        (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) + (ROOK_VAL * whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

    const int bMaterial =
        (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) + (ROOK_VAL * blackRookScore) + (QUEEN_VAL * blackQueenScore);

    trace.pawnScore[WHITE] = whitePawnScore;
    trace.pawnScore[BLACK] = blackPawnScore;

    trace.knightScore[WHITE] = whiteKnightScore;
    trace.knightScore[BLACK] = blackKnightScore;

    trace.bishopScore[WHITE] = whiteBishopScore;
    trace.bishopScore[BLACK] = blackBishopScore;

    trace.rookScore[WHITE] = whiteRookScore;
    trace.rookScore[BLACK] = blackRookScore;

    trace.queenScore[WHITE] = whiteQueenScore;
    trace.queenScore[BLACK] = blackQueenScore;

    const auto color = (board.turn == WHITE) ? 1 : -1;

    int eval = TEMPO;
    const auto pcSqEval = pieceSquare<TRACE, WHITE>(board) - pieceSquare<TRACE, BLACK>(board);
    const auto passed = passedPawnScore<TRACE, WHITE>(board) - passedPawnScore<TRACE, BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<TRACE, WHITE>(board) - doubledPawnPenalty<TRACE, BLACK>(board);
    const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
    const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
    const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
    const auto backwardPenalty = backwardPawnScore<TRACE, WHITE>(board) - backwardPawnScore<TRACE, BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
    const auto mobility = mobilityScore<TRACE, WHITE>(board) - mobilityScore<TRACE, BLACK>(board);

    trace.isolatedPawns[WHITE] = whiteIsolatedPawnCount;
    trace.isolatedPawns[BLACK] = blackIsolatedPawnCount;

    eval += (mobility / 2);
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);
    // std::cout << "material: " << (wMaterial - bMaterial) << std::endl;
    // std::cout << "pcSqEval: " << pcSqEval << std::endl;
    // std::cout << "passed: " << passed << std::endl;
    // std::cout << "doubledPenalty: " << doubledPenalty << std::endl;
    // std::cout << "isolatedPenalty: " << isolatedPenalty << std::endl;
    // std::cout << "backwardPenalty: " << backwardPenalty << std::endl;
    // std::cout << "pawnStructureScore: " << pawnStructureScore << std::endl;
    // std::cout << "mobility: " << mobility << std::endl;

    return  eval * color;
}

template <> int eval<NO_TRACE>(Board &board, moveList &mList) {
    const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
    const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
    const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
    const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
    const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

    const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
    const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
    const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
    const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
    const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

    const int wMaterial =
        (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) + (ROOK_VAL * whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

    const int bMaterial =
        (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) + (ROOK_VAL * blackRookScore) + (QUEEN_VAL * blackQueenScore);


    const auto color = (board.turn == WHITE) ? 1 : -1;

    int eval = TEMPO;
    const auto pcSqEval = pieceSquare<NO_TRACE, WHITE>(board) - pieceSquare<NO_TRACE, BLACK>(board);
    const auto passed = passedPawnScore<NO_TRACE, WHITE>(board) - passedPawnScore<NO_TRACE, BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<NO_TRACE, WHITE>(board) - doubledPawnPenalty<NO_TRACE, BLACK>(board);
    const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
    const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
    const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
    const auto backwardPenalty = backwardPawnScore<NO_TRACE, WHITE>(board) - backwardPawnScore<NO_TRACE, BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
    const auto mobility = mobilityScore<NO_TRACE, WHITE>(board) - mobilityScore<NO_TRACE, BLACK>(board);

    eval += (mobility / 2);
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);
    // std::cout << "material: " << (wMaterial - bMaterial) << std::endl;
    // std::cout << "pcSqEval: " << pcSqEval << std::endl;
    // std::cout << "passed: " << passed << std::endl;
    // std::cout << "doubledPenalty: " << doubledPenalty << std::endl;
    // std::cout << "isolatedPenalty: " << isolatedPenalty << std::endl;
    // std::cout << "backwardPenalty: " << backwardPenalty << std::endl;
    // std::cout << "pawnStructureScore: " << pawnStructureScore << std::endl;
    // std::cout << "mobility: " << mobility << std::endl;

    return  eval * color;
}

}
