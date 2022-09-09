#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.h"
#include "movegen.h"
#include "util.h"
#include <thread>

namespace Yayo {
namespace { // penalties
constexpr int UNMOVED_PASSED   = -20;
constexpr int DOUBLED_PENALTY  = -10;
constexpr int ISOLATED_PENALTY = -5;
constexpr int TEMPO            = 10;
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

template <Color C> constexpr Bitboard backwardPawns(Board &board) {
    constexpr Direction Up   = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);

    const Bitboard pawns      = board.pieces(PAWN, C);
    const Bitboard enemyPawns = board.pieces(PAWN, ~C);
    const Bitboard stopSquare = shift<Up>(pawns);

    const Bitboard candidateBackwardPawns = shift<Down>(pawnDblAttacks<~C>(enemyPawns) & stopSquare) & pawns;
    const Bitboard defendedStopSquares    = pawnDblAttacks<C>(pawns) & stopSquare;
    const Bitboard backwardPawns          = candidateBackwardPawns & ~shift<Down>(defendedStopSquares);

    return backwardPawns;
}

template <Color C> constexpr int backwardPawnScore(Board &board) { return -1 * popcount(backwardPawns<C>(board)); }

template <Color C> constexpr int isolatedPawnCount(Board &board) {
    int count = 0;

    Bitboard pawns = board.pieces(PAWN, C);
    while (pawns) {
        Square psq = Square(lsb_index(pawns));
        if (!(isolatedPawnMasks[psq] & board.pieces(PAWN, C))) {
            count++;
        }

        pawns &= pawns - 1;
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
    return popcount(doubledPawns<C>(board)) * DOUBLED_PENALTY;
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

constexpr int dotProduct(Bitboard moves, const int weights[64]) {
    int res      = 0;
    Bitboard bit = 1;

    for (int sq = 0; sq < 64; sq++, bit += bit) {
        if ((moves & bit) && (weights[sq] > 0))
            res += weights[sq];
    }

    return res;
}

template <Color C> constexpr int mobilityScore(Board &board) {
    constexpr Direction Up   = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);

    Bitboard knights = board.pieces(KNIGHT, C);
    Bitboard bishops = board.pieces(BISHOP, C);
    Bitboard rooks   = board.pieces(ROOK, C);
    Bitboard queens  = board.pieces(QUEEN, C);

    const Bitboard friendlyPawns = board.pieces(PAWN, C);
    const Bitboard enemyPawns    = board.pieces(PAWN, ~C);

    const Bitboard enemyPawnAttacks     = pawnDblAttacks<~C>(enemyPawns);
    const Bitboard secondThirdRankPawns = friendlyPawns & (RANK_2BB | RANK_3BB);
    const Bitboard blockedPawns         = shift<Down>(enemyPawns) & friendlyPawns;
    const Bitboard friendlyKing         = board.pieces(KING, C);
    const Bitboard friendlyQueens       = board.pieces(QUEEN, C);

    const Bitboard excludedSquares =
        enemyPawnAttacks | secondThirdRankPawns | blockedPawns | friendlyKing | friendlyQueens;

    int knightMobility = 0;
    while (knights) {
        Square knightSq      = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;
        knightMobility += dotProduct(knightMoves, knight);
        knights &= knights - 1;
    }

    int bishopMobility = 0;
    while (bishops) {
        Square bishopSq      = Square(lsb_index(bishops));
        Bitboard bishopMoves = getBishopAttacks(bishopSq, board.pieces()) & ~excludedSquares;
        bishopMobility += dotProduct(bishopMoves, bishop);
        bishops &= bishops - 1;
    }

    int rookMobility = 0;
    while (rooks) {
        Square rookSq      = Square(lsb_index(rooks));
        Bitboard rookMoves = getRookAttacks(rookSq, board.pieces()) & ~excludedSquares;
        rookMobility += dotProduct(rookMoves, rook);
        rooks &= rooks - 1;
    }

    int queenMobility = 0;
    while (queens) {
        Square queenSq      = Square(lsb_index(queens));
        Bitboard queenMoves = getRookAttacks(queenSq, board.pieces()) | getBishopAttacks(queenSq, board.pieces());
        queenMoves &= ~excludedSquares;
        queenMobility += dotProduct(queenMoves, queen);
        queens &= queens - 1;
    }

    int mobilityScore = knightMobility + bishopMobility + rookMobility + queenMobility;

    // std::cout << ((C == WHITE) ? "WHITE: " : "BLACK: ") << std::endl;
    // std::cout << "knight mobility evaluation: " << knightMobility << std::endl;
    // std::cout << "bishop mobility evaluation: " << bishopMobility << std::endl;
    // std::cout << "rook mobility evaluation: " << rookMobility << std::endl;
    // std::cout << "queen mobility evaluation: " << queenMobility << std::endl;
    // std::cout << "total mobility evaluation: " << mobilityScore / 2 << std::endl;
    // std::cout << std::endl;

    return mobilityScore / 2;
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
    eval += mobility;
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);

    return  eval * color;
}

} // namespace Yayo
#endif // SEARCH_H_
