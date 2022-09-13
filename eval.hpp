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

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

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

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),
    S(98, 178), S(134, 173), S(61, 158), S(95, 134), S(68, 147), S(126, 132), S(34, 165), S(-11, 187),
    S(-6, 94),  S(7, 100),   S(26, 85),  S(31, 67),  S(65, 56),  S(56, 53),   S(25, 82),  S(-20, 84),
    S(-14, 32), S(13, 24),   S(6, 13),   S(21, 5),   S(23, -2),  S(12, 4),    S(17, 17),  S(-23, 17),
    S(-27, 13), S(-2, 9),    S(-5, -3),  S(12, -7),  S(17, -7),  S(6, -8),    S(10, 3),   S(-25, -1),
    S(-26, 4),  S(-4, 7),    S(-4, -6),  S(-10, 1),  S(3, 0),    S(3, -5),    S(33, -1),  S(-12, -8),
    S(-35, 13), S(-1, 8),    S(-20, 8),  S(-23, 10), S(-15, 13), S(24, 0),    S(38, 2),   S(-22, -7),
    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),
};

constexpr Score taperedKnightPcSq[SQUARE_CT] = {
    S(-167, -58), S(-89, -38), S(-34, -13), S(-49, -28), S(61, -31),  S(-97, -27), S(-15, -63), S(-107, -99),
    S(-73, -25),  S(-41, -8),  S(72, -25),  S(36, -2),   S(23, -9),   S(62, -25),  S(7, -24),   S(-17, -52),
    S(-47, -24),  S(60, -20),  S(37, 10),   S(65, 9),    S(84, -1),   S(129, -9),  S(73, -19),  S(44, -41),
    S(-9, -17),   S(17, 3),    S(19, 22),   S(53, 22),   S(37, 22),   S(69, 11),   S(18, 8),    S(22, -18),
    S(-13, -18),  S(4, -6),    S(16, 16),   S(13, 25),   S(28, 16),   S(19, 17),   S(21, 4),    S(-8, -18),
    S(-23, -23),  S(-9, -3),   S(12, -1),   S(10, 15),   S(19, 10),   S(17, -3),   S(25, -20),  S(-16, -22),
    S(-29, -42),  S(-53, -20), S(-12, -10), S(-3, -5),   S(-1, -2),   S(18, -20),  S(-14, -23), S(-19, -44),
    S(-105, -29), S(-21, -51), S(-58, -23), S(-33, -15), S(-17, -22), S(-28, -18), S(-19, -50), S(-23, -64),
};

constexpr Score taperedBishopPcSq[SQUARE_CT] = {
    S(-29, -14), S(4, -21),  S(-82, -11), S(-37, -8),  S(-25, -7), S(-42, -9),  S(7, -17),  S(-8, -24),
    S(-26, -8),  S(16, -4),  S(-18, 7),   S(-13, -12), S(30, -3),  S(59, -13),  S(18, -4),  S(-47, -14),
    S(-16, 2),   S(37, -8),  S(43, 0),    S(40, -1),   S(35, -2),  S(50, 6),    S(37, 0),   S(-2, 4),
    S(-4, -3),   S(5, 9),    S(19, 12),   S(50, 9),    S(37, 14),  S(37, 10),   S(7, 3),    S(-2, 2),
    S(-6, -6),   S(13, 3),   S(13, 13),   S(26, 19),   S(34, 7),   S(12, 10),   S(10, -3),  S(4, -9),
    S(0, -12),   S(15, -3),  S(15, 8),    S(15, 10),   S(14, 13),  S(27, 3),    S(18, -7),  S(10, -15),
    S(4, -14),   S(15, -18), S(16, -7),   S(0, -1),    S(7, 4),    S(21, -9),   S(33, -15), S(1, -27),
    S(-33, -23), S(-3, -9),  S(-14, -23), S(-21, -5),  S(-13, -9), S(-12, -16), S(-39, -5), S(-21, -17),
};

