#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"
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

static const int *pieceTbls[6] = {
    pawn, knight, bishop, rook, queen, king,
};

struct Trace {
    int pawnScore[NUM_COLOR]                    = {0};
    int knightScore[NUM_COLOR]                  = {0};
    int bishopScore[NUM_COLOR]                  = {0};
    int rookScore[NUM_COLOR]                    = {0};
    int queenScore[NUM_COLOR]                   = {0};
    int pawnPcSq[NUM_COLOR][SQUARE_CT]          = {{0}};
    int knightPcSq[NUM_COLOR][SQUARE_CT]        = {{0}};
    int bishopPcSq[NUM_COLOR][SQUARE_CT]        = {{0}};
    int rookPcSq[NUM_COLOR][SQUARE_CT]          = {{0}};
    int queenPcSq[NUM_COLOR][SQUARE_CT]         = {{0}};
    int kingPcSq[NUM_COLOR][SQUARE_CT]          = {{0}};
    int passedPawn[8][NUM_COLOR]                = {{0}};
    int passedPawnBlockedAttacked[8][NUM_COLOR] = {{0}};
    int passedPawnBlockedDefended[8][NUM_COLOR] = {{0}};
    int passedPawnBlocked[8][NUM_COLOR]         = {{0}};
    int doubledPawns[NUM_COLOR]                 = {0};
    int isolatedPawns[NUM_COLOR]                = {0};
    int backwardPawns[NUM_COLOR]                = {0};
    int knightMobility[NUM_COLOR][9]            = {{0}};
    int bishopMobility[NUM_COLOR][14]           = {{0}};
    int rookMobility[NUM_COLOR][15]             = {{0}};
    int queenMobility[NUM_COLOR][28]            = {{0}};
    int kingMobility[NUM_COLOR]                 = {0};
    int evalTerms[511]                          = {0};
    void print();
};

extern Trace trace;

enum Tracing : bool { NO_TRACE, TRACE };

template <Color C> constexpr Bitboard doubledPawns(Board &board);
template <Color C> constexpr Bitboard backwardPawns(Board &board);
template <Color C> constexpr int backwardPawnScore(Board &board);
template <Color C> constexpr int passedPawnScore(Board &board);
template <Color C> constexpr int isolatedPawnCount(Board &board);
template <Color C> constexpr int passedBlockBonus(Bitboard passedPawns, Board &board);
template <Color C> constexpr int doubledPawnPenalty(Board &board);

template <Tracing T, Color C> constexpr int pieceSquare(Board &board);

template <Color C> constexpr int dotProduct(Bitboard moves, const int weights[64]);
template <Color C> constexpr int mobilityScore(Board &board);

template <Tracing T> int eval(Board &board, moveList &mList);

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

template <Tracing T = NO_TRACE, Color C> constexpr int backwardPawnScore(Board &board) {
    int numBackwardPawns = popcount(backwardPawns<C>(board));
    if (T) {
        trace.backwardPawns[C] = numBackwardPawns;
    }
    return -1 * numBackwardPawns;
}

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

template <Tracing T = NO_TRACE, Color C> constexpr int passedBlockBonus(Bitboard passedPawns, Board &board) {
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
                if (T) {
                    trace.passedPawnBlockedAttacked[RANK_OF(sq)][C]++;
                }
            }

            if (attacked && defended) {
                score += rankBonus[RANK_OF(sq)] / 2;
                nDef++;
                if (T) {
                    trace.passedPawnBlockedDefended[RANK_OF(sq)][C]++;
                }
            }

            pushToQueen &= pushToQueen - 1;
        }

        if (nAtk || nDef) {
            continue;
        }

        score += rankBonus[RANK_OF(psq)];
        if (T) {
            trace.passedPawnBlocked[RANK_OF(psq)][C]++;
        }
    }

    return score;
}

