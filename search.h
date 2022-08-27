#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.h"
#include "movegen.h"
#include <thread>

namespace Yayo {

int eval(Board &board) {
    const int _PAWN = 100;
    const int _KNIGHT = 300;
    const int _BISHOP = 330;
    const int _ROOK = 500;
    const int _QUEEN = 900;

    const int wMaterial = (_PAWN * __builtin_popcountll(board.pieces(PAWN, WHITE))) +
                          (_KNIGHT * __builtin_popcountll(board.pieces(KNIGHT, WHITE))) +
                          (_BISHOP * __builtin_popcountll(board.pieces(BISHOP, WHITE))) +
                          (_ROOK * __builtin_popcountll(board.pieces(ROOK, WHITE))) +
                          (_QUEEN * __builtin_popcountll(board.pieces(QUEEN, WHITE)));

    const int bMaterial = (_PAWN * __builtin_popcountll(board.pieces(PAWN, BLACK))) +
                          (_KNIGHT * __builtin_popcountll(board.pieces(KNIGHT, BLACK))) +
                          (_BISHOP * __builtin_popcountll(board.pieces(BISHOP, BLACK))) +
                          (_ROOK * __builtin_popcountll(board.pieces(ROOK, BLACK))) +
                          (_QUEEN * __builtin_popcountll(board.pieces(QUEEN, BLACK)));

    const auto color = (board.turn == WHITE) ? 1 : -1;

    moveList mList = {0};
    generate(board, &mList);
    moveList otherMoves = {0};
    makeNullMove(board);
    generate(board, &otherMoves);
    unmakeNullMove(board);

    return ((0.10 * (mList.nMoves - otherMoves.nMoves)) + (wMaterial - bMaterial)) * color;
}

} // namespace Yayo
#endif // SEARCH_H_
