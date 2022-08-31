#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.h"
#include "movegen.h"
#include "util.h"
#include <thread>

namespace Yayo {

// clang-format off
constexpr int pawn[SQUARE_CT] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50,50, 50, 50, 50, 50, 50, 50,
    10,10, 20, 30, 30, 20, 10, 10,
    5, 5,  10, 25, 25, 10, 5,  5,
    0, 0,  0,  20, 20, 0,  0,  0,
    5, -10,-10,0,  0,  -10,-10,5,
    10,10, 10, -20,-20,10, 10, 10,
    0, 0,  0,  0,  0,  0,  0,  0,
};

// clang-format off
constexpr int knight[SQUARE_CT] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20, 0,  0,  0,  0, -20,-40,
    -30, 0,  10, 15, 15, 10, 0, -30,
    -30, 5,  15, 20, 20, 15, 5, -30,
    -30, 0,  15, 20, 20, 15, 0, -30,
    -30, 5,  10, 15, 15, 10, 5, -30,
    -40,-20, 0,  5,  5,  0, -20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

// clang-format off
constexpr int bishop[SQUARE_CT] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

// clang-format off
constexpr int rook[SQUARE_CT] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

// clang-format off
constexpr int queen[SQUARE_CT] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// clang-format off
constexpr int king[SQUARE_CT] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

constexpr int egKing[SQUARE_CT] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// clang-format on

const int *pieceTbls[6] = {
    pawn, knight, bishop, rook, queen, king,
};

template <Color C> inline int pieceSquare(Board &board) { return 0; }

template <> inline int pieceSquare<WHITE>(Board &board) {
    int eval = 0;
    for (int i = 0; i < 64; i++) {
        Piece p = board.board[i];
        PieceT pt = getPcType(p);

        if (p < B_PAWN && p != NO_PC) {
            eval += pieceTbls[pt - 1][i];
        }
    }
    return eval;
}

template <> inline int pieceSquare<BLACK>(Board &board) {
    int eval = 0;
    for (int i = 0; i < 64; i++) {
        Piece p = board.board[i];
        PieceT pt = getPcType(p);

        if (p >= B_PAWN && p != NO_PC) {
            eval += pieceTbls[pt - 1][mirror(i)];
        }
    }
    return eval;
}

int eval(Board &board) {
    const int wMaterial = (PAWN_VAL * __builtin_popcountll(board.pieces(PAWN, WHITE))) +
                          (KNIGHT_VAL * __builtin_popcountll(board.pieces(KNIGHT, WHITE))) +
                          (BISHOP_VAL * __builtin_popcountll(board.pieces(BISHOP, WHITE))) +
                          (ROOK_VAL * __builtin_popcountll(board.pieces(ROOK, WHITE))) +
                          (QUEEN_VAL * __builtin_popcountll(board.pieces(QUEEN, WHITE)));

    const int bMaterial = (PAWN_VAL * __builtin_popcountll(board.pieces(PAWN, BLACK))) +
                          (KNIGHT_VAL * __builtin_popcountll(board.pieces(KNIGHT, BLACK))) +
                          (BISHOP_VAL * __builtin_popcountll(board.pieces(BISHOP, BLACK))) +
                          (ROOK_VAL * __builtin_popcountll(board.pieces(ROOK, BLACK))) +
                          (QUEEN_VAL * __builtin_popcountll(board.pieces(QUEEN, BLACK)));

    const auto color = (board.turn == WHITE) ? 1 : -1;

    moveList mList = {0};
    generate(board, &mList);
    moveList otherMoves = {0};
    makeNullMove(board);
    generate(board, &otherMoves);
    unmakeNullMove(board);

    const auto pcSqEval = pieceSquare<WHITE>(board) - pieceSquare<BLACK>(board);

    int mobility = 0;
    if (board.turn == WHITE) {
        mobility = mList.nMoves - otherMoves.nMoves;
    } else {
        mobility = otherMoves.nMoves - mList.nMoves;
    }

    return ((0.10 * (mobility)) + (1.1 * (wMaterial - bMaterial)) + pcSqEval + 10) * color;
}

} // namespace Yayo
#endif // SEARCH_H_