constexpr Score taperedRookPcSq[SQUARE_CT] = {
    S(32, 13),  S(42, 10),  S(32, 18),  S(51, 15),  S(63, 12), S(9, 12),  S(31, 8),   S(43, 5),
    S(27, 11),  S(32, 13),  S(58, 13),  S(62, 11),  S(80, -3), S(67, 3),  S(26, 8),   S(44, 3),
    S(-5, 7),   S(19, 7),   S(26, 7),   S(36, 5),   S(17, 4),  S(45, -3), S(61, -5),  S(16, -3),
    S(-24, 4),  S(-11, 3),  S(7, 13),   S(26, 1),   S(24, 2),  S(35, 1),  S(-8, -1),  S(-20, 2),
    S(-36, 3),  S(-26, 5),  S(-12, 8),  S(-1, 4),   S(9, -5),  S(-7, -6), S(6, -8),   S(-23, -11),
    S(-45, -4), S(-25, 0),  S(-16, -5), S(-17, -1), S(3, -7),  S(0, -12), S(-5, -8),  S(-33, -16),
    S(-44, -6), S(-16, -6), S(-20, 0),  S(-9, 2),   S(-1, -9), S(11, -9), S(-6, -11), S(-71, -3),
    S(-19, -9), S(-13, 2),  S(1, 3),    S(17, -1),  S(16, -5), S(7, -13), S(-37, 4),  S(-26, -20),
};

constexpr Score taperedQueenPcSq[SQUARE_CT] = {
    S(-28, -9),  S(0, 22),    S(29, 22),  S(12, 27),  S(59, 27),  S(44, 19),   S(43, 10),   S(45, 20),
    S(-24, -17), S(-39, 20),  S(-5, 32),  S(1, 41),   S(-16, 58), S(57, 25),   S(28, 30),   S(54, 0),
    S(-13, -20), S(-17, 6),   S(7, 9),    S(8, 49),   S(29, 47),  S(56, 35),   S(47, 19),   S(57, 9),
    S(-27, 3),   S(-27, 22),  S(-16, 24), S(-16, 45), S(-1, 57),  S(17, 40),   S(-2, 57),   S(1, 36),
    S(-9, -18),  S(-26, 28),  S(-9, 19),  S(-10, 47), S(-2, 31),  S(-4, 34),   S(3, 39),    S(-3, 23),
    S(-14, -16), S(2, -27),   S(-11, 15), S(-2, 6),   S(-5, 9),   S(2, 17),    S(14, 10),   S(5, 5),
    S(-35, -22), S(-8, -23),  S(11, -30), S(2, -16),  S(8, -16),  S(15, -23),  S(-3, -36),  S(1, -32),
    S(-1, -33),  S(-18, -28), S(-9, -22), S(10, -43), S(-15, -5), S(-25, -32), S(-31, -20), S(-50, -41),
};

constexpr Score taperedKingPcSq[SQUARE_CT] = {
    S(-65, -74), S(23, -35), S(16, -18), S(-15, -18), S(-56, -11), S(-34, 15),  S(2, 4),    S(13, -17),
    S(29, -12),  S(-1, 17),  S(-20, 14), S(-7, 17),   S(-8, 17),   S(-4, 38),   S(-38, 23), S(-29, 11),
    S(-9, 10),   S(24, 17),  S(2, 23),   S(-16, 15),  S(-20, 20),  S(6, 45),    S(22, 44),  S(-22, 13),
    S(-17, -8),  S(-20, 22), S(-12, 24), S(-27, 27),  S(-30, 26),  S(-25, 33),  S(-14, 26), S(-36, 3),
    S(-49, -18), S(-1, -4),  S(-27, 21), S(-39, 24),  S(-46, 27),  S(-44, 23),  S(-33, 9),  S(-51, -11),
    S(-14, -19), S(-14, -3), S(-22, 11), S(-46, 21),  S(-44, 23),  S(-30, 16),  S(-15, 7),  S(-27, -9),
    S(1, -27),   S(7, -11),  S(-8, 4),   S(-64, 13),  S(-43, 14),  S(-16, 4),   S(9, -5),   S(8, -17),
    S(-15, -53), S(36, -34), S(12, -21), S(-54, -11), S(8, -28),   S(-28, -14), S(24, -24), S(14, -43),
};

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

