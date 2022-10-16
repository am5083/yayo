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

constexpr Score blockedPassedPawnRankBonus[] = {
      S(0, 0),     S(0, 0),     S(0, 0),     S(40, 40),
      S(200, 200), S(260, 260), S(400, 400), S(0, 0)};

constexpr Score pawnScore = S(94, 125);
constexpr Score knightScore = S(381, 418);
constexpr Score bishopScore = S(396, 414);
constexpr Score rookScore = S(532, 695);
constexpr Score queenScore = S(1132, 1341);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(18, 118), S(9, 95),
      S(-21, 106),  S(7, 46),   S(-29, 47), S(-1, 37),  S(-115, 106),
      S(-119, 138), S(-9, 101), S(-9, 104), S(17, 51),  S(18, 1),
      S(35, -5),    S(86, 21),  S(58, 71),  S(21, 78),  S(-20, 60),
      S(-16, 42),   S(-1, 19),  S(6, -10),  S(29, -6),  S(28, 4),
      S(14, 23),    S(9, 25),   S(-36, 33), S(-28, 25), S(-12, 7),
      S(2, -7),     S(1, -4),   S(5, 4),    S(-3, 7),   S(-10, 6),
      S(-26, 24),   S(-19, 19), S(-7, 10),  S(-3, 9),   S(17, 10),
      S(10, 11),    S(27, 6),   S(2, 1),    S(-22, 26), S(-17, 25),
      S(-4, 12),    S(-2, 15),  S(9, 17),   S(36, 10),  S(43, 2),
      S(-1, 2),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-159, -49), S(-111, -36), S(-40, -13), S(-20, -12), S(45, -11),
      S(-75, -18),  S(-41, -38),  S(-91, -99), S(-26, -8),  S(1, 0),
      S(47, -19),   S(54, -5),    S(57, -15),  S(93, -18),  S(17, -17),
      S(48, -32),   S(-18, 11),   S(37, -11),  S(58, 13),   S(79, 12),
      S(101, 7),    S(121, -11),  S(72, -22),  S(47, -30),  S(2, 6),
      S(9, 17),     S(33, 39),    S(68, 35),   S(45, 34),   S(69, 29),
      S(28, 10),    S(61, -2),    S(-7, 17),   S(5, 10),    S(16, 38),
      S(29, 29),    S(36, 40),    S(34, 23),   S(53, 4),    S(22, 8),
      S(-23, -16),  S(-19, -1),   S(1, 4),     S(9, 23),    S(28, 24),
      S(13, -3),    S(16, -13),   S(-1, -9),   S(-41, -6),  S(-23, 3),
      S(-14, -5),   S(2, -2),     S(9, -2),    S(8, -5),    S(1, -21),
      S(-6, -10),   S(-86, -20),  S(-18, -22), S(-36, -7),  S(-8, 1),
      S(-4, -10),   S(-4, -10),   S(-14, -6),  S(-33, -15),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-11, 4),  S(-41, 3),   S(-64, 9),  S(-72, 5),  S(-44, -5), S(-56, 2),
      S(-14, 1),  S(-24, -13), S(-8, -11), S(8, -10),  S(-14, -4), S(-14, 2),
      S(13, -12), S(19, -12),  S(36, -4),  S(-7, -12), S(-10, 14), S(22, 2),
      S(28, 4),   S(38, -1),   S(35, -1),  S(59, 6),   S(55, -2),  S(34, 2),
      S(-17, -2), S(7, 7),     S(13, 16),  S(50, 20),  S(34, 21),  S(38, 11),
      S(9, 14),   S(-3, 2),    S(-9, -3),  S(-7, 16),  S(6, 20),   S(35, 15),
      S(31, 21),  S(5, 18),    S(-2, 12),  S(28, -18), S(1, 11),   S(21, 6),
      S(14, 17),  S(15, 16),   S(24, 21),  S(18, 14),  S(19, 1),   S(25, -7),
      S(15, 3),   S(13, -3),   S(27, -7),  S(0, 10),   S(11, 6),   S(29, 0),
      S(35, -1),  S(20, 0),    S(-4, -2),  S(32, 15),  S(11, 11),  S(-4, 6),
      S(0, 5),    S(3, 14),    S(16, -3),  S(12, -7),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(24, 20),  S(47, 16),  S(10, 40),  S(20, 28),  S(56, 17),  S(33, 26),
      S(45, 21),  S(69, 13),  S(5, 22),   S(5, 32),   S(26, 37),  S(62, 15),
      S(39, 17),  S(73, 9),   S(42, 9),   S(67, 6),   S(-1, 22),  S(23, 26),
      S(21, 20),  S(37, 19),  S(54, 9),   S(76, -6),  S(110, -3), S(67, 2),
      S(-18, 25), S(9, 18),   S(2, 34),   S(13, 19),  S(20, 3),   S(27, 1),
      S(37, 7),   S(28, -2),  S(-36, 16), S(-30, 20), S(-20, 25), S(-17, 20),
      S(-11, 15), S(-26, 16), S(12, 6),   S(-8, 10),  S(-37, 13), S(-28, 16),
      S(-20, 8),  S(-18, 12), S(-11, 10), S(-3, -4),  S(34, -21), S(-6, -14),
      S(-48, 6),  S(-28, 4),  S(-10, 9),  S(-7, 6),   S(0, -6),   S(5, -10),
      S(30, -18), S(-33, -1), S(-21, 6),  S(-17, 7),  S(-3, 7),   S(7, 5),
      S(12, -5),  S(1, -3),   S(7, -6),   S(-19, -8),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-41, 36),  S(-20, 18),  S(4, 50),   S(17, 60),  S(34, 48),  S(41, 38),
      S(74, 10),   S(46, 31),   S(-16, 24), S(-49, 40), S(-21, 65), S(-26, 85),
      S(-16, 105), S(33, 39),   S(18, 36),  S(76, 36),  S(-12, 26), S(-26, 32),
      S(-9, 54),   S(-12, 85),  S(21, 79),  S(72, 54),  S(83, 16),  S(71, 55),
      S(-25, 32),  S(-24, 35),  S(-21, 46), S(-21, 69), S(-4, 80),  S(11, 79),
      S(19, 77),   S(23, 46),   S(-15, 8),  S(-27, 42), S(-28, 47), S(-21, 73),
      S(-17, 73),  S(-13, 73),  S(-2, 53),  S(15, 61),  S(-13, 3),  S(-8, 17),
      S(-13, 29),  S(-15, 39),  S(-12, 59), S(7, 29),   S(16, 19),  S(11, 32),
      S(-12, 7),   S(-14, -10), S(3, -9),   S(15, -9),  S(8, 5),    S(19, -21),
      S(17, -19),  S(30, -39),  S(-7, -11), S(-1, -20), S(10, -29), S(19, -11),
      S(9, -14),   S(-5, -6),   S(-1, -20), S(-7, -4),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-71, -105), S(8, -65),   S(12, -35),  S(-26, -10), S(-63, -21),
      S(-26, -1),   S(18, 9),    S(6, -87),   S(-19, -34), S(-14, 10),
      S(-70, 19),   S(-4, 21),   S(0, 26),    S(-1, 41),   S(26, 37),
      S(9, 3),      S(-70, -7),  S(6, 11),    S(-30, 40),  S(-23, 53),
      S(-23, 65),   S(30, 67),   S(48, 49),   S(-6, 9),    S(-39, -18),
      S(-33, 25),   S(-62, 44),  S(-118, 60), S(-103, 63), S(-79, 62),
      S(-70, 47),   S(-77, 5),   S(-87, -23), S(-47, 4),   S(-114, 42),
      S(-134, 62),  S(-158, 66), S(-109, 48), S(-110, 28), S(-133, 6),
      S(-50, -33),  S(-42, -5),  S(-91, 19),  S(-136, 42), S(-129, 43),
      S(-122, 34),  S(-68, 7),   S(-95, -8),  S(39, -48),  S(-12, -17),
      S(-30, -3),   S(-70, 8),   S(-76, 15),  S(-54, 6),   S(9, -14),
      S(22, -40),   S(37, -93),  S(72, -71),  S(44, -50),  S(-65, -28),
      S(6, -43),    S(-37, -32), S(51, -63),  S(45, -103),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-5, 3),   S(-5, 14),  S(-4, 45),
      S(5, 82), S(20, 145), S(91, 188), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-5, -28),  S(-8, -19),  S(0, -33),
      S(4, -49),   S(-60, -50), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-16, -6), S(-22, -15), S(-16, -13),
      S(-5, -26), S(6, -29),  S(-37, -23), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-17, -12), S(-5, -6),   S(-9, -3),
      S(-17, -7),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-35, -21), S(-17, 15), S(-5, 25), S(6, 34),  S(15, 49),
      S(25, 47),   S(36, 45),  S(45, 31), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-9, -17), S(15, 9),  S(23, 32), S(37, 46), S(45, 60),
      S(53, 67),  S(57, 71), S(61, 77), S(65, 73), S(70, 74),
      S(79, 69),  S(56, 82), S(82, 57), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-25, 59), S(-15, 78), S(-7, 95),  S(-2, 107), S(0, 111),
      S(8, 126),  S(17, 127), S(25, 134), S(39, 139), S(44, 142),
      S(52, 150), S(56, 159), S(59, 157), S(45, 162), S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-71, -93), S(-19, -33), S(4, -6),    S(11, 40),   S(20, 70),
      S(28, 68),   S(33, 107),  S(40, 115),  S(49, 114),  S(54, 139),
      S(61, 135),  S(65, 147),  S(67, 156),  S(65, 165),  S(74, 173),
      S(69, 190),  S(71, 197),  S(74, 201),  S(85, 208),  S(84, 214),
      S(120, 194), S(135, 202), S(118, 197), S(148, 203), S(126, 190),
      S(119, 182), S(122, 208), S(119, 221),
};
static const Score *pcSq[] = {taperedPawnPcSq,   taperedKnightPcSq,
                              taperedBishopPcSq, taperedRookPcSq,
                              taperedQueenPcSq,  taperedKingPcSq};

