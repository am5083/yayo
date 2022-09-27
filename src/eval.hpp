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
#include "util.hpp"
#include <thread>

namespace Yayo {
namespace {
constexpr int TEMPO = 10;
} // namespace

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

constexpr Score blockedPassedPawnRankBonus[] = {S(0, 0),     S(0, 0),     S(0, 0),     S(40, 40),
                                                S(200, 200), S(260, 260), S(400, 400), S(0, 0)};

constexpr Score pawnScore   = S(91, 118);
constexpr Score knightScore = S(368, 392);
constexpr Score bishopScore = S(382, 386);
constexpr Score rookScore   = S(517, 639);
constexpr Score queenScore  = S(1093, 1221);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),   S(0, 0),    S(0, 0),   S(0, 0),     S(0, 0),
    S(25, 123), S(25, 93),  S(-19, 103), S(10, 52), S(-19, 60), S(25, 44), S(-85, 100), S(-99, 134),
    S(-12, 98), S(-7, 101), S(9, 55),    S(0, 10),  S(25, 2),   S(75, 26), S(48, 72),   S(10, 80),
    S(-18, 56), S(-12, 38), S(0, 17),    S(6, -9),  S(28, -5),  S(27, 4),  S(14, 22),   S(4, 25),
    S(-35, 31), S(-23, 22), S(-10, 5),   S(2, -7),  S(1, -4),   S(4, 4),   S(-2, 6),    S(-15, 6),
    S(-26, 22), S(-15, 15), S(-6, 8),    S(-3, 8),  S(16, 10),  S(8, 10),  S(26, 4),    S(-2, 2),
    S(-21, 25), S(-12, 22), S(-4, 10),   S(-2, 16), S(7, 19),   S(34, 10), S(42, 1),    S(-6, 4),
    S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),   S(0, 0),    S(0, 0),   S(0, 0),     S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
    S(-154, -45), S(-98, -35), S(-36, -13), S(-31, -15), S(53, -19), S(-82, -20), S(-26, -47), S(-94, -94),
    S(-39, -7),   S(-9, 1),    S(54, -23),  S(47, -3),   S(46, -10), S(83, -18),  S(17, -19),  S(27, -34),
    S(-26, 7),    S(43, -15),  S(54, 12),   S(77, 9),    S(97, 5),   S(124, -13), S(72, -21),  S(48, -33),
    S(0, 0),      S(8, 13),    S(32, 36),   S(67, 31),   S(45, 31),  S(70, 24),   S(27, 8),    S(54, -4),
    S(-7, 10),    S(6, 8),     S(16, 34),   S(28, 27),   S(36, 35),  S(32, 21),   S(48, 4),    S(20, 3),
    S(-23, -19),  S(-19, -3),  S(2, 1),     S(9, 21),    S(27, 20),  S(12, -4),   S(16, -15),  S(-3, -11),
    S(-37, -16),  S(-28, 0),   S(-15, -8),  S(1, -4),    S(8, -4),   S(9, -9),    S(-3, -21),  S(-7, -17),
    S(-91, -21),  S(-19, -22), S(-40, -9),  S(-12, -1),  S(-7, -12), S(-7, -11),  S(-14, -11), S(-28, -29),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
    S(-15, 3),  S(-19, -4), S(-68, 7),  S(-55, 0),  S(-34, -6), S(-47, -2), S(-3, -4), S(-15, -16),
    S(-13, -8), S(10, -10), S(-16, -3), S(-11, -1), S(19, -13), S(32, -16), S(33, -4), S(-16, -9),
    S(-11, 12), S(24, -2),  S(31, 0),   S(37, -2),  S(35, -3),  S(57, 3),   S(50, -2), S(29, 4),
    S(-15, -3), S(5, 5),    S(14, 12),  S(50, 14),  S(35, 15),  S(38, 8),   S(8, 11),  S(-2, 2),
    S(-9, -5),  S(-4, 11),  S(6, 15),   S(33, 11),  S(32, 15),  S(4, 14),   S(2, 6),   S(24, -15),
    S(1, 6),    S(21, 3),   S(14, 13),  S(15, 13),  S(22, 18),  S(17, 10),  S(18, -2), S(23, -8),
    S(14, -1),  S(12, -5),  S(26, -8),  S(0, 8),    S(10, 4),   S(27, -2),  S(34, -4), S(17, -5),
    S(-8, -3),  S(28, 12),  S(9, 11),   S(-7, 5),   S(-3, 4),   S(2, 12),   S(-1, 0),  S(4, -7),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
    S(27, 14),  S(44, 11),  S(20, 30),  S(30, 17),  S(57, 11), S(23, 21), S(39, 16),  S(58, 9),
    S(12, 16),  S(14, 25),  S(36, 28),  S(62, 11),  S(52, 6),  S(72, 6),  S(37, 8),   S(60, 4),
    S(-1, 19),  S(23, 22),  S(24, 16),  S(38, 15),  S(46, 10), S(66, -3), S(91, 0),   S(50, 5),
    S(-18, 22), S(7, 17),   S(4, 29),   S(18, 13),  S(22, 2),  S(29, -1), S(25, 10),  S(14, 2),
    S(-36, 13), S(-26, 16), S(-17, 20), S(-12, 14), S(-5, 9),  S(-22, 9), S(12, 3),   S(-10, 7),
    S(-36, 11), S(-26, 12), S(-19, 5),  S(-17, 9),  S(-9, 6),  S(-3, -7), S(26, -16), S(-11, -13),
    S(-46, 3),  S(-25, 0),  S(-11, 7),  S(-8, 4),   S(-1, -8), S(5, -13), S(22, -15), S(-42, 2),
    S(-21, 6),  S(-16, 5),  S(-3, 5),   S(8, 2),    S(12, -8), S(1, -4),  S(2, -4),   S(-23, -4),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
    S(-32, 24), S(-15, 20),  S(15, 40),  S(17, 51),  S(43, 39),  S(43, 31),  S(62, 14),   S(47, 29),
    S(-14, 17), S(-46, 35),  S(-14, 54), S(-13, 66), S(-12, 90), S(41, 31),  S(23, 34),   S(75, 29),
    S(-10, 17), S(-21, 24),  S(-3, 39),  S(-6, 71),  S(24, 67),  S(68, 51),  S(72, 22),   S(70, 43),
    S(-24, 25), S(-25, 33),  S(-19, 40), S(-18, 61), S(-3, 72),  S(15, 67),  S(15, 74),   S(21, 47),
    S(-15, 2),  S(-27, 37),  S(-25, 35), S(-20, 63), S(-13, 58), S(-10, 59), S(-1, 47),   S(14, 52),
    S(-14, -1), S(-7, 5),    S(-14, 24), S(-14, 29), S(-10, 45), S(5, 25),   S(15, 15),   S(10, 25),
    S(-14, 2),  S(-13, -15), S(2, -15),  S(14, -11), S(8, 0),    S(16, -22), S(13, -21),  S(19, -32),
    S(-6, -18), S(-5, -18),  S(5, -23),  S(16, -12), S(3, -7),   S(-8, -10), S(-12, -15), S(-20, -12),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
    S(-71, -92), S(12, -55), S(11, -30), S(-20, -13), S(-60, -17), S(-32, 5),   S(13, 8),   S(8, -57),
    S(3, -29),   S(-9, 11),  S(-46, 15), S(-6, 19),   S(-3, 24),   S(-2, 38),   S(0, 36),   S(-5, 8),
    S(-42, -5),  S(14, 11),  S(-13, 35), S(-15, 47),  S(-16, 58),  S(23, 63),   S(37, 48),  S(-13, 10),
    S(-28, -16), S(-26, 24), S(-35, 39), S(-73, 52),  S(-65, 55),  S(-50, 55),  S(-41, 40), S(-56, 3),
    S(-69, -23), S(-25, 1),  S(-72, 35), S(-89, 53),  S(-107, 56), S(-77, 42),  S(-75, 21), S(-95, -1),
    S(-35, -32), S(-30, -6), S(-64, 14), S(-102, 35), S(-99, 37),  S(-92, 27),  S(-53, 6),  S(-74, -10),
    S(27, -37),  S(-8, -16), S(-25, -3), S(-67, 9),   S(-70, 14),  S(-51, 8),   S(6, -10),  S(16, -32),
    S(21, -75),  S(64, -60), S(38, -41), S(-62, -24), S(3, -36),   S(-40, -26), S(43, -54), S(36, -89),
};
constexpr Score passedPawnRankBonus[8] = {
    S(0, 0), S(-6, 4), S(-5, 14), S(-4, 43), S(7, 76), S(41, 126), S(132, 194), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
    S(-10, -10), S(-7, -26), S(-8, -17), S(-2, -29), S(-10, -41), S(-44, -40), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
    S(-6, -6), S(-14, -6), S(-19, -15), S(-14, -13), S(-4, -24), S(0, -24), S(-84, -63), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
    S(-15, -15), S(-16, -13), S(-4, -6), S(-9, -3), S(-15, -8), S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
    S(-35, -27), S(-18, 11), S(-6, 20), S(3, 27), S(12, 40), S(23, 39), S(33, 36), S(39, 23), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
    S(-10, -21), S(13, 3),  S(22, 26), S(35, 39), S(43, 53), S(50, 60), S(55, 64),
    S(59, 70),   S(62, 67), S(66, 69), S(74, 64), S(63, 77), S(81, 59), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
    S(-21, 40), S(-13, 67), S(-6, 87),  S(-1, 99),  S(0, 104),  S(7, 120),  S(16, 121), S(22, 129),
    S(35, 134), S(40, 138), S(46, 146), S(47, 155), S(50, 152), S(38, 158), S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
    S(-52, -71), S(-16, -32), S(3, -5),    S(11, 36),   S(20, 59),   S(28, 62),   S(33, 93),
    S(41, 103),  S(48, 103),  S(53, 126),  S(60, 124),  S(64, 134),  S(66, 145),  S(64, 153),
    S(72, 161),  S(69, 175),  S(71, 182),  S(74, 186),  S(84, 195),  S(84, 199),  S(109, 188),
    S(122, 194), S(112, 190), S(129, 193), S(119, 187), S(116, 184), S(117, 198), S(119, 221),
};

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
    const Score pawnScore   = S(91, 118);
    const Score knightScore = S(368, 392);
    const Score bishopScore = S(382, 386);
    const Score rookScore   = S(517, 639);
    const Score queenScore  = S(1093, 1221);

    const Score taperedPawnPcSq[SQUARE_CT] = {
        S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),   S(0, 0),    S(0, 0),   S(0, 0),     S(0, 0),
        S(25, 123), S(25, 93),  S(-19, 103), S(10, 52), S(-19, 60), S(25, 44), S(-85, 100), S(-99, 134),
        S(-12, 98), S(-7, 101), S(9, 55),    S(0, 10),  S(25, 2),   S(75, 26), S(48, 72),   S(10, 80),
        S(-18, 56), S(-12, 38), S(0, 17),    S(6, -9),  S(28, -5),  S(27, 4),  S(14, 22),   S(4, 25),
        S(-35, 31), S(-23, 22), S(-10, 5),   S(2, -7),  S(1, -4),   S(4, 4),   S(-2, 6),    S(-15, 6),
        S(-26, 22), S(-15, 15), S(-6, 8),    S(-3, 8),  S(16, 10),  S(8, 10),  S(26, 4),    S(-2, 2),
        S(-21, 25), S(-12, 22), S(-4, 10),   S(-2, 16), S(7, 19),   S(34, 10), S(42, 1),    S(-6, 4),
        S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),   S(0, 0),    S(0, 0),   S(0, 0),     S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
        S(-154, -45), S(-98, -35), S(-36, -13), S(-31, -15), S(53, -19), S(-82, -20), S(-26, -47), S(-94, -94),
        S(-39, -7),   S(-9, 1),    S(54, -23),  S(47, -3),   S(46, -10), S(83, -18),  S(17, -19),  S(27, -34),
        S(-26, 7),    S(43, -15),  S(54, 12),   S(77, 9),    S(97, 5),   S(124, -13), S(72, -21),  S(48, -33),
        S(0, 0),      S(8, 13),    S(32, 36),   S(67, 31),   S(45, 31),  S(70, 24),   S(27, 8),    S(54, -4),
        S(-7, 10),    S(6, 8),     S(16, 34),   S(28, 27),   S(36, 35),  S(32, 21),   S(48, 4),    S(20, 3),
        S(-23, -19),  S(-19, -3),  S(2, 1),     S(9, 21),    S(27, 20),  S(12, -4),   S(16, -15),  S(-3, -11),
        S(-37, -16),  S(-28, 0),   S(-15, -8),  S(1, -4),    S(8, -4),   S(9, -9),    S(-3, -21),  S(-7, -17),
        S(-91, -21),  S(-19, -22), S(-40, -9),  S(-12, -1),  S(-7, -12), S(-7, -11),  S(-14, -11), S(-28, -29),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
        S(-15, 3),  S(-19, -4), S(-68, 7),  S(-55, 0),  S(-34, -6), S(-47, -2), S(-3, -4), S(-15, -16),
        S(-13, -8), S(10, -10), S(-16, -3), S(-11, -1), S(19, -13), S(32, -16), S(33, -4), S(-16, -9),
        S(-11, 12), S(24, -2),  S(31, 0),   S(37, -2),  S(35, -3),  S(57, 3),   S(50, -2), S(29, 4),
        S(-15, -3), S(5, 5),    S(14, 12),  S(50, 14),  S(35, 15),  S(38, 8),   S(8, 11),  S(-2, 2),
        S(-9, -5),  S(-4, 11),  S(6, 15),   S(33, 11),  S(32, 15),  S(4, 14),   S(2, 6),   S(24, -15),
        S(1, 6),    S(21, 3),   S(14, 13),  S(15, 13),  S(22, 18),  S(17, 10),  S(18, -2), S(23, -8),
        S(14, -1),  S(12, -5),  S(26, -8),  S(0, 8),    S(10, 4),   S(27, -2),  S(34, -4), S(17, -5),
        S(-8, -3),  S(28, 12),  S(9, 11),   S(-7, 5),   S(-3, 4),   S(2, 12),   S(-1, 0),  S(4, -7),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
        S(27, 14),  S(44, 11),  S(20, 30),  S(30, 17),  S(57, 11), S(23, 21), S(39, 16),  S(58, 9),
        S(12, 16),  S(14, 25),  S(36, 28),  S(62, 11),  S(52, 6),  S(72, 6),  S(37, 8),   S(60, 4),
        S(-1, 19),  S(23, 22),  S(24, 16),  S(38, 15),  S(46, 10), S(66, -3), S(91, 0),   S(50, 5),
        S(-18, 22), S(7, 17),   S(4, 29),   S(18, 13),  S(22, 2),  S(29, -1), S(25, 10),  S(14, 2),
        S(-36, 13), S(-26, 16), S(-17, 20), S(-12, 14), S(-5, 9),  S(-22, 9), S(12, 3),   S(-10, 7),
        S(-36, 11), S(-26, 12), S(-19, 5),  S(-17, 9),  S(-9, 6),  S(-3, -7), S(26, -16), S(-11, -13),
        S(-46, 3),  S(-25, 0),  S(-11, 7),  S(-8, 4),   S(-1, -8), S(5, -13), S(22, -15), S(-42, 2),
        S(-21, 6),  S(-16, 5),  S(-3, 5),   S(8, 2),    S(12, -8), S(1, -4),  S(2, -4),   S(-23, -4),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
        S(-32, 24), S(-15, 20),  S(15, 40),  S(17, 51),  S(43, 39),  S(43, 31),  S(62, 14),   S(47, 29),
        S(-14, 17), S(-46, 35),  S(-14, 54), S(-13, 66), S(-12, 90), S(41, 31),  S(23, 34),   S(75, 29),
        S(-10, 17), S(-21, 24),  S(-3, 39),  S(-6, 71),  S(24, 67),  S(68, 51),  S(72, 22),   S(70, 43),
        S(-24, 25), S(-25, 33),  S(-19, 40), S(-18, 61), S(-3, 72),  S(15, 67),  S(15, 74),   S(21, 47),
        S(-15, 2),  S(-27, 37),  S(-25, 35), S(-20, 63), S(-13, 58), S(-10, 59), S(-1, 47),   S(14, 52),
        S(-14, -1), S(-7, 5),    S(-14, 24), S(-14, 29), S(-10, 45), S(5, 25),   S(15, 15),   S(10, 25),
        S(-14, 2),  S(-13, -15), S(2, -15),  S(14, -11), S(8, 0),    S(16, -22), S(13, -21),  S(19, -32),
        S(-6, -18), S(-5, -18),  S(5, -23),  S(16, -12), S(3, -7),   S(-8, -10), S(-12, -15), S(-20, -12),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
        S(-71, -92), S(12, -55), S(11, -30), S(-20, -13), S(-60, -17), S(-32, 5),   S(13, 8),   S(8, -57),
        S(3, -29),   S(-9, 11),  S(-46, 15), S(-6, 19),   S(-3, 24),   S(-2, 38),   S(0, 36),   S(-5, 8),
        S(-42, -5),  S(14, 11),  S(-13, 35), S(-15, 47),  S(-16, 58),  S(23, 63),   S(37, 48),  S(-13, 10),
        S(-28, -16), S(-26, 24), S(-35, 39), S(-73, 52),  S(-65, 55),  S(-50, 55),  S(-41, 40), S(-56, 3),
        S(-69, -23), S(-25, 1),  S(-72, 35), S(-89, 53),  S(-107, 56), S(-77, 42),  S(-75, 21), S(-95, -1),
        S(-35, -32), S(-30, -6), S(-64, 14), S(-102, 35), S(-99, 37),  S(-92, 27),  S(-53, 6),  S(-74, -10),
        S(27, -37),  S(-8, -16), S(-25, -3), S(-67, 9),   S(-70, 14),  S(-51, 8),   S(6, -10),  S(16, -32),
        S(21, -75),  S(64, -60), S(38, -41), S(-62, -24), S(3, -36),   S(-40, -26), S(43, -54), S(36, -89),
    };
    const Score passedPawnRankBonus[8] = {
        S(0, 0), S(-6, 4), S(-5, 14), S(-4, 43), S(7, 76), S(41, 126), S(132, 194), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
        S(-10, -10), S(-7, -26), S(-8, -17), S(-2, -29), S(-10, -41), S(-44, -40), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
        S(-6, -6), S(-14, -6), S(-19, -15), S(-14, -13), S(-4, -24), S(0, -24), S(-84, -63), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
        S(-15, -15), S(-16, -13), S(-4, -6), S(-9, -3), S(-15, -8), S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
        S(-35, -27), S(-18, 11), S(-6, 20), S(3, 27), S(12, 40), S(23, 39), S(33, 36), S(39, 23), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
        S(-10, -21), S(13, 3),  S(22, 26), S(35, 39), S(43, 53), S(50, 60), S(55, 64),
        S(59, 70),   S(62, 67), S(66, 69), S(74, 64), S(63, 77), S(81, 59), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
        S(-21, 40), S(-13, 67), S(-6, 87),  S(-1, 99),  S(0, 104),  S(7, 120),  S(16, 121), S(22, 129),
        S(35, 134), S(40, 138), S(46, 146), S(47, 155), S(50, 152), S(38, 158), S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
        S(-52, -71), S(-16, -32), S(3, -5),    S(11, 36),   S(20, 59),   S(28, 62),   S(33, 93),
        S(41, 103),  S(48, 103),  S(53, 126),  S(60, 124),  S(64, 134),  S(66, 145),  S(64, 153),
        S(72, 161),  S(69, 175),  S(71, 182),  S(74, 186),  S(84, 195),  S(84, 199),  S(109, 188),
        S(122, 194), S(112, 190), S(129, 193), S(119, 187), S(116, 184), S(117, 198), S(119, 221),
    };
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

        mgScore += MgScore(doubledPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(doubledPawnRankBonus[RANK_OF(psq)]);

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

                mgScore += MgScore(pcSq[pt - 1][i]);
                egScore += EgScore(pcSq[pt - 1][i]);
            }
        } else if (C == BLACK) {
            if (p >= B_PAWN && p != NO_PC) {
                if (T) {
                    auto pcSq = {&trace.pawnPcSq, &trace.knightPcSq, &trace.bishopPcSq,
                                 &trace.rookPcSq, &trace.queenPcSq,  &trace.kingPcSq};

                    (*std::data(pcSq))[pt - 1][mirror(i)][BLACK] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][mirror(i)]);
                egScore += EgScore(pcSq[pt - 1][mirror(i)]);
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

        mgScore += MgScore(KnightMobilityScore[numMoves]);
        egScore += EgScore(KnightMobilityScore[numMoves]);

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

        queens &= queens - 1;
    }

    return S(mgScore, egScore);
}

} // namespace Yayo
#endif // SEARCH_H_
