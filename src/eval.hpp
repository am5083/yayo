/*
**    Yayo is a UCI chess engine written by am5083 (am@kvasm.us)
**    Copyright (C) 2022 Ahmed Mohamed (am@kvasm.us)
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEARCH_H_
#define SEARCH_H_
#include "board.hpp"
#include "move.hpp"
#include "tuned.hpp"
#include "util.hpp"
#include <thread>

namespace Yayo {
namespace {
constexpr int TEMPO = 10;
} // namespace

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

constexpr Score blockedPassedPawnRankBonus[] = {S(0, 0),     S(0, 0),     S(0, 0),     S(40, 40),
                                                S(200, 200), S(260, 260), S(400, 400), S(0, 0)};

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

constexpr Score passedPawnRankBonus[] = {S(0, 0),   S(-20, -20), S(17, 17),   S(15, 15),
                                         S(35, 35), S(175, 175), S(400, 400), S(0, 0)};

constexpr Score doubledPawnRankPenalty[8] = {S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10),
                                             S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10)};

constexpr Score isolatedPawnRankBonus[8] = {S(-6, -6), S(-6, -6), S(-6, -6), S(-6, -6),
                                            S(-6, -6), S(-6, -6), S(-6, -6), S(-6, -6)};

constexpr Score backwardPawnRankBonus[8] = {S(-15, -15), S(-15, -15), S(-15, -15), S(-15, -15),
                                            S(-15, -15), S(-15, -15), S(-15, -15), S(-15, -15)};

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

static const Score *pcSq[] = {taperedPawnPcSq, taperedKnightPcSq, taperedBishopPcSq,
                              taperedRookPcSq, taperedQueenPcSq,  taperedKingPcSq};

struct Trace {
    int pawnScore[NUM_COLOR]             = {0};
    int knightScore[NUM_COLOR]           = {0};
    int bishopScore[NUM_COLOR]           = {0};
    int rookScore[NUM_COLOR]             = {0};
    int queenScore[NUM_COLOR]            = {0};
    int pawnPcSq[SQUARE_CT][NUM_COLOR]   = {{0}};
    int knightPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int bishopPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int rookPcSq[SQUARE_CT][NUM_COLOR]   = {{0}};
    int queenPcSq[SQUARE_CT][NUM_COLOR]  = {{0}};
    int kingPcSq[SQUARE_CT][NUM_COLOR]   = {{0}};
    int passedPawn[8][NUM_COLOR]         = {{0}};
    int doubledPawns[8][NUM_COLOR]       = {0};
    int isolatedPawns[8][NUM_COLOR]      = {0};
    int backwardPawns[8][NUM_COLOR]      = {0};
    int knightMobility[9][NUM_COLOR]     = {{0}};
    int bishopMobility[14][NUM_COLOR]    = {{0}};
    int rookMobility[15][NUM_COLOR]      = {{0}};
    int queenMobility[28][NUM_COLOR]     = {{0}};
};

struct EvalWeights {
    const Score pawnScore   = S(PAWN_VAL, PAWN_VAL);
    const Score knightScore = S(KNIGHT_VAL, KNIGHT_VAL);
    const Score bishopScore = S(BISHOP_VAL, BISHOP_VAL);
    const Score rookScore   = S(ROOK_VAL, ROOK_VAL);
    const Score queenScore  = S(QUEEN_VAL, QUEEN_VAL);

    const Score taperedPawnPcSq[SQUARE_CT] = {
        S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),
        S(98, 178), S(134, 173), S(61, 158), S(95, 134), S(68, 147), S(126, 132), S(34, 165), S(-11, 187),
        S(-6, 94),  S(7, 100),   S(26, 85),  S(31, 67),  S(65, 56),  S(56, 53),   S(25, 82),  S(-20, 84),
        S(-14, 32), S(13, 24),   S(6, 13),   S(21, 5),   S(23, -2),  S(12, 4),    S(17, 17),  S(-23, 17),
        S(-27, 13), S(-2, 9),    S(-5, -3),  S(12, -7),  S(17, -7),  S(6, -8),    S(10, 3),   S(-25, -1),
        S(-26, 4),  S(-4, 7),    S(-4, -6),  S(-10, 1),  S(3, 0),    S(3, -5),    S(33, -1),  S(-12, -8),
        S(-35, 13), S(-1, 8),    S(-20, 8),  S(-23, 10), S(-15, 13), S(24, 0),    S(38, 2),   S(-22, -7),
        S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),    S(0, 0),
    };

    const Score taperedKnightPcSq[SQUARE_CT] = {
        S(-167, -58), S(-89, -38), S(-34, -13), S(-49, -28), S(61, -31),  S(-97, -27), S(-15, -63), S(-107, -99),
        S(-73, -25),  S(-41, -8),  S(72, -25),  S(36, -2),   S(23, -9),   S(62, -25),  S(7, -24),   S(-17, -52),
        S(-47, -24),  S(60, -20),  S(37, 10),   S(65, 9),    S(84, -1),   S(129, -9),  S(73, -19),  S(44, -41),
        S(-9, -17),   S(17, 3),    S(19, 22),   S(53, 22),   S(37, 22),   S(69, 11),   S(18, 8),    S(22, -18),
        S(-13, -18),  S(4, -6),    S(16, 16),   S(13, 25),   S(28, 16),   S(19, 17),   S(21, 4),    S(-8, -18),
        S(-23, -23),  S(-9, -3),   S(12, -1),   S(10, 15),   S(19, 10),   S(17, -3),   S(25, -20),  S(-16, -22),
        S(-29, -42),  S(-53, -20), S(-12, -10), S(-3, -5),   S(-1, -2),   S(18, -20),  S(-14, -23), S(-19, -44),
        S(-105, -29), S(-21, -51), S(-58, -23), S(-33, -15), S(-17, -22), S(-28, -18), S(-19, -50), S(-23, -64),
    };

    const Score taperedBishopPcSq[SQUARE_CT] = {
        S(-29, -14), S(4, -21),  S(-82, -11), S(-37, -8),  S(-25, -7), S(-42, -9),  S(7, -17),  S(-8, -24),
        S(-26, -8),  S(16, -4),  S(-18, 7),   S(-13, -12), S(30, -3),  S(59, -13),  S(18, -4),  S(-47, -14),
        S(-16, 2),   S(37, -8),  S(43, 0),    S(40, -1),   S(35, -2),  S(50, 6),    S(37, 0),   S(-2, 4),
        S(-4, -3),   S(5, 9),    S(19, 12),   S(50, 9),    S(37, 14),  S(37, 10),   S(7, 3),    S(-2, 2),
        S(-6, -6),   S(13, 3),   S(13, 13),   S(26, 19),   S(34, 7),   S(12, 10),   S(10, -3),  S(4, -9),
        S(0, -12),   S(15, -3),  S(15, 8),    S(15, 10),   S(14, 13),  S(27, 3),    S(18, -7),  S(10, -15),
        S(4, -14),   S(15, -18), S(16, -7),   S(0, -1),    S(7, 4),    S(21, -9),   S(33, -15), S(1, -27),
        S(-33, -23), S(-3, -9),  S(-14, -23), S(-21, -5),  S(-13, -9), S(-12, -16), S(-39, -5), S(-21, -17),
    };

    const Score taperedRookPcSq[SQUARE_CT] = {
        S(32, 13),  S(42, 10),  S(32, 18),  S(51, 15),  S(63, 12), S(9, 12),  S(31, 8),   S(43, 5),
        S(27, 11),  S(32, 13),  S(58, 13),  S(62, 11),  S(80, -3), S(67, 3),  S(26, 8),   S(44, 3),
        S(-5, 7),   S(19, 7),   S(26, 7),   S(36, 5),   S(17, 4),  S(45, -3), S(61, -5),  S(16, -3),
        S(-24, 4),  S(-11, 3),  S(7, 13),   S(26, 1),   S(24, 2),  S(35, 1),  S(-8, -1),  S(-20, 2),
        S(-36, 3),  S(-26, 5),  S(-12, 8),  S(-1, 4),   S(9, -5),  S(-7, -6), S(6, -8),   S(-23, -11),
        S(-45, -4), S(-25, 0),  S(-16, -5), S(-17, -1), S(3, -7),  S(0, -12), S(-5, -8),  S(-33, -16),
        S(-44, -6), S(-16, -6), S(-20, 0),  S(-9, 2),   S(-1, -9), S(11, -9), S(-6, -11), S(-71, -3),
        S(-19, -9), S(-13, 2),  S(1, 3),    S(17, -1),  S(16, -5), S(7, -13), S(-37, 4),  S(-26, -20),
    };

    const Score taperedQueenPcSq[SQUARE_CT] = {
        S(-28, -9),  S(0, 22),    S(29, 22),  S(12, 27),  S(59, 27),  S(44, 19),   S(43, 10),   S(45, 20),
        S(-24, -17), S(-39, 20),  S(-5, 32),  S(1, 41),   S(-16, 58), S(57, 25),   S(28, 30),   S(54, 0),
        S(-13, -20), S(-17, 6),   S(7, 9),    S(8, 49),   S(29, 47),  S(56, 35),   S(47, 19),   S(57, 9),
        S(-27, 3),   S(-27, 22),  S(-16, 24), S(-16, 45), S(-1, 57),  S(17, 40),   S(-2, 57),   S(1, 36),
        S(-9, -18),  S(-26, 28),  S(-9, 19),  S(-10, 47), S(-2, 31),  S(-4, 34),   S(3, 39),    S(-3, 23),
        S(-14, -16), S(2, -27),   S(-11, 15), S(-2, 6),   S(-5, 9),   S(2, 17),    S(14, 10),   S(5, 5),
        S(-35, -22), S(-8, -23),  S(11, -30), S(2, -16),  S(8, -16),  S(15, -23),  S(-3, -36),  S(1, -32),
        S(-1, -33),  S(-18, -28), S(-9, -22), S(10, -43), S(-15, -5), S(-25, -32), S(-31, -20), S(-50, -41),
    };

    const Score taperedKingPcSq[SQUARE_CT] = {
        S(-65, -74), S(23, -35), S(16, -18), S(-15, -18), S(-56, -11), S(-34, 15),  S(2, 4),    S(13, -17),
        S(29, -12),  S(-1, 17),  S(-20, 14), S(-7, 17),   S(-8, 17),   S(-4, 38),   S(-38, 23), S(-29, 11),
        S(-9, 10),   S(24, 17),  S(2, 23),   S(-16, 15),  S(-20, 20),  S(6, 45),    S(22, 44),  S(-22, 13),
        S(-17, -8),  S(-20, 22), S(-12, 24), S(-27, 27),  S(-30, 26),  S(-25, 33),  S(-14, 26), S(-36, 3),
        S(-49, -18), S(-1, -4),  S(-27, 21), S(-39, 24),  S(-46, 27),  S(-44, 23),  S(-33, 9),  S(-51, -11),
        S(-14, -19), S(-14, -3), S(-22, 11), S(-46, 21),  S(-44, 23),  S(-30, 16),  S(-15, 7),  S(-27, -9),
        S(1, -27),   S(7, -11),  S(-8, 4),   S(-64, 13),  S(-43, 14),  S(-16, 4),   S(9, -5),   S(8, -17),
        S(-15, -53), S(36, -34), S(12, -21), S(-54, -11), S(8, -28),   S(-28, -14), S(24, -24), S(14, -43),
    };

    const Score passedPawnRankBonus[8] = {S(0, 0),   S(-20, -20), S(17, 17),   S(15, 15),
                                          S(35, 35), S(175, 175), S(400, 400), S(0, 0)};

    const Score doubledPawnRankBonus[8] = {S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10),
                                           S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10)};

    const Score isolatedPawnRankBonus[8] = {S(-6, -6), S(-6, -6), S(-6, -6), S(-6, -6),
                                            S(-6, -6), S(-6, -6), S(-6, -6), S(-6, -6)};

    const Score backwardPawnRankBonus[8] = {S(-15, -15), S(-15, -15), S(-15, -15), S(-15, -15),
                                            S(-15, -15), S(-15, -15), S(-15, -15), S(-15, -15)};

    const Score KnightMobilityScore[9] = {S(-60, -80), S(-50, -30), S(-10, -20), S(-5, 10), S(5, 10),
                                          S(15, 14),   S(21, 15),   S(30, 21),   S(40, 30)};

    const Score BishopMobilityScore[14] = {S(-50, -60), S(-20, -25), S(15, -10), S(30, 12), S(40, 21),
                                           S(55, 49),   S(55, 55),   S(60, 58),  S(62, 65), S(70, 72),
                                           S(80, 78),   S(83, 87),   S(91, 88),  S(96, 98)};

    const Score RookMobilityScore[15] = {S(-60, -80), S(-25, -15), S(0, 20),   S(3, 40),   S(4, 70),
                                         S(15, 100),  S(20, 102),  S(30, 122), S(40, 133), S(40, 139),
                                         S(40, 153),  S(45, 160),  S(60, 165), S(61, 170), S(70, 175)};

    const Score QueenMobilityScore[28] = {S(-30, -50), S(-15, -30), S(-10, -10), S(-10, 20),  S(20, 40),   S(25, 55),
                                          S(23, 60),   S(35, 73),   S(40, 76),   S(55, 95),   S(65, 95),   S(68, 101),
                                          S(69, 124),  S(70, 128),  S(70, 132),  S(70, 133),  S(71, 136),  S(72, 140),
                                          S(74, 147),  S(76, 149),  S(90, 153),  S(104, 169), S(105, 171), S(106, 171),
                                          S(112, 178), S(114, 185), S(114, 187), S(119, 221)};
};

struct TracePeek {
    TracePeek(Trace &ts, EvalWeights &ws) : t(ts), w(ws){};

  public:
    void print();
    int calculate(std::tuple<int, int, int> phase);

  private:
    Trace &t;
    EvalWeights &w;
};

extern const EvalWeights evalWeights;
static Trace tempTrace;

enum Tracing : bool { NO_TRACE, TRACE };

template <Tracing T = NO_TRACE> class Eval {
  public:
    Eval(Board &b) : board(b), trace(tempTrace) { init(); };
    Eval(Board &b, Trace &t) : board(b), trace(t) { init(); }

    int eval() {
        const auto whitePawnCount   = popcount(board.pieces(PAWN, WHITE));
        const auto whiteKnightCount = popcount(board.pieces(KNIGHT, WHITE));
        const auto whiteBishopCount = popcount(board.pieces(BISHOP, WHITE));
        const auto whiteRookCount   = popcount(board.pieces(ROOK, WHITE));
        const auto whiteQueenCount  = popcount(board.pieces(QUEEN, WHITE));

        const auto blackPawnCount   = popcount(board.pieces(PAWN, BLACK));
        const auto blackKnightCount = popcount(board.pieces(KNIGHT, BLACK));
        const auto blackBishopCount = popcount(board.pieces(BISHOP, BLACK));
        const auto blackRookCount   = popcount(board.pieces(ROOK, BLACK));
        const auto blackQueenCount  = popcount(board.pieces(QUEEN, BLACK));

        const auto pawnVal   = (MgScore(pawnScore) * mgPhase + EgScore(pawnScore) * egPhase) / 24;
        const auto knightVal = (MgScore(knightScore) * mgPhase + EgScore(knightScore) * egPhase) / 24;
        const auto bishopVal = (MgScore(bishopScore) * mgPhase + EgScore(bishopScore) * egPhase) / 24;
        const auto rookVal   = (MgScore(rookScore) * mgPhase + EgScore(bishopScore) * egPhase) / 24;
        const auto queenVal  = (MgScore(queenScore) * mgPhase + EgScore(queenScore) * egPhase) / 24;

        const auto wMaterial = (pawnVal * whitePawnCount) + (knightVal * whiteKnightCount) +
                               (bishopVal * whiteBishopCount) + (rookVal * whiteRookCount) +
                               (queenVal * whiteQueenCount);
        const auto bMaterial = (pawnVal * blackPawnCount) + (knightVal * blackKnightCount) +
                               (bishopVal * blackBishopCount) + (rookVal * blackRookCount) +
                               (queenVal * blackQueenCount);

        if (T) {
            trace.pawnScore[WHITE] = whitePawnCount;
            trace.pawnScore[BLACK] = blackPawnCount;

            trace.knightScore[WHITE] = whiteKnightCount;
            trace.knightScore[BLACK] = blackKnightCount;

            trace.bishopScore[WHITE] = whiteBishopCount;
            trace.bishopScore[BLACK] = blackBishopCount;

            trace.rookScore[WHITE] = whiteRookCount;
            trace.rookScore[BLACK] = blackRookCount;

            trace.queenScore[WHITE] = whiteQueenCount;
            trace.queenScore[BLACK] = blackQueenCount;
        }

        const Score wPcSq = pieceSquare<WHITE>();
        const Score bPcSq = pieceSquare<BLACK>();
        const int mgPcSq  = MgScore(wPcSq) - MgScore(bPcSq);
        const int egPcSq  = EgScore(wPcSq) - EgScore(bPcSq);

        const Score wPassedPawn = passedPawnScore<WHITE>();
        const Score bPassedPawn = passedPawnScore<BLACK>();
        const int mgPassedPawn  = MgScore(wPassedPawn) - MgScore(bPassedPawn);
        const int egPassedPawn  = EgScore(wPassedPawn) - EgScore(bPassedPawn);

        const Score wDoubledPawn = doubledPawnPenalty<WHITE>();
        const Score bDoubledPawn = doubledPawnPenalty<BLACK>();
        const int mgDoubledPawn  = MgScore(wDoubledPawn) - MgScore(bDoubledPawn);
        const int egDoubledPawn  = EgScore(wDoubledPawn) - EgScore(bDoubledPawn);

        const Score wIsolatedPawn = isolatedPawnPenalty<WHITE>();
        const Score bIsolatedPawn = isolatedPawnPenalty<BLACK>();
        const int mgIsolatedPawn  = MgScore(wIsolatedPawn) - MgScore(bIsolatedPawn);
        const int egIsolatedPawn  = EgScore(wIsolatedPawn) - EgScore(bIsolatedPawn);

        const Score wBackwardPawn = backwardPawnScore<WHITE>();
        const Score bBackwardPawn = backwardPawnScore<BLACK>();
        const int mgBackwardPawn  = MgScore(wBackwardPawn) - MgScore(bBackwardPawn);
        const int egBackwardPawn  = EgScore(wBackwardPawn) - EgScore(bBackwardPawn);

        const Score wMobility = mobilityScore<WHITE>();
        const Score bMobility = mobilityScore<BLACK>();
        const int mgMobility  = MgScore(wMobility) - MgScore(bMobility);
        const int egMobility  = EgScore(wMobility) - EgScore(bMobility);

        const auto color           = (board.turn == WHITE) ? 1 : -1;
        const auto materialScore   = wMaterial - bMaterial;
        const int pcSqEval         = (mgPcSq * mgPhase + egPcSq * egPhase) / 24;
        const int passedPawnEval   = (mgPassedPawn * mgPhase + egPassedPawn * egPhase) / 24;
        const int doubledPawnEval  = (mgDoubledPawn * mgPhase + egDoubledPawn * egPhase) / 24;
        const int isolatedPawnEval = (mgIsolatedPawn * mgPhase + egIsolatedPawn * egPhase) / 24;
        const int backwardPawnEval = (mgBackwardPawn * mgPhase + egBackwardPawn * egPhase) / 24;
        const int mobilityEval     = (mgMobility * mgPhase + egMobility * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + passedPawnEval + doubledPawnEval + isolatedPawnEval + backwardPawnEval +
                mobilityEval;

        return eval * color;
    }

  private:
    Board &board;
    Trace &trace;

  private:
    int phase;
    int mgPhase;
    int egPhase;

  private:
    template <Color C> constexpr Bitboard doubledPawns();
    template <Color C> constexpr Bitboard backwardPawns();

    template <Color C> constexpr Score isolatedPawnPenalty();
    template <Color C> constexpr Score backwardPawnScore();
    template <Color C> constexpr Score passedPawnScore();
    template <Color C> constexpr Score doubledPawnPenalty();
    template <Color C> constexpr Score pieceSquare();
    template <Color C> constexpr Score mobilityScore();

  private:
    void init() {
        phase   = 0;
        mgPhase = 0;
        egPhase = 0;

        // clang-format off
        phase = 4 * popcount(board.pieces(QUEEN)) +
                2 * popcount(board.pieces(ROOK)) +
                1 * popcount(board.pieces(BISHOP)) +
                1 * popcount(board.pieces(KNIGHT));
        // clang-format on

        mgPhase = phase;
        if (mgPhase > 24)
            mgPhase = 24;
        egPhase = 24 - mgPhase;
    }
};

template <Tracing T> template <Color C> constexpr Bitboard Eval<T>::doubledPawns() {
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

template <Tracing T> template <Color C> constexpr Bitboard Eval<T>::backwardPawns() {
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

template <Tracing T> template <Color C> constexpr Score Eval<T>::backwardPawnScore() {
    Bitboard bckPawns = backwardPawns<C>();

    int mgScore = 0, egScore = 0;
    while (bckPawns) {
        Square psq = Square(lsb_index(bckPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        mgScore += MgScore(backwardPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(backwardPawnRankBonus[RANK_OF(psq)]);
        mgScore += MgScore(t_backwardPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(t_backwardPawnRankBonus[RANK_OF(psq)]);

        if (T) {
            trace.backwardPawns[RANK_OF(psq)][C]++;
        }

        bckPawns &= bckPawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::isolatedPawnPenalty() {
    Bitboard pawns = board.pieces(PAWN, C);

    int mgScore = 0, egScore = 0;
    while (pawns) {
        Square psq = Square(lsb_index(pawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        if (!(isolatedPawnMasks[psq] & board.pieces(PAWN, C))) {

            mgScore += MgScore(isolatedPawnRankBonus[RANK_OF(psq)]);
            egScore += EgScore(isolatedPawnRankBonus[RANK_OF(psq)]);
            mgScore += MgScore(t_isolatedPawnRankBonus[RANK_OF(psq)]);
            egScore += EgScore(t_isolatedPawnRankBonus[RANK_OF(psq)]);

            if (T) {
                trace.isolatedPawns[RANK_OF(psq)][C]++;
            }
        }

        pawns &= pawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::passedPawnScore() {
    constexpr Direction Up   = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);
    Bitboard ourPawns        = board.pieces(PAWN, C);
    Bitboard enemyPawns      = board.pieces(PAWN, ~C);

    Bitboard opponentPawnSpan = fill<Down>(shift<Down>(enemyPawns));
    opponentPawnSpan |= shift<WEST>(opponentPawnSpan) | shift<EAST>(opponentPawnSpan);

    Bitboard passedPawns = board.pieces(PAWN, C) & ~opponentPawnSpan;

    Rank startRank       = (C == WHITE) ? RANK_2 : RANK_7;
    Bitboard startPassed = passedPawns & startRank;

    int mgScore = 0;
    int egScore = 0;

    while (passedPawns) {
        Square psq = Square(lsb_index(passedPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        mgScore += MgScore(passedPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(passedPawnRankBonus[RANK_OF(psq)]);
        mgScore += MgScore(t_passedPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(t_passedPawnRankBonus[RANK_OF(psq)]);

        if (T) {
            trace.passedPawn[RANK_OF(psq)][C]++;
        }

        passedPawns &= passedPawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::doubledPawnPenalty() {
    Bitboard dblPawns = doubledPawns<C>();

    int mgScore = 0, egScore = 0;
    while (dblPawns) {
        Square psq = Square(lsb_index(dblPawns));
        psq        = (C == WHITE) ? psq : Square(mirror(psq));

        mgScore += MgScore(doubledPawnRankPenalty[RANK_OF(psq)]);
        egScore += EgScore(doubledPawnRankPenalty[RANK_OF(psq)]);
        mgScore += MgScore(t_doubledPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(t_doubledPawnRankBonus[RANK_OF(psq)]);

        if (T) {
            trace.doubledPawns[RANK_OF(psq)][C]++;
        }

        dblPawns &= dblPawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::pieceSquare() {
    int mgScore = 0;
    int egScore = 0;

    for (int i = 0; i < 64; i++) {
        Piece p   = board.board[i];
        PieceT pt = getPcType(p);

        if (C == WHITE) {
            if (p < B_PAWN && p != NO_PC) {
                if (T) {

                    auto pcSq = {&trace.pawnPcSq, &trace.knightPcSq, &trace.bishopPcSq,
                                 &trace.rookPcSq, &trace.queenPcSq,  &trace.kingPcSq};

                    (*std::data(pcSq))[pt - 1][i][WHITE] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][i]) + MgScore(t_pcSq[pt - 1][i]);
                egScore += EgScore(pcSq[pt - 1][i]) + EgScore(t_pcSq[pt - 1][i]);
            }
        } else if (C == BLACK) {
            if (p >= B_PAWN && p != NO_PC) {
                if (T) {
                    auto pcSq = {&trace.pawnPcSq, &trace.knightPcSq, &trace.bishopPcSq,
                                 &trace.rookPcSq, &trace.queenPcSq,  &trace.kingPcSq};

                    (*std::data(pcSq))[pt - 1][mirror(i)][BLACK] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][mirror(i)]) + MgScore(t_pcSq[pt - 1][mirror(i)]);
                egScore += EgScore(pcSq[pt - 1][mirror(i)]) + EgScore(t_pcSq[pt - 1][mirror(i)]);
            }
        }
    }

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::mobilityScore() {
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

    int mgScore = 0;
    int egScore = 0;

    while (knights) {
        Square knightSq      = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.knightMobility[numMoves][C]++;
        }

        mgScore += MgScore(KnightMobilityScore[numMoves]) + MgScore(t_KnightMobilityScore[numMoves]);
        egScore += EgScore(KnightMobilityScore[numMoves]) + EgScore(t_KnightMobilityScore[numMoves]);

        knights &= knights - 1;
    }

    while (bishops) {
        Square bishopSq      = Square(lsb_index(bishops));
        Bitboard bishopMoves = getBishopAttacks(bishopSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(bishopMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.bishopMobility[numMoves][C]++;
        }

        mgScore += MgScore(BishopMobilityScore[numMoves]);
        egScore += EgScore(BishopMobilityScore[numMoves]);
        mgScore += MgScore(t_BishopMobilityScore[numMoves]);
        egScore += EgScore(t_BishopMobilityScore[numMoves]);

        bishops &= bishops - 1;
    }

    while (rooks) {
        Square rookSq      = Square(lsb_index(rooks));
        Bitboard rookMoves = getRookAttacks(rookSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(rookMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.rookMobility[numMoves][C]++;
        }

        mgScore += MgScore(RookMobilityScore[numMoves]);
        egScore += EgScore(RookMobilityScore[numMoves]);
        mgScore += MgScore(t_RookMobilityScore[numMoves]);
        egScore += EgScore(t_RookMobilityScore[numMoves]);

        rooks &= rooks - 1;
    }

    while (queens) {
        Square queenSq      = Square(lsb_index(queens));
        Bitboard queenMoves = getRookAttacks(queenSq, board.pieces()) | getBishopAttacks(queenSq, board.pieces());
        queenMoves &= ~excludedSquares;

        int numMoves = popcount(queenMoves) - 1;
        if (numMoves < 0)
            numMoves = 0;

        if (T) {
            trace.queenMobility[numMoves][C]++;
        }

        mgScore += MgScore(QueenMobilityScore[numMoves]);
        egScore += EgScore(QueenMobilityScore[numMoves]);
        mgScore += MgScore(t_QueenMobilityScore[numMoves]);
        egScore += EgScore(t_QueenMobilityScore[numMoves]);

        queens &= queens - 1;
    }

    return S(mgScore, egScore);
}

} // namespace Yayo
#endif // SEARCH_H_