struct Trace {
    int pawnScore[NUM_COLOR] = {0};
    int knightScore[NUM_COLOR] = {0};
    int bishopScore[NUM_COLOR] = {0};
    int rookScore[NUM_COLOR] = {0};
    int queenScore[NUM_COLOR] = {0};
    int pawnPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int knightPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int bishopPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int rookPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int queenPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int kingPcSq[SQUARE_CT][NUM_COLOR] = {{0}};
    int passedPawn[8][NUM_COLOR] = {{0}};
    int doubledPawns[8][NUM_COLOR] = {0};
    int isolatedPawns[8][NUM_COLOR] = {0};
    int backwardPawns[8][NUM_COLOR] = {0};
    int knightMobility[9][NUM_COLOR] = {{0}};
    int bishopMobility[14][NUM_COLOR] = {{0}};
    int rookMobility[15][NUM_COLOR] = {{0}};
    int queenMobility[28][NUM_COLOR] = {{0}};
};

struct EvalWeights {
    const Score pawnScore = S(94, 125);
    const Score knightScore = S(381, 418);
    const Score bishopScore = S(396, 414);
    const Score rookScore = S(532, 695);
    const Score queenScore = S(1132, 1341);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(18, 118), S(9, 95),
          S(-21, 106),  S(7, 46),   S(-29, 47), S(-1, 37),  S(-115, 106),
          S(-119, 138), S(-9, 101), S(-9, 104), S(17, 51),  S(18, 1),
          S(35, -5),    S(86, 21),  S(58, 71),  S(21, 78),  S(-20, 60),
          S(-16, 42),   S(-1, 19),  S(6, -10),  S(29, -6),  S(28, 4),
          S(14, 23),    S(9, 25),   S(-36, 33), S(-28, 25), S(-12, 7),
          S(2, -7),     S(1, -4),   S(5, 4),    S(-3, 7),   S(-10, 6),
          S(-26, 24),   S(-19, 19), S(-7, 10),  S(-3, 9),   S(17, 10),
          S(10, 11),    S(27, 6),   S(2, 1),    S(-22, 26), S(-17, 25),
          S(-4, 12),    S(-2, 15),  S(9, 17),   S(36, 10),  S(43, 2),
          S(-1, 2),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-159, -49), S(-111, -36), S(-40, -13), S(-20, -12), S(45, -11),
          S(-75, -18),  S(-41, -38),  S(-91, -99), S(-26, -8),  S(1, 0),
          S(47, -19),   S(54, -5),    S(57, -15),  S(93, -18),  S(17, -17),
          S(48, -32),   S(-18, 11),   S(37, -11),  S(58, 13),   S(79, 12),
          S(101, 7),    S(121, -11),  S(72, -22),  S(47, -30),  S(2, 6),
          S(9, 17),     S(33, 39),    S(68, 35),   S(45, 34),   S(69, 29),
          S(28, 10),    S(61, -2),    S(-7, 17),   S(5, 10),    S(16, 38),
          S(29, 29),    S(36, 40),    S(34, 23),   S(53, 4),    S(22, 8),
          S(-23, -16),  S(-19, -1),   S(1, 4),     S(9, 23),    S(28, 24),
          S(13, -3),    S(16, -13),   S(-1, -9),   S(-41, -6),  S(-23, 3),
          S(-14, -5),   S(2, -2),     S(9, -2),    S(8, -5),    S(1, -21),
          S(-6, -10),   S(-86, -20),  S(-18, -22), S(-36, -7),  S(-8, 1),
          S(-4, -10),   S(-4, -10),   S(-14, -6),  S(-33, -15),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-11, 4),  S(-41, 3),  S(-64, 9),   S(-72, 5),  S(-44, -5),
          S(-56, 2),  S(-14, 1),  S(-24, -13), S(-8, -11), S(8, -10),
          S(-14, -4), S(-14, 2),  S(13, -12),  S(19, -12), S(36, -4),
          S(-7, -12), S(-10, 14), S(22, 2),    S(28, 4),   S(38, -1),
          S(35, -1),  S(59, 6),   S(55, -2),   S(34, 2),   S(-17, -2),
          S(7, 7),    S(13, 16),  S(50, 20),   S(34, 21),  S(38, 11),
          S(9, 14),   S(-3, 2),   S(-9, -3),   S(-7, 16),  S(6, 20),
          S(35, 15),  S(31, 21),  S(5, 18),    S(-2, 12),  S(28, -18),
          S(1, 11),   S(21, 6),   S(14, 17),   S(15, 16),  S(24, 21),
          S(18, 14),  S(19, 1),   S(25, -7),   S(15, 3),   S(13, -3),
          S(27, -7),  S(0, 10),   S(11, 6),    S(29, 0),   S(35, -1),
          S(20, 0),   S(-4, -2),  S(32, 15),   S(11, 11),  S(-4, 6),
          S(0, 5),    S(3, 14),   S(16, -3),   S(12, -7),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(24, 20),  S(47, 16),  S(10, 40),  S(20, 28),  S(56, 17),
          S(33, 26),  S(45, 21),  S(69, 13),  S(5, 22),   S(5, 32),
          S(26, 37),  S(62, 15),  S(39, 17),  S(73, 9),   S(42, 9),
          S(67, 6),   S(-1, 22),  S(23, 26),  S(21, 20),  S(37, 19),
          S(54, 9),   S(76, -6),  S(110, -3), S(67, 2),   S(-18, 25),
          S(9, 18),   S(2, 34),   S(13, 19),  S(20, 3),   S(27, 1),
          S(37, 7),   S(28, -2),  S(-36, 16), S(-30, 20), S(-20, 25),
          S(-17, 20), S(-11, 15), S(-26, 16), S(12, 6),   S(-8, 10),
          S(-37, 13), S(-28, 16), S(-20, 8),  S(-18, 12), S(-11, 10),
          S(-3, -4),  S(34, -21), S(-6, -14), S(-48, 6),  S(-28, 4),
          S(-10, 9),  S(-7, 6),   S(0, -6),   S(5, -10),  S(30, -18),
          S(-33, -1), S(-21, 6),  S(-17, 7),  S(-3, 7),   S(7, 5),
          S(12, -5),  S(1, -3),   S(7, -6),   S(-19, -8),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-41, 36), S(-20, 18), S(4, 50),    S(17, 60),  S(34, 48),
          S(41, 38),  S(74, 10),  S(46, 31),   S(-16, 24), S(-49, 40),
          S(-21, 65), S(-26, 85), S(-16, 105), S(33, 39),  S(18, 36),
          S(76, 36),  S(-12, 26), S(-26, 32),  S(-9, 54),  S(-12, 85),
          S(21, 79),  S(72, 54),  S(83, 16),   S(71, 55),  S(-25, 32),
          S(-24, 35), S(-21, 46), S(-21, 69),  S(-4, 80),  S(11, 79),
          S(19, 77),  S(23, 46),  S(-15, 8),   S(-27, 42), S(-28, 47),
          S(-21, 73), S(-17, 73), S(-13, 73),  S(-2, 53),  S(15, 61),
          S(-13, 3),  S(-8, 17),  S(-13, 29),  S(-15, 39), S(-12, 59),
          S(7, 29),   S(16, 19),  S(11, 32),   S(-12, 7),  S(-14, -10),
          S(3, -9),   S(15, -9),  S(8, 5),     S(19, -21), S(17, -19),
          S(30, -39), S(-7, -11), S(-1, -20),  S(10, -29), S(19, -11),
          S(9, -14),  S(-5, -6),  S(-1, -20),  S(-7, -4),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-71, -105), S(8, -65),   S(12, -35),  S(-26, -10), S(-63, -21),
          S(-26, -1),   S(18, 9),    S(6, -87),   S(-19, -34), S(-14, 10),
          S(-70, 19),   S(-4, 21),   S(0, 26),    S(-1, 41),   S(26, 37),
          S(9, 3),      S(-70, -7),  S(6, 11),    S(-30, 40),  S(-23, 53),
          S(-23, 65),   S(30, 67),   S(48, 49),   S(-6, 9),    S(-39, -18),
          S(-33, 25),   S(-62, 44),  S(-118, 60), S(-103, 63), S(-79, 62),
          S(-70, 47),   S(-77, 5),   S(-87, -23), S(-47, 4),   S(-114, 42),
          S(-134, 62),  S(-158, 66), S(-109, 48), S(-110, 28), S(-133, 6),
          S(-50, -33),  S(-42, -5),  S(-91, 19),  S(-136, 42), S(-129, 43),
          S(-122, 34),  S(-68, 7),   S(-95, -8),  S(39, -48),  S(-12, -17),
          S(-30, -3),   S(-70, 8),   S(-76, 15),  S(-54, 6),   S(9, -14),
          S(22, -40),   S(37, -93),  S(72, -71),  S(44, -50),  S(-65, -28),
          S(6, -43),    S(-37, -32), S(51, -63),  S(45, -103),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-5, 3),   S(-5, 14),  S(-4, 45),
          S(5, 82), S(20, 145), S(91, 188), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-5, -28),  S(-8, -19),  S(0, -33),
          S(4, -49),   S(-60, -50), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-16, -6), S(-22, -15), S(-16, -13),
          S(-5, -26), S(6, -29),  S(-37, -23), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-17, -12), S(-5, -6),   S(-9, -3),
          S(-17, -7),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-35, -21), S(-17, 15), S(-5, 25), S(6, 34),  S(15, 49),
          S(25, 47),   S(36, 45),  S(45, 31), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-9, -17), S(15, 9),  S(23, 32), S(37, 46), S(45, 60),
          S(53, 67),  S(57, 71), S(61, 77), S(65, 73), S(70, 74),
          S(79, 69),  S(56, 82), S(82, 57), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-25, 59), S(-15, 78), S(-7, 95),  S(-2, 107), S(0, 111),
          S(8, 126),  S(17, 127), S(25, 134), S(39, 139), S(44, 142),
          S(52, 150), S(56, 159), S(59, 157), S(45, 162), S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-71, -93), S(-19, -33), S(4, -6),    S(11, 40),   S(20, 70),
          S(28, 68),   S(33, 107),  S(40, 115),  S(49, 114),  S(54, 139),
          S(61, 135),  S(65, 147),  S(67, 156),  S(65, 165),  S(74, 173),
          S(69, 190),  S(71, 197),  S(74, 201),  S(85, 208),  S(84, 214),
          S(120, 194), S(135, 202), S(118, 197), S(148, 203), S(126, 190),
          S(119, 182), S(122, 208), S(119, 221),
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
        const auto whitePawnCount = popcount(board.pieces(PAWN, WHITE));
        const auto whiteKnightCount = popcount(board.pieces(KNIGHT, WHITE));
        const auto whiteBishopCount = popcount(board.pieces(BISHOP, WHITE));
        const auto whiteRookCount = popcount(board.pieces(ROOK, WHITE));
        const auto whiteQueenCount = popcount(board.pieces(QUEEN, WHITE));

        const auto blackPawnCount = popcount(board.pieces(PAWN, BLACK));
        const auto blackKnightCount = popcount(board.pieces(KNIGHT, BLACK));
        const auto blackBishopCount = popcount(board.pieces(BISHOP, BLACK));
        const auto blackRookCount = popcount(board.pieces(ROOK, BLACK));
        const auto blackQueenCount = popcount(board.pieces(QUEEN, BLACK));

        const auto pawnVal =
              (MgScore(pawnScore) * mgPhase + EgScore(pawnScore) * egPhase) /
              24;
        const auto knightVal = (MgScore(knightScore) * mgPhase +
                                EgScore(knightScore) * egPhase) /
                               24;
        const auto bishopVal = (MgScore(bishopScore) * mgPhase +
                                EgScore(bishopScore) * egPhase) /
                               24;
        const auto rookVal =
              (MgScore(rookScore) * mgPhase + EgScore(bishopScore) * egPhase) /
              24;
        const auto queenVal =
              (MgScore(queenScore) * mgPhase + EgScore(queenScore) * egPhase) /
              24;

        const auto wMaterial =
              (pawnVal * whitePawnCount) + (knightVal * whiteKnightCount) +
              (bishopVal * whiteBishopCount) + (rookVal * whiteRookCount) +
              (queenVal * whiteQueenCount);
        const auto bMaterial =
              (pawnVal * blackPawnCount) + (knightVal * blackKnightCount) +
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
        const int mgPcSq = MgScore(wPcSq) - MgScore(bPcSq);
        const int egPcSq = EgScore(wPcSq) - EgScore(bPcSq);

        const Score wPassedPawn = passedPawnScore<WHITE>();
        const Score bPassedPawn = passedPawnScore<BLACK>();
        const int mgPassedPawn = MgScore(wPassedPawn) - MgScore(bPassedPawn);
        const int egPassedPawn = EgScore(wPassedPawn) - EgScore(bPassedPawn);

        const Score wDoubledPawn = doubledPawnPenalty<WHITE>();
        const Score bDoubledPawn = doubledPawnPenalty<BLACK>();
        const int mgDoubledPawn = MgScore(wDoubledPawn) - MgScore(bDoubledPawn);
        const int egDoubledPawn = EgScore(wDoubledPawn) - EgScore(bDoubledPawn);

        const Score wIsolatedPawn = isolatedPawnPenalty<WHITE>();
        const Score bIsolatedPawn = isolatedPawnPenalty<BLACK>();
        const int mgIsolatedPawn =
              MgScore(wIsolatedPawn) - MgScore(bIsolatedPawn);
        const int egIsolatedPawn =
              EgScore(wIsolatedPawn) - EgScore(bIsolatedPawn);

        const Score wBackwardPawn = backwardPawnScore<WHITE>();
        const Score bBackwardPawn = backwardPawnScore<BLACK>();
        const int mgBackwardPawn =
              MgScore(wBackwardPawn) - MgScore(bBackwardPawn);
        const int egBackwardPawn =
              EgScore(wBackwardPawn) - EgScore(bBackwardPawn);

        const Score wMobility = mobilityScore<WHITE>();
        const Score bMobility = mobilityScore<BLACK>();
        const int mgMobility = MgScore(wMobility) - MgScore(bMobility);
        const int egMobility = EgScore(wMobility) - EgScore(bMobility);

        const auto color = (board.turn == WHITE) ? 1 : -1;
        const auto materialScore = wMaterial - bMaterial;
        const int pcSqEval = (mgPcSq * mgPhase + egPcSq * egPhase) / 24;
        const int passedPawnEval =
              (mgPassedPawn * mgPhase + egPassedPawn * egPhase) / 24;
        const int doubledPawnEval =
              (mgDoubledPawn * mgPhase + egDoubledPawn * egPhase) / 24;
        const int isolatedPawnEval =
              (mgIsolatedPawn * mgPhase + egIsolatedPawn * egPhase) / 24;
        const int backwardPawnEval =
              (mgBackwardPawn * mgPhase + egBackwardPawn * egPhase) / 24;
        const int mobilityEval =
              (mgMobility * mgPhase + egMobility * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + passedPawnEval + doubledPawnEval +
                isolatedPawnEval + backwardPawnEval + mobilityEval;

        return eval * color;
    }

  public:
    int phase;
    int mgPhase;
    int egPhase;

  private:
    Board &board;
    Trace &trace;

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
        phase = 0;
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

