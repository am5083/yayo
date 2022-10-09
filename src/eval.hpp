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
#include <unordered_map>

namespace Yayo {
namespace {
constexpr int TEMPO = 10;
} // namespace

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

constexpr Score blockedPassedPawnRankBonus[] = {
      S(0, 0),     S(0, 0),     S(0, 0),     S(40, 40),
      S(200, 200), S(260, 260), S(400, 400), S(0, 0)};

constexpr Score pawnScore = S(94, 123);
constexpr Score knightScore = S(376, 441);
constexpr Score bishopScore = S(410, 462);
constexpr Score rookScore = S(520, 812);
constexpr Score queenScore = S(1131, 1474);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(19, 109), S(22, 82),
      S(-11, 97),   S(24, 42),  S(-31, 44), S(-16, 30), S(-127, 112),
      S(-135, 137), S(-3, 99),  S(2, 97),   S(39, 44),  S(46, -7),
      S(61, -15),   S(101, 14), S(68, 64),  S(32, 70),  S(-17, 62),
      S(-4, 41),    S(11, 19),  S(17, -9),  S(42, -6),  S(40, 7),
      S(22, 24),    S(16, 24),  S(-31, 31), S(-20, 22), S(-7, 8),
      S(16, -9),    S(12, -4),  S(9, 5),    S(4, 7),    S(-4, 5),
      S(-30, 22),   S(-24, 20), S(-9, 10),  S(-11, 14), S(8, 12),
      S(4, 13),     S(27, 4),   S(2, 0),    S(-32, 28), S(-26, 25),
      S(-12, 16),   S(-22, 11), S(-3, 17),  S(25, 11),  S(36, 2),
      S(-7, 4),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-184, -67), S(-119, -35), S(-50, -14),   S(-14, -16), S(33, -11),
      S(-81, -18),  S(-69, -32),  S(-101, -116), S(-21, -18), S(8, -1),
      S(53, -5),    S(64, -5),    S(63, -13),    S(103, -13), S(19, -15),
      S(56, -42),   S(-6, -1),    S(48, -2),     S(77, 23),   S(99, 17),
      S(113, 16),   S(136, -1),   S(78, -16),    S(40, -33),  S(-2, 3),
      S(18, 21),    S(50, 46),    S(81, 41),     S(50, 46),   S(87, 38),
      S(26, 19),    S(52, -17),   S(-13, 14),    S(10, 13),   S(29, 45),
      S(32, 38),    S(44, 49),    S(31, 31),     S(45, 14),   S(12, -6),
      S(-29, -22),  S(-8, 4),     S(10, 14),     S(20, 32),   S(36, 37),
      S(17, 9),     S(15, -5),    S(-15, -22),   S(-60, -13), S(-28, 2),
      S(-15, -2),   S(0, 1),      S(-2, 0),      S(7, 2),     S(-7, -29),
      S(-9, -23),   S(-106, -37), S(-40, -52),   S(-44, -15), S(-31, -10),
      S(-26, -18),  S(-6, -26),   S(-31, -26),   S(-55, -28),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-9, -2), S(-59, 11), S(-41, 9),   S(-80, 16), S(-53, -1),  S(-60, 6),
      S(-21, 2), S(-34, -6), S(-3, -16),  S(37, -3),  S(12, 1),    S(-9, 16),
      S(34, -3), S(19, -5),  S(45, 6),    S(-5, -13), S(4, 11),    S(35, 10),
      S(44, 9),  S(53, 3),   S(43, 12),   S(65, 13),  S(59, 8),    S(30, 4),
      S(-10, 3), S(17, 16),  S(30, 29),   S(55, 34),  S(50, 32),   S(48, 21),
      S(19, 21), S(-1, 9),   S(-11, 3),   S(-1, 27),  S(10, 31),   S(47, 26),
      S(36, 34), S(15, 26),  S(8, 23),    S(14, -13), S(13, 4),    S(16, 6),
      S(17, 23), S(20, 24),  S(26, 31),   S(18, 25),  S(17, 12),   S(20, -9),
      S(10, -4), S(16, -4),  S(23, -2),   S(4, 8),    S(11, 13),   S(24, 5),
      S(35, -3), S(13, -3),  S(-12, -20), S(13, 15),  S(-10, -13), S(-16, 5),
      S(-8, 1),  S(-12, 4),  S(20, -17),  S(2, -22),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(31, 39),  S(53, 26),  S(19, 55),  S(31, 45),  S(67, 29),  S(52, 31),
      S(60, 36),  S(86, 19),  S(18, 28),  S(17, 41),  S(39, 44),  S(72, 29),
      S(38, 28),  S(78, 20),  S(49, 14),  S(76, 13),  S(11, 28),  S(37, 30),
      S(37, 27),  S(36, 28),  S(75, 10),  S(77, 0),   S(120, -3), S(77, 9),
      S(-7, 25),  S(31, 20),  S(12, 33),  S(14, 33),  S(28, 3),   S(25, 4),
      S(40, 8),   S(38, -2),  S(-35, 16), S(-34, 24), S(-14, 33), S(-2, 29),
      S(2, 19),   S(-27, 13), S(7, 9),    S(-6, 12),  S(-34, 12), S(-25, 14),
      S(-21, 11), S(-12, 15), S(-11, 21), S(-15, -4), S(27, -23), S(-4, -14),
      S(-49, 10), S(-29, 6),  S(-9, 8),   S(-22, 11), S(-8, -3),  S(1, -7),
      S(21, -15), S(-33, -2), S(-16, 7),  S(-17, 16), S(-6, 22),  S(1, 18),
      S(7, 6),    S(-6, 2),   S(6, -2),   S(-18, -9),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-55, 55), S(-25, 30),  S(-3, 73),   S(28, 81),   S(36, 70),
      S(47, 56),  S(99, 18),   S(29, 46),   S(-8, 32),   S(-26, 46),
      S(-11, 86), S(-23, 123), S(-11, 132), S(36, 59),   S(9, 49),
      S(58, 45),  S(-2, 28),   S(-15, 40),  S(4, 74),    S(-3, 93),
      S(24, 95),  S(80, 69),   S(79, 11),   S(51, 54),   S(-19, 32),
      S(-4, 40),  S(-10, 49),  S(-3, 88),   S(10, 92),   S(16, 94),
      S(18, 82),  S(19, 50),   S(-7, 2),    S(-14, 41),  S(-14, 56),
      S(-5, 80),  S(-6, 89),   S(-13, 86),  S(2, 63),    S(6, 56),
      S(-8, 5),   S(-3, 21),   S(-8, 38),   S(-4, 41),   S(-7, 64),
      S(8, 24),   S(13, 15),   S(3, 24),    S(-15, 3),   S(-8, -8),
      S(9, -10),  S(13, -2),   S(5, 12),    S(20, -26),  S(7, -28),
      S(20, -63), S(-12, -9),  S(-18, -21), S(-12, -19), S(3, -14),
      S(-8, -19), S(-24, -3),  S(-12, -36), S(-17, -16),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-62, -105), S(25, -62),  S(30, -34),   S(-23, -7),  S(-53, -10),
      S(1, 2),      S(50, 18),   S(32, -109),  S(-35, -22), S(-8, 10),
      S(-84, 31),   S(2, 18),    S(6, 26),     S(-1, 44),   S(64, 35),
      S(51, -2),    S(-98, 1),   S(-1, 12),    S(-46, 40),  S(-34, 57),
      S(-36, 68),   S(39, 64),   S(61, 46),    S(9, 12),    S(-46, -10),
      S(-42, 28),   S(-91, 44),  S(-170, 61),  S(-144, 67), S(-111, 62),
      S(-107, 52),  S(-104, 10), S(-114, -25), S(-80, 10),  S(-155, 44),
      S(-180, 62),  S(-202, 69), S(-137, 52),  S(-136, 32), S(-171, 15),
      S(-58, -32),  S(-47, -1),  S(-97, 24),   S(-144, 44), S(-123, 45),
      S(-120, 33),  S(-58, 7),   S(-86, -6),   S(47, -52),  S(-1, -18),
      S(-17, -2),   S(-54, 6),   S(-60, 14),   S(-31, 3),   S(26, -17),
      S(35, -39),   S(47, -113), S(73, -70),   S(39, -50),  S(-86, -28),
      S(-7, -48),   S(-49, -27), S(51, -59),   S(51, -100),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-7, 6),   S(-8, 10),  S(-6, 42),
      S(9, 78), S(13, 148), S(87, 176), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-7, -28),  S(-8, -18),  S(-1, -33),
      S(15, -51),  S(-66, -62), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-16, -8), S(-21, -15), S(-18, -12),
      S(-7, -27), S(8, -34),  S(8, -19),   S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-18, -6),  S(-3, -7),   S(-8, -4),
      S(-13, -5),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-10, -37), S(-2, -7), S(1, 5),    S(5, 3),   S(8, 14),
      S(13, 8),    S(20, -1), S(27, -15), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-12, -60), S(5, -37), S(5, -15), S(13, -9), S(16, 0),
      S(16, 10),   S(18, 9),  S(10, 10), S(16, 7),  S(9, 11),
      S(23, 2),    S(14, 28), S(34, -3), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-31, -30), S(-20, -17), S(-16, -24), S(-21, -20), S(-22, -19),
      S(-22, -8),  S(-24, -9),  S(-21, -3),  S(-14, -6),  S(-19, 4),
      S(-23, 7),   S(-32, 21),  S(-39, 15),  S(-58, 20),  S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-85, -115), S(-35, -57), S(-9, -48),  S(-11, -20), S(2, -38),
      S(8, -59),    S(10, -36),  S(16, -47),  S(21, -50),  S(15, -29),
      S(21, -36),   S(20, -28),  S(17, -22),  S(13, -9),   S(17, -2),
      S(0, 19),     S(-8, 24),   S(-18, 41),  S(-19, 56),  S(-30, 57),
      S(16, 39),    S(29, 60),   S(5, 64),    S(87, 111),  S(64, 93),
      S(86, 152),   S(78, 160),  S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(10, -11), S(12, -1), S(20, -5), S(34, -9), S(23, -14),
};
constexpr Score trappedRookWeight = S(1, 14);
constexpr Score rookOnOpenFile[2] = {
      S(19, 8),
      S(23, 3),
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
    int kingAttackers[5][NUM_COLOR] = {{0}};
    int trappedRooks[NUM_COLOR] = {0};
    int openFileRooks[2][NUM_COLOR] = {{0}};
};

