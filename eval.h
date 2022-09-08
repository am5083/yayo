#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.h"
#include "movegen.h"
#include "util.h"
#include <thread>

namespace Yayo {
namespace { // penalties
constexpr int UNMOVED_PASSED  = -20;
constexpr int DOUBLED_PENALTY = -10;
constexpr int TEMPO           = 10;
} // namespace
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

template <Color C> constexpr int isolatedPawnCount(Board &board) {
    int count = 0;

    Bitboard pawns = board.pieces(PAWN, C);
    while (pawns) {
        Square psq = Square(lsb_index(pawns));
        if (!(isolatedPawnMasks[psq] & board.pieces(PAWN, C))) {
            count++;
        }

        pawns &= -pawns;
    }

    return count;
}

template <Color C> constexpr Bitboard doubledPawns(Board &board) {
    Bitboard pawns        = board.pieces(PAWN, C);
    Bitboard blockedPawns = 0;

    while (pawns) {
        Bitboard b      = SQUARE_BB(Square(lsb_index(pawns)));
        Bitboard pushes = fill<pushDirection(C)>(shift<pushDirection(C)>(b));

        if (pushes & board.pieces(PAWN, C)) {
            blockedPawns |= b;
        }

        pawns &= pawns - 1;
    }

    return blockedPawns;
}

template <Color C> constexpr int passedBlockBonus(Bitboard passedPawns, Board &board) {
    constexpr Direction Up    = pushDirection(C);
    constexpr Direction Down  = pushDirection(~C);
    constexpr Rank startRank  = (C == WHITE) ? RANK_2 : RANK_7;
    constexpr Rank singlePush = startRank + Up;

    int score = 0;
    while (passedPawns) {
        Square psq      = Square(lsb_index(passedPawns));
        Square pushSq   = psq + Up;
        Bitboard pushBB = SQUARE_BB(pushSq);
        passedPawns &= passedPawns - 1;
        passedPawns &= ~doubledPawns<C>(board);

        if (RANK_OF(psq) == startRank || RANK_OF(psq) == singlePush)
            continue;

        if (pushBB & board.pieces())
            continue;

        if (board.isSqAttacked(pushSq, board.pieces(), ~C))
            continue;

        int nAtk = 0, nDef = 0;
        bool pathAttacked     = false;
        const int rankBonus[] = {0, 0, 0, 40, 200, 260, 400, 0};

        Bitboard pushToQueen = fill<Up>(pushBB);
        while (pushToQueen) {
            Square sq     = Square(lsb_index(pushToQueen));
            bool attacked = board.isSqAttacked(sq, board.pieces(), ~C);
            bool defended = board.isSqAttacked(sq, board.pieces(), C);

            if (attacked && !defended) {
                score += rankBonus[RANK_OF(sq)] / 4;
                nAtk++;
            }

            if (attacked && defended) {
                score += rankBonus[RANK_OF(sq)] / 2;
                nDef++;
            }

            pushToQueen &= pushToQueen - 1;
        }

        if (nAtk || nDef) {
            continue;
        }

        score += rankBonus[RANK_OF(psq)];
    }

    return score;
}

template <Color C> constexpr int passedPawnScore(Board &board) {
    constexpr Direction Up   = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);
    Bitboard ourPawns        = board.pieces(PAWN, C);
    Bitboard enemyPawns      = board.pieces(PAWN, ~C);

    Bitboard opponentPawnSpan = fill<Down>(shift<Down>(enemyPawns));
    opponentPawnSpan |= shift<WEST>(opponentPawnSpan) | shift<EAST>(opponentPawnSpan);

    Bitboard passedPawns = board.pieces(PAWN, C) & ~opponentPawnSpan;

    Rank startRank       = (C == WHITE) ? RANK_2 : RANK_7;
    Bitboard startPassed = passedPawns & startRank;

    int score = 0;

    score += passedBlockBonus<C>(passedPawns, board);

    const int rankBonuses[] = {0, -20, 17, 15, 35, 175, 400};
    // const int rankBonuses[] = {0, 10, 17, 15, 62, 168, 278};
    while (passedPawns) {
        Square psq = Square(lsb_index(passedPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        score += rankBonuses[RANK_OF(psq)];
        passedPawns &= passedPawns - 1;
    }

    return score;
}

template <Color C> constexpr int doubledPawnPenalty(Board &board) {
    return 0; // popcount(doubledPawns<C>(board)) * DOUBLED_PENALTY;
}

template <Color C> inline int pieceSquare(Board &board) { return 0; }

template <> inline int pieceSquare<WHITE>(Board &board) {
    int eval = 0;
    for (int i = 0; i < 64; i++) {
        Piece p   = board.board[i];
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
        Piece p   = board.board[i];
        PieceT pt = getPcType(p);

        if (p >= B_PAWN && p != NO_PC) {
            eval += pieceTbls[pt - 1][mirror(i)];
        }
    }
    return eval;
}

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

    moveList otherMoves = {0};
    makeNullMove(board);
    generate(board, &otherMoves);
    unmakeNullMove(board);

    const auto pcSqEval = (pieceSquare<WHITE>(board) - pieceSquare<BLACK>(board));
    const auto passed = passedPawnScore<WHITE>(board) - passedPawnScore<BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<WHITE>(board) - doubledPawnPenalty<BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty;

    int mobility = 0;
    if (board.turn == WHITE) {
        mobility = mList.nMoves - otherMoves.nMoves;
    } else {
        mobility = otherMoves.nMoves - mList.nMoves;
    }

    std::cout << "isolated pawn count: " << isolatedPawnCount<WHITE>(board) << std::endl;

    return ((0.10 * mobility) + (wMaterial - bMaterial) + (1.2 * pcSqEval) + (0.3 * pawnStructureScore) + TEMPO) * color;
}

} // namespace Yayo
#endif // SEARCH_H_