static const Score *pcSq[] = {taperedPawnPcSq, taperedKnightPcSq, taperedBishopPcSq,
                              taperedRookPcSq, taperedQueenPcSq,  taperedKingPcSq};

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
    int eval    = 0;
    int phase   = 0;
    int mgScore = 0;
    int egScore = 0;

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

                mgScore += MgScore(pcSq[pt - 1][i]);
                egScore += EgScore(pcSq[pt - 1][i]);
                phase += gamePhaseValues[pt - 1];

                eval += pieceTbls[pt - 1][i];
            }
        } else if (C == BLACK) {
            if (p >= B_PAWN && p != NO_PC) {
                if (T) {
                    int *pcSq[] = {trace.pawnPcSq[BLACK], trace.knightPcSq[BLACK], trace.bishopPcSq[BLACK],
                                   trace.rookPcSq[BLACK], trace.queenPcSq[BLACK],  trace.kingPcSq[BLACK]};

                    pcSq[pt - 1][mirror(i)] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][mirror(i)]);
                egScore += EgScore(pcSq[pt - 1][mirror(i)]);
                phase += gamePhaseValues[pt - 1];
                eval += pieceTbls[pt - 1][mirror(i)];
            }
        }
    }

    int mgPhase = phase;
    if (mgPhase > 24)
        mgPhase = 24;
    int egPhase = 24 - mgPhase;

    // std::cout << "---------------\n";
    // std::cout << "mgScore: " << mgScore << std::endl;
    // std::cout << "egScore: " << egScore << std::endl;
    // std::cout << "mgPhase: " << mgPhase << std::endl;
    // std::cout << "egPhase: " << egPhase << std::endl;
    // std::cout << "mgScore*mgPhase: " << mgScore * mgPhase << std::endl;
    // std::cout << "egScore*egPhase: " << egScore * egPhase << std::endl;
    // std::cout << "score: " << ((mgScore * mgPhase) + (egScore * egPhase)) / 24 << std::endl;
    // std::cout << "---------------\n";

    return (mgScore * mgPhase + egScore * egPhase) / 24;
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

    int phase   = 0;
    int mgScore = 0;
    int egScore = 0;

    int knightMobility = 0;
    while (knights) {
        Square knightSq      = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.knightMobility[C][numMoves]++;
        }

        mgScore += MgScore(KnightMobilityScore[numMoves]);
        egScore += EgScore(KnightMobilityScore[numMoves]);

        knightMobility += MgScore(KnightMobilityScore[numMoves]);

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

        mgScore += MgScore(BishopMobilityScore[numMoves]);
        egScore += EgScore(BishopMobilityScore[numMoves]);

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

        mgScore += MgScore(RookMobilityScore[numMoves]);
        egScore += EgScore(RookMobilityScore[numMoves]);

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

        mgScore += MgScore(QueenMobilityScore[numMoves]);
        egScore += EgScore(QueenMobilityScore[numMoves]);

        queenMobility += MgScore(QueenMobilityScore[numMoves]);
        queens &= queens - 1;
    }

    const int kingMobility = popcount(kingAttacks[lsb_index(board.pieces(KING, C))]);

    if (T) {
        trace.kingMobility[C] = kingMobility;
    }

    for (int i = 0; i < 64; i++) {
        PieceT pt = getPcType(board.board[i]);
        phase += gamePhaseValues[pt - 1];
    }

    int mgPhase = phase;
    if (mgPhase > 24)
        mgPhase = 24;
    int egPhase = 24 - mgPhase;

    const int taperedMobilityScore = (mgScore * mgPhase + egScore * egPhase) / 24;

    const int mobilityScore = knightMobility + bishopMobility + rookMobility + queenMobility + kingMobility;

    // std::cout << ((C == WHITE) ? "WHITE: " : "BLACK: ") << std::endl;
    // std::cout << "knight mobility evaluation: " << MgScore(Score(knightMobility)) << std::endl;
    // std::cout << "bishop mobility evaluation: " << bishopMobility << std::endl;
    // std::cout << "rook mobility evaluation: " << rookMobility << std::endl;
    // std::cout << "queen mobility evaluation: " << queenMobility << std::endl;
    // std::cout << "total mobility evaluation: " << MgScore(Score(mobilityScore)) / 2 << std::endl;
    // std::cout << std::endl;

    return taperedMobilityScore;
}

} // namespace Yayo
#endif // SEARCH_H_