template <Tracing T = NO_TRACE, Color C> constexpr int passedPawnScore(Board &board) {
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

    score += passedBlockBonus<T, C>(passedPawns, board);

    const int rankBonuses[] = {0, -20, 17, 15, 35, 175, 400};
    // const int rankBonuses[] = {0, 10, 17, 15, 62, 168, 278};
    while (passedPawns) {
        Square psq = Square(lsb_index(passedPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        score += rankBonuses[RANK_OF(psq)];
        if (T) {
            trace.passedPawn[RANK_OF(psq)][C]++;
        }
        passedPawns &= passedPawns - 1;
    }

    return score;
}

template <Tracing T = NO_TRACE, Color C> constexpr int doubledPawnPenalty(Board &board) {
    const int numDoubledPawns = popcount(doubledPawns<C>(board));
    if (T) {
        trace.doubledPawns[C] = numDoubledPawns;
    }
    return numDoubledPawns * DOUBLED_PENALTY;
}

template <Tracing T = NO_TRACE, Color C> constexpr int pieceSquare(Board &board) {
    int eval = 0;
    for (int i = 0; i < 64; i++) {
        Piece p   = board.board[i];
        PieceT pt = getPcType(p);

        if (C == WHITE) {
            if (p < B_PAWN && p != NO_PC) {
                if (T) {
                    int *pcSq[] = {trace.pawnPcSq[WHITE], trace.knightPcSq[WHITE], trace.bishopPcSq[WHITE],
                                   trace.rookPcSq[WHITE], trace.queenPcSq[WHITE],  trace.kingPcSq[WHITE]};

                    pcSq[pt - 1][i] = 1;
                }

                eval += pieceTbls[pt - 1][i];
            }
        } else if (C == BLACK) {
            if (p >= B_PAWN && p != NO_PC) {
                if (T) {
                    int *pcSq[] = {trace.pawnPcSq[BLACK], trace.knightPcSq[BLACK], trace.bishopPcSq[BLACK],
                                   trace.rookPcSq[BLACK], trace.queenPcSq[BLACK],  trace.kingPcSq[BLACK]};

                    pcSq[pt - 1][mirror(i)] = 1;
                }

                eval += pieceTbls[pt - 1][mirror(i)];
            }
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

template <Tracing T = NO_TRACE, Color C> constexpr int mobilityScore(Board &board) {
    constexpr int KnightBonus[] = {-60, -50, -10, -5, 5, 15, 21, 30, 40};
    constexpr int BishopBonus[] = {-50, -20, 15, 30, 40, 55, 55, 60, 62, 70, 80, 83, 91, 96};
    constexpr int RookBonus[]   = {-60, -25, 0, 3, 4, 15, 20, 30, 40, 40, 40, 45, 60, 61, 70};
    constexpr int QueenBonus[]  = {-30, -15, -10, -10, 20, 25, 23, 35,  40,  55,  65,  68,  69,  70,
                                   70,  70,  71,  72,  74, 76, 90, 104, 105, 106, 112, 114, 114, 119};

    constexpr Direction Up   = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);

    Bitboard knights = board.pieces(KNIGHT, C);
    Bitboard bishops = board.pieces(BISHOP, C);
    Bitboard rooks   = board.pieces(ROOK, C);
    Bitboard queens  = board.pieces(QUEEN, C);

    const Bitboard friendlyPawns = board.pieces(PAWN, C);
    const Bitboard enemyPawns    = board.pieces(PAWN, ~C);

    const Bitboard secondThirdRank      = (C == WHITE) ? (RANK_2BB | RANK_3BB) : (RANK_7BB | RANK_6BB);
    const Bitboard enemyPawnAttacks     = pawnDblAttacks<~C>(enemyPawns);
    const Bitboard secondThirdRankPawns = friendlyPawns & secondThirdRank;
    const Bitboard blockedPawns         = shift<Down>(enemyPawns) & friendlyPawns;
    const Bitboard friendlyKing         = board.pieces(KING, C);
    const Bitboard friendlyQueens       = board.pieces(QUEEN, C);

    const Bitboard excludedSquares =
        enemyPawnAttacks | secondThirdRankPawns | blockedPawns | friendlyKing | friendlyQueens;

    Bitboards::print_bitboard(excludedSquares);

    int knightMobility = 0;
    while (knights) {
        Square knightSq      = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        knightMobility += KnightBonus[numMoves];
        if (T) {
            trace.knightMobility[C][numMoves]++;
        }
        knights &= knights - 1;
    }

    int bishopMobility = 0;
    while (bishops) {
        Square bishopSq      = Square(lsb_index(bishops));
        Bitboard bishopMoves = getBishopAttacks(bishopSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(bishopMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.bishopMobility[C][numMoves]++;
        }
        bishopMobility += BishopBonus[numMoves];
        bishops &= bishops - 1;
    }

    int rookMobility = 0;
    while (rooks) {
        Square rookSq      = Square(lsb_index(rooks));
        Bitboard rookMoves = getRookAttacks(rookSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(rookMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.rookMobility[C][numMoves]++;
        }
        rookMobility += RookBonus[numMoves];
        rooks &= rooks - 1;
    }

    int queenMobility = 0;
    while (queens) {
        Square queenSq      = Square(lsb_index(queens));
        Bitboard queenMoves = getRookAttacks(queenSq, board.pieces()) | getBishopAttacks(queenSq, board.pieces());
        queenMoves &= ~excludedSquares;

        int numMoves = popcount(queenMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.queenMobility[C][numMoves]++;
        }
        queenMobility += QueenBonus[numMoves];
        queens &= queens - 1;
    }

    const int kingMobility = popcount(kingAttacks[lsb_index(board.pieces(KING, C))]);

    if (T) {
        trace.kingMobility[C] = kingMobility;
    }

    const int mobilityScore = knightMobility + bishopMobility + rookMobility + queenMobility + kingMobility;

    // std::cout << ((C == WHITE) ? "WHITE: " : "BLACK: ") << std::endl;
    // std::cout << "knight mobility evaluation: " << knightMobility << std::endl;
    // std::cout << "bishop mobility evaluation: " << bishopMobility << std::endl;
    // std::cout << "rook mobility evaluation: " << rookMobility << std::endl;
    // std::cout << "queen mobility evaluation: " << queenMobility << std::endl;
    // std::cout << "total mobility evaluation: " << mobilityScore / 2 << std::endl;
    // std::cout << std::endl;

    return mobilityScore;
}

} // namespace Yayo
#endif // SEARCH_H_
