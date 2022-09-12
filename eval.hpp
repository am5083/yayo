#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"
#include <thread>

namespace Yayo {

enum Score : int { NO_SCORE };

#define S(mg, eg) (MakeScore(mg, eg))

constexpr Score MakeScore(const int mg, const int eg) { return Score((int)((unsigned int)eg << 16) + mg); }

inline std::int16_t MgScore(const Score score) {
    union {
        std::uint16_t upper;
        std::int16_t lower;
    } mg = {std::uint16_t(unsigned(score))};

    return mg.lower;
}

constexpr std::int16_t EgScore(const Score score) {
    union {
        std::uint16_t upper;
        std::int16_t lower;
    } eg = {std::uint16_t(unsigned(score + 0x8000) >> 16)};

    return eg.lower;
}

namespace { // penalties
constexpr int UNMOVED_PASSED   = -20;
constexpr int DOUBLED_PENALTY  = -10;
constexpr int ISOLATED_PENALTY = -5;
constexpr int TEMPO            = 10;
} // namespace

constexpr Score KnightMobilityScore[] = {S(-60, -80), S(-50, -30), S(-10, -20), S(-5, 10), S(5, 10),
                                         S(15, 14),   S(21, 15),   S(30, 21),   S(40, 30)};

constexpr Score BishopMobilityScore[] = {S(-50, -60), S(-20, -25), S(15, -10), S(30, 12), S(40, 21),
                                         S(55, 49),   S(55, 55),   S(60, 58),  S(62, 65), S(70, 72),
                                         S(80, 78),   S(83, 87),   S(91, 88),  S(96, 98)};

constexpr Score RookMobilityScore[] = {S(-60, -80), S(-25, -15), S(0, 20),   S(3, 40),   S(4, 70),
                                       S(15, 100),  S(20, 102),  S(30, 122), S(40, 133), S(40, 139),
                                       S(40, 153),  S(45, 160),  S(60, 165), S(61, 170), S(70, 175)};

constexpr Score QueenMobilityScore[] = {S(-30, -50), S(-15, -30), S(-10, -10), S(-10, 20),  S(20, 40),   S(25, 55),
                                        S(23, 60),   S(35, 73),   S(40, 76),   S(55, 95),   S(65, 95),   S(68, 101),
                                        S(69, 124),  S(70, 128),  S(70, 132),  S(70, 133),  S(71, 136),  S(72, 140),
                                        S(74, 147),  S(76, 149),  S(90, 153),  S(104, 169), S(105, 171), S(106, 171),
                                        S(112, 178), S(114, 185), S(114, 187), S(119, 221)};

constexpr Score blockedPassedPawnRankBonus[] = {S(0, 0),     S(0, 0),     S(0, 0),     S(40, 40),
                                                S(200, 200), S(260, 260), S(400, 400), S(0, 0)};

constexpr Score passedPawnRankBonus[] = {S(0, 0),   S(-20, -20), S(17, 17),  S(15, 15),
                                         S(35, 35), S(175, 175), S(400, 400)};

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

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
        bool pathAttacked = false;

        Bitboard pushToQueen = fill<Up>(pushBB);
        while (pushToQueen) {
            Square sq     = Square(lsb_index(pushToQueen));
            bool attacked = board.isSqAttacked(sq, board.pieces(), ~C);
            bool defended = board.isSqAttacked(sq, board.pieces(), C);

            if (attacked && !defended) {
                score += MgScore(blockedPassedPawnRankBonus[RANK_OF(sq)]) / 4;
                nAtk++;
                if (T) {
                    trace.passedPawnBlockedAttacked[RANK_OF(sq)][C]++;
                }
            }

            if (attacked && defended) {
                score += MgScore(blockedPassedPawnRankBonus[RANK_OF(sq)]) / 2;
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

        score += MgScore(blockedPassedPawnRankBonus[RANK_OF(psq)]);
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

    while (passedPawns) {
        Square psq = Square(lsb_index(passedPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        score += MgScore(passedPawnRankBonus[RANK_OF(psq)]);
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

    int knightMobility = 0;
    while (knights) {
        Square knightSq      = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        knightMobility += MgScore(KnightMobilityScore[numMoves]);
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
        bishopMobility += MgScore(BishopMobilityScore[numMoves]);
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
        rookMobility += MgScore(RookMobilityScore[numMoves]);
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
        queenMobility += MgScore(QueenMobilityScore[numMoves]);
        queens &= queens - 1;
    }

    const int kingMobility = popcount(kingAttacks[lsb_index(board.pieces(KING, C))]);

    if (T) {
        trace.kingMobility[C] = kingMobility;
    }

    const int mobilityScore = knightMobility + bishopMobility + rookMobility + queenMobility + kingMobility;

    // std::cout << ((C == WHITE) ? "WHITE: " : "BLACK: ") << std::endl;
    // std::cout << "knight mobility evaluation: " << MgScore(Score(knightMobility)) << std::endl;
    // std::cout << "bishop mobility evaluation: " << bishopMobility << std::endl;
    // std::cout << "rook mobility evaluation: " << rookMobility << std::endl;
    // std::cout << "queen mobility evaluation: " << queenMobility << std::endl;
    // std::cout << "total mobility evaluation: " << MgScore(Score(mobilityScore)) / 2 << std::endl;
    // std::cout << std::endl;

    return mobilityScore;
}

} // namespace Yayo
#endif // SEARCH_H_