struct EvalWeights {
    const Score pawnScore = S(94, 123);
    const Score knightScore = S(376, 441);
    const Score bishopScore = S(410, 462);
    const Score rookScore = S(520, 812);
    const Score queenScore = S(1131, 1474);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(19, 109), S(22, 82),
          S(-11, 97),   S(24, 42),  S(-31, 44), S(-16, 30), S(-127, 112),
          S(-135, 137), S(-3, 99),  S(2, 97),   S(39, 44),  S(46, -7),
          S(61, -15),   S(101, 14), S(68, 64),  S(32, 70),  S(-17, 62),
          S(-4, 41),    S(11, 19),  S(17, -9),  S(42, -6),  S(40, 7),
          S(22, 24),    S(16, 24),  S(-31, 31), S(-20, 22), S(-7, 8),
          S(16, -9),    S(12, -4),  S(9, 5),    S(4, 7),    S(-4, 5),
          S(-30, 22),   S(-24, 20), S(-9, 10),  S(-11, 14), S(8, 12),
          S(4, 13),     S(27, 4),   S(2, 0),    S(-32, 28), S(-26, 25),
          S(-12, 16),   S(-22, 11), S(-3, 17),  S(25, 11),  S(36, 2),
          S(-7, 4),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-184, -67), S(-119, -35), S(-50, -14),   S(-14, -16), S(33, -11),
          S(-81, -18),  S(-69, -32),  S(-101, -116), S(-21, -18), S(8, -1),
          S(53, -5),    S(64, -5),    S(63, -13),    S(103, -13), S(19, -15),
          S(56, -42),   S(-6, -1),    S(48, -2),     S(77, 23),   S(99, 17),
          S(113, 16),   S(136, -1),   S(78, -16),    S(40, -33),  S(-2, 3),
          S(18, 21),    S(50, 46),    S(81, 41),     S(50, 46),   S(87, 38),
          S(26, 19),    S(52, -17),   S(-13, 14),    S(10, 13),   S(29, 45),
          S(32, 38),    S(44, 49),    S(31, 31),     S(45, 14),   S(12, -6),
          S(-29, -22),  S(-8, 4),     S(10, 14),     S(20, 32),   S(36, 37),
          S(17, 9),     S(15, -5),    S(-15, -22),   S(-60, -13), S(-28, 2),
          S(-15, -2),   S(0, 1),      S(-2, 0),      S(7, 2),     S(-7, -29),
          S(-9, -23),   S(-106, -37), S(-40, -52),   S(-44, -15), S(-31, -10),
          S(-26, -18),  S(-6, -26),   S(-31, -26),   S(-55, -28),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-9, -2),  S(-59, 11),  S(-41, 9),  S(-80, 16),  S(-53, -1),
          S(-60, 6),  S(-21, 2),   S(-34, -6), S(-3, -16),  S(37, -3),
          S(12, 1),   S(-9, 16),   S(34, -3),  S(19, -5),   S(45, 6),
          S(-5, -13), S(4, 11),    S(35, 10),  S(44, 9),    S(53, 3),
          S(43, 12),  S(65, 13),   S(59, 8),   S(30, 4),    S(-10, 3),
          S(17, 16),  S(30, 29),   S(55, 34),  S(50, 32),   S(48, 21),
          S(19, 21),  S(-1, 9),    S(-11, 3),  S(-1, 27),   S(10, 31),
          S(47, 26),  S(36, 34),   S(15, 26),  S(8, 23),    S(14, -13),
          S(13, 4),   S(16, 6),    S(17, 23),  S(20, 24),   S(26, 31),
          S(18, 25),  S(17, 12),   S(20, -9),  S(10, -4),   S(16, -4),
          S(23, -2),  S(4, 8),     S(11, 13),  S(24, 5),    S(35, -3),
          S(13, -3),  S(-12, -20), S(13, 15),  S(-10, -13), S(-16, 5),
          S(-8, 1),   S(-12, 4),   S(20, -17), S(2, -22),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(31, 39),  S(53, 26),  S(19, 55),  S(31, 45),  S(67, 29),
          S(52, 31),  S(60, 36),  S(86, 19),  S(18, 28),  S(17, 41),
          S(39, 44),  S(72, 29),  S(38, 28),  S(78, 20),  S(49, 14),
          S(76, 13),  S(11, 28),  S(37, 30),  S(37, 27),  S(36, 28),
          S(75, 10),  S(77, 0),   S(120, -3), S(77, 9),   S(-7, 25),
          S(31, 20),  S(12, 33),  S(14, 33),  S(28, 3),   S(25, 4),
          S(40, 8),   S(38, -2),  S(-35, 16), S(-34, 24), S(-14, 33),
          S(-2, 29),  S(2, 19),   S(-27, 13), S(7, 9),    S(-6, 12),
          S(-34, 12), S(-25, 14), S(-21, 11), S(-12, 15), S(-11, 21),
          S(-15, -4), S(27, -23), S(-4, -14), S(-49, 10), S(-29, 6),
          S(-9, 8),   S(-22, 11), S(-8, -3),  S(1, -7),   S(21, -15),
          S(-33, -2), S(-16, 7),  S(-17, 16), S(-6, 22),  S(1, 18),
          S(7, 6),    S(-6, 2),   S(6, -2),   S(-18, -9),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-55, 55), S(-25, 30),  S(-3, 73),   S(28, 81),   S(36, 70),
          S(47, 56),  S(99, 18),   S(29, 46),   S(-8, 32),   S(-26, 46),
          S(-11, 86), S(-23, 123), S(-11, 132), S(36, 59),   S(9, 49),
          S(58, 45),  S(-2, 28),   S(-15, 40),  S(4, 74),    S(-3, 93),
          S(24, 95),  S(80, 69),   S(79, 11),   S(51, 54),   S(-19, 32),
          S(-4, 40),  S(-10, 49),  S(-3, 88),   S(10, 92),   S(16, 94),
          S(18, 82),  S(19, 50),   S(-7, 2),    S(-14, 41),  S(-14, 56),
          S(-5, 80),  S(-6, 89),   S(-13, 86),  S(2, 63),    S(6, 56),
          S(-8, 5),   S(-3, 21),   S(-8, 38),   S(-4, 41),   S(-7, 64),
          S(8, 24),   S(13, 15),   S(3, 24),    S(-15, 3),   S(-8, -8),
          S(9, -10),  S(13, -2),   S(5, 12),    S(20, -26),  S(7, -28),
          S(20, -63), S(-12, -9),  S(-18, -21), S(-12, -19), S(3, -14),
          S(-8, -19), S(-24, -3),  S(-12, -36), S(-17, -16),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-62, -105), S(25, -62),  S(30, -34),   S(-23, -7),  S(-53, -10),
          S(1, 2),      S(50, 18),   S(32, -109),  S(-35, -22), S(-8, 10),
          S(-84, 31),   S(2, 18),    S(6, 26),     S(-1, 44),   S(64, 35),
          S(51, -2),    S(-98, 1),   S(-1, 12),    S(-46, 40),  S(-34, 57),
          S(-36, 68),   S(39, 64),   S(61, 46),    S(9, 12),    S(-46, -10),
          S(-42, 28),   S(-91, 44),  S(-170, 61),  S(-144, 67), S(-111, 62),
          S(-107, 52),  S(-104, 10), S(-114, -25), S(-80, 10),  S(-155, 44),
          S(-180, 62),  S(-202, 69), S(-137, 52),  S(-136, 32), S(-171, 15),
          S(-58, -32),  S(-47, -1),  S(-97, 24),   S(-144, 44), S(-123, 45),
          S(-120, 33),  S(-58, 7),   S(-86, -6),   S(47, -52),  S(-1, -18),
          S(-17, -2),   S(-54, 6),   S(-60, 14),   S(-31, 3),   S(26, -17),
          S(35, -39),   S(47, -113), S(73, -70),   S(39, -50),  S(-86, -28),
          S(-7, -48),   S(-49, -27), S(51, -59),   S(51, -100),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-7, 6),   S(-8, 10),  S(-6, 42),
          S(9, 78), S(13, 148), S(87, 176), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-7, -28),  S(-8, -18),  S(-1, -33),
          S(15, -51),  S(-66, -62), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-16, -8), S(-21, -15), S(-18, -12),
          S(-7, -27), S(8, -34),  S(8, -19),   S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-18, -6),  S(-3, -7),   S(-8, -4),
          S(-13, -5),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-10, -37), S(-2, -7), S(1, 5),    S(5, 3),   S(8, 14),
          S(13, 8),    S(20, -1), S(27, -15), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-12, -60), S(5, -37), S(5, -15), S(13, -9), S(16, 0),
          S(16, 10),   S(18, 9),  S(10, 10), S(16, 7),  S(9, 11),
          S(23, 2),    S(14, 28), S(34, -3), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-31, -30), S(-20, -17), S(-16, -24), S(-21, -20), S(-22, -19),
          S(-22, -8),  S(-24, -9),  S(-21, -3),  S(-14, -6),  S(-19, 4),
          S(-23, 7),   S(-32, 21),  S(-39, 15),  S(-58, 20),  S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-85, -115), S(-35, -57), S(-9, -48),  S(-11, -20), S(2, -38),
          S(8, -59),    S(10, -36),  S(16, -47),  S(21, -50),  S(15, -29),
          S(21, -36),   S(20, -28),  S(17, -22),  S(13, -9),   S(17, -2),
          S(0, 19),     S(-8, 24),   S(-18, 41),  S(-19, 56),  S(-30, 57),
          S(16, 39),    S(29, 60),   S(5, 64),    S(87, 111),  S(64, 93),
          S(86, 152),   S(78, 160),  S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(10, -11), S(12, -1), S(20, -5), S(34, -9), S(23, -14),
    };
    const Score trappedRookWeight = S(1, 14);
    const Score rookOnOpenFile[2] = {
          S(19, 8),
          S(23, 3),
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

        if (evalTable.contains(board.key)) {
            return evalTable[board.key];
        }

        const Score wPcSq = pieceSquare<WHITE>();
        const Score bPcSq = pieceSquare<BLACK>();
        const int mgPcSq = MgScore(wPcSq) - MgScore(bPcSq);
        const int egPcSq = EgScore(wPcSq) - EgScore(bPcSq);

        const Score wMobility = mobilityScore<WHITE>();
        const Score bMobility = mobilityScore<BLACK>();
        const int mgMobility = MgScore(wMobility) - MgScore(bMobility);
        const int egMobility = EgScore(wMobility) - EgScore(bMobility);

        const Score wKingSafety = kingAttackers<WHITE>();
        const Score bKingSafety = kingAttackers<BLACK>();
        // const Score wKingSafety = S(0, 0);
        // const Score bKingSafety = wKingSafety;
        const int mgSafety = MgScore(wKingSafety) - MgScore(bKingSafety);
        const int egSafety = EgScore(wKingSafety) - EgScore(bKingSafety);

        const Score wRookEval = rookEval<WHITE>();
        const Score bRookEval = rookEval<BLACK>();
        const int mgRookEval = MgScore(wRookEval) - MgScore(bRookEval);
        const int egRookEval = EgScore(wRookEval) - EgScore(bRookEval);

        const auto color = (board.turn == WHITE) ? 1 : -1;
        const auto materialScore = materialEval();
        const auto pawnStructureEval = pawnStructure();

        const int pcSqEval = (mgPcSq * mgPhase + egPcSq * egPhase) / 24;
        const int mobilityEval =
              (mgMobility * mgPhase + egMobility * egPhase) / 24;
        const int safetyEval = (mgSafety * mgPhase + egSafety * egPhase) / 24;
        const int rookEval = (mgRookEval * mgPhase + egRookEval * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + pawnStructureEval + mobilityEval +
                rookEval + safetyEval;

        evalTable[board.key] = eval * color;

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
    std::unordered_map<std::uint64_t, int> pawnTable;
    std::unordered_map<std::uint64_t, int> evalTable;
    std::unordered_map<std::uint64_t, Score> mobilityTable;
    std::unordered_map<std::uint64_t, Score> safetyTable;
    std::unordered_map<std::uint64_t, Score> rookTable;

  private:
    constexpr int materialEval();
    constexpr int pawnStructure();

  private:
    template <Color C> constexpr Bitboard doubledPawns();
    template <Color C> constexpr Bitboard backwardPawns();

    template <Color C> constexpr Score isolatedPawnPenalty();
    template <Color C> constexpr Score backwardPawnScore();
    template <Color C> constexpr Score passedPawnScore();
    template <Color C> constexpr Score doubledPawnPenalty();
    template <Color C> constexpr Score pieceSquare();
    template <Color C> constexpr Score mobilityScore();
    template <Color C> constexpr Score kingAttackers();
    template <Color C> constexpr Score rookEval();

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

template <Tracing T> template <Color C> constexpr Score Eval<T>::rookEval() {
    if (rookTable.contains(board.key))
        return rookTable[board.key];

    const Direction Up = pushDirection(C);
    const Rank startRank = C == WHITE ? RANK_1 : RANK_8;
    Bitboard rooks = board.pieces(ROOK, C);

    int mg = 0, eg = 0;
    int numOnOpenFiles = 0;
    Bitboard temp = rooks;
    while (temp) {
        Square rookSquare = Square(lsb_index(temp));
        Bitboard rookMask = SQUARE_BB(rookSquare);
        File rookFile = FILE_OF(rookSquare);

        int open = 0;
        if (FILE_BB(rookFile) & board.pieces(PAWN, C)) {
            temp &= temp - 1;
            continue;
        } else if (FILE_BB(rookFile) & board.pieces(PAWN, ~C))
            open = 1;
        else
            open = 2;

        if (T) {
            trace.openFileRooks[open - 1][C]++;
        }

        rooks ^= rookMask;

        mg += MgScore(rookOnOpenFile[open - 1]);
        eg += EgScore(rookOnOpenFile[open - 1]);

        temp &= temp - 1;
    }

    int numTrapped = 0;
    while (rooks) {
        Square rookSquare = Square(lsb_index(rooks));
        if (popcount(getRookAttacks(rookSquare, board.pieces())) > 3) {
            rooks &= rooks - 1;
            continue;
        } else {
            numTrapped++;
        }
        rooks &= rooks - 1;
    }

    if (T) {
        trace.trappedRooks[C] = numTrapped;
    }

    int mgScore = MgScore(trappedRookWeight) * numTrapped;
    int egScore = EgScore(trappedRookWeight) * numTrapped;

    rookTable[board.key] = S(mgScore, egScore);

    return S(mgScore, egScore);
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::kingAttackers() {
    if (safetyTable.contains(board.key))
        return safetyTable[board.key];

    const Bitboard king = board.pieces(KING, ~C);
    const Square kingSquare = Sq(king);
    Bitboard kingArea = kingAttacks[kingSquare];

    int mgScore = 0, egScore = 0;
    while (kingArea) {
        Square areaSq = Square(lsb_index(kingArea));
        Bitboard attackPcs = board.attacksToKing<C>(areaSq, board.pieces());

        while (attackPcs) {
            Square attacker = Square(lsb_index(attackPcs));
            PieceT piece = getPcType(board.board[attacker]);

            if (T == TRACE) {
                trace.kingAttackers[piece - 1][C]++;
            }

            mgScore += MgScore(kingAttackersWeight[piece - 1]);
            egScore += EgScore(kingAttackersWeight[piece - 1]);
            attackPcs &= attackPcs - 1;
        }

        kingArea &= kingArea - 1;
    }

    safetyTable[board.key] = S(mgScore, egScore);

    return S(mgScore, egScore);
}

template <Tracing T> constexpr int Eval<T>::pawnStructure() {
    Bitboard key = board.pieces(PAWN, WHITE) ^ board.pieces(PAWN, BLACK);
    if (pawnTable.contains(key)) {
        return pawnTable.at(key);
    }

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
    const int mgIsolatedPawn = MgScore(wIsolatedPawn) - MgScore(bIsolatedPawn);
    const int egIsolatedPawn = EgScore(wIsolatedPawn) - EgScore(bIsolatedPawn);

    const Score wBackwardPawn = backwardPawnScore<WHITE>();
    const Score bBackwardPawn = backwardPawnScore<BLACK>();
    const int mgBackwardPawn = MgScore(wBackwardPawn) - MgScore(bBackwardPawn);
    const int egBackwardPawn = EgScore(wBackwardPawn) - EgScore(bBackwardPawn);

    const int passedPawnEval =
          (mgPassedPawn * mgPhase + egPassedPawn * egPhase) / 24;
    const int doubledPawnEval =
          (mgDoubledPawn * mgPhase + egDoubledPawn * egPhase) / 24;
    const int isolatedPawnEval =
          (mgIsolatedPawn * mgPhase + egIsolatedPawn * egPhase) / 24;
    const int backwardPawnEval =
          (mgBackwardPawn * mgPhase + egBackwardPawn * egPhase) / 24;

    const auto pawnStructure = passedPawnEval + doubledPawnEval +
                               isolatedPawnEval + backwardPawnEval;

    pawnTable[key] = pawnStructure;

    return pawnStructure;
}

template <Tracing T> constexpr int Eval<T>::materialEval() {
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
          (MgScore(pawnScore) * mgPhase + EgScore(pawnScore) * egPhase) / 24;
    const auto knightVal =
          (MgScore(knightScore) * mgPhase + EgScore(knightScore) * egPhase) /
          24;
    const auto bishopVal =
          (MgScore(bishopScore) * mgPhase + EgScore(bishopScore) * egPhase) /
          24;
    const auto rookVal =
          (MgScore(rookScore) * mgPhase + EgScore(bishopScore) * egPhase) / 24;
    const auto queenVal =
          (MgScore(queenScore) * mgPhase + EgScore(queenScore) * egPhase) / 24;

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

    return wMaterial - bMaterial;
}

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
    if (mobilityTable.contains(board.key))
        return mobilityTable[board.key];

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
        Square bishopSq = Square(lsb_index(bishops));
        Bitboard bishopMoves =
              getBishopAttacks(bishopSq, board.pieces()) & ~excludedSquares;

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
        Square rookSq = Square(lsb_index(rooks));
        Bitboard rookMoves =
              getRookAttacks(rookSq, board.pieces()) & ~excludedSquares;

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
        Square queenSq = Square(lsb_index(queens));
        Bitboard queenMoves = getRookAttacks(queenSq, board.pieces()) |
                              getBishopAttacks(queenSq, board.pieces());
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

    mobilityTable[board.key] = S(mgScore, egScore);

    return S(mgScore, egScore);
}

} // namespace Yayo
#endif // SEARCH_H_