template <Tracing T>
template <Color C>
constexpr Bitboard Eval<T>::doubledPawns() {
    Bitboard pawns = board.pieces(PAWN, C);
    Bitboard blockedPawns = 0;

    while (pawns) {
        Bitboard b = SQUARE_BB(Square(lsb_index(pawns)));
        Bitboard pushes = fill<pushDirection(C)>(shift<pushDirection(C)>(b));

        if (pushes & board.pieces(PAWN, C)) {
            blockedPawns |= b;
        }

        pawns &= pawns - 1;
    }

    return blockedPawns;
}

template <Tracing T>
template <Color C>
constexpr Bitboard Eval<T>::backwardPawns() {
    constexpr Direction Up = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);

    const Bitboard pawns = board.pieces(PAWN, C);
    const Bitboard enemyPawns = board.pieces(PAWN, ~C);
    const Bitboard stopSquare = shift<Up>(pawns);

    const Bitboard candidateBackwardPawns =
          shift<Down>(pawnDblAttacks<~C>(enemyPawns) & stopSquare) & pawns;
    const Bitboard defendedStopSquares = pawnDblAttacks<C>(pawns) & stopSquare;
    const Bitboard backwardPawns =
          candidateBackwardPawns & ~shift<Down>(defendedStopSquares);

    return backwardPawns;
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::backwardPawnScore() {
    Bitboard bckPawns = backwardPawns<C>();

    int mgScore = 0, egScore = 0;
    while (bckPawns) {
        Square psq = Square(lsb_index(bckPawns));
        psq = (C == WHITE) ? psq : Square(mirror(psq));

        mgScore += MgScore(backwardPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(backwardPawnRankBonus[RANK_OF(psq)]);

        if (T) {
            trace.backwardPawns[RANK_OF(psq)][C]++;
        }

        bckPawns &= bckPawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::isolatedPawnPenalty() {
    Bitboard pawns = board.pieces(PAWN, C);

    int mgScore = 0, egScore = 0;
    while (pawns) {
        Square psq = Square(lsb_index(pawns));
        psq = (C == WHITE) ? psq : Square(mirror(psq));

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

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::passedPawnScore() {
    constexpr Direction Up = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);
    Bitboard ourPawns = board.pieces(PAWN, C);
    Bitboard enemyPawns = board.pieces(PAWN, ~C);

    Bitboard opponentPawnSpan = fill<Down>(shift<Down>(enemyPawns));
    opponentPawnSpan |=
          shift<WEST>(opponentPawnSpan) | shift<EAST>(opponentPawnSpan);

    Bitboard passedPawns = board.pieces(PAWN, C) & ~opponentPawnSpan;

    Rank startRank = (C == WHITE) ? RANK_2 : RANK_7;
    Bitboard startPassed = passedPawns & startRank;

    int mgScore = 0;
    int egScore = 0;

    while (passedPawns) {
        Square psq = Square(lsb_index(passedPawns));
        psq = (C == WHITE) ? psq : Square(mirror(psq));

        mgScore += MgScore(passedPawnRankBonus[RANK_OF(psq)]);
        egScore += EgScore(passedPawnRankBonus[RANK_OF(psq)]);

        if (T) {
            trace.passedPawn[RANK_OF(psq)][C]++;
        }

        passedPawns &= passedPawns - 1;
    }

    return S(mgScore, egScore);
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::doubledPawnPenalty() {
    Bitboard dblPawns = doubledPawns<C>();

    int mgScore = 0, egScore = 0;
    while (dblPawns) {
        Square psq = Square(lsb_index(dblPawns));
        psq = (C == WHITE) ? psq : Square(mirror(psq));

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
        Piece p = board.board[i];
        PieceT pt = getPcType(p);

        if (C == WHITE) {
            if (p < B_PAWN && p != NO_PC) {
                if (T) {

                    auto pcSq = {&trace.pawnPcSq,   &trace.knightPcSq,
                                 &trace.bishopPcSq, &trace.rookPcSq,
                                 &trace.queenPcSq,  &trace.kingPcSq};

                    (*std::data(pcSq))[pt - 1][i][WHITE] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][i]);
                egScore += EgScore(pcSq[pt - 1][i]);
            }
        } else if (C == BLACK) {
            if (p >= B_PAWN && p != NO_PC) {
                if (T) {
                    auto pcSq = {&trace.pawnPcSq,   &trace.knightPcSq,
                                 &trace.bishopPcSq, &trace.rookPcSq,
                                 &trace.queenPcSq,  &trace.kingPcSq};

                    (*std::data(pcSq))[pt - 1][mirror(i)][BLACK] = 1;
                }

                mgScore += MgScore(pcSq[pt - 1][mirror(i)]);
                egScore += EgScore(pcSq[pt - 1][mirror(i)]);
            }
        }
    }

    return S(mgScore, egScore);
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::mobilityScore() {
    constexpr Direction Up = pushDirection(C);
    constexpr Direction Down = pushDirection(~C);

    Bitboard knights = board.pieces(KNIGHT, C);
    Bitboard bishops = board.pieces(BISHOP, C);
    Bitboard rooks = board.pieces(ROOK, C);
    Bitboard queens = board.pieces(QUEEN, C);

    const Bitboard friendlyPawns = board.pieces(PAWN, C);
    const Bitboard enemyPawns = board.pieces(PAWN, ~C);

    const Bitboard secondThirdRank =
          (C == WHITE) ? (RANK_2BB | RANK_3BB) : (RANK_7BB | RANK_6BB);
    const Bitboard enemyPawnAttacks = pawnDblAttacks<~C>(enemyPawns);
    const Bitboard secondThirdRankPawns = friendlyPawns & secondThirdRank;
    const Bitboard blockedPawns = shift<Down>(enemyPawns) & friendlyPawns;
    const Bitboard friendlyKing = board.pieces(KING, C);
    const Bitboard friendlyQueens = board.pieces(QUEEN, C);

    const Bitboard excludedSquares = enemyPawnAttacks | secondThirdRankPawns |
                                     blockedPawns | friendlyKing |
                                     friendlyQueens;

    int mgScore = 0;
    int egScore = 0;

    while (knights) {
        Square knightSq = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves);
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
        Square bishopSq = Square(lsb_index(bishops));
        Bitboard bishopMoves =
              getBishopAttacks(bishopSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(bishopMoves);
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
        Square rookSq = Square(lsb_index(rooks));
        Bitboard rookMoves =
              getRookAttacks(rookSq, board.pieces()) & ~excludedSquares;

        int numMoves = popcount(rookMoves);
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
        Square queenSq = Square(lsb_index(queens));
        Bitboard queenMoves = getRookAttacks(queenSq, board.pieces()) |
                              getBishopAttacks(queenSq, board.pieces());
        queenMoves &= ~excludedSquares;

        int numMoves = popcount(queenMoves);
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
