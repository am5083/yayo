#include "eval.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "util.hpp"

namespace Yayo {
// clang-format off
int eval(Board &board, moveList &mList) {
    const int wMaterial =
        (PAWN_VAL * popcount(board.pieces(PAWN, WHITE)))     + (KNIGHT_VAL * popcount(board.pieces(KNIGHT, WHITE))) +
        (BISHOP_VAL * popcount(board.pieces(BISHOP, WHITE))) + (ROOK_VAL * popcount(board.pieces(ROOK, WHITE)))     +
        (QUEEN_VAL * popcount(board.pieces(QUEEN, WHITE)));

    const int bMaterial =
        (PAWN_VAL * popcount(board.pieces(PAWN, BLACK)))     + (KNIGHT_VAL * popcount(board.pieces(KNIGHT, BLACK))) +
        (BISHOP_VAL * popcount(board.pieces(BISHOP, BLACK))) + (ROOK_VAL * popcount(board.pieces(ROOK, BLACK)))     +
        (QUEEN_VAL * popcount(board.pieces(QUEEN, BLACK)));

    const auto color = (board.turn == WHITE) ? 1 : -1;

    const auto pcSqEval = (pieceSquare<WHITE>(board) - pieceSquare<BLACK>(board));
    const auto passed = passedPawnScore<WHITE>(board) - passedPawnScore<BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<WHITE>(board) - doubledPawnPenalty<BLACK>(board);
    const auto isolatedPenalty = (ISOLATED_PENALTY * isolatedPawnCount<WHITE>(board)) - (ISOLATED_PENALTY * isolatedPawnCount<BLACK>(board));
    const auto backwardPenalty = backwardPawnScore<WHITE>(board) - backwardPawnScore<BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
    const auto mobility = mobilityScore<WHITE>(board) - mobilityScore<BLACK>(board);

    // std::cout << "material: " << (wMaterial - bMaterial) << std::endl;
    // std::cout << "pcSqEval: " << pcSqEval << std::endl;
    // std::cout << "passed: " << passed << std::endl;
    // std::cout << "doubledPenalty: " << doubledPenalty << std::endl;
    // std::cout << "isolatedPenalty: " << isolatedPenalty << std::endl;
    // std::cout << "backwardPenalty: " << backwardPenalty << std::endl;
    // std::cout << "pawnStructureScore: " << pawnStructureScore << std::endl;
    // std::cout << "mobility: " << mobility << std::endl;

    int eval = TEMPO;
    eval += (mobility / 2);
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);

    return  eval * color;
}

}
