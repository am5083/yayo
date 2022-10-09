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
#include "src/hashtable.hpp"
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

constexpr Score pawnScore = S(93, 134);
constexpr Score knightScore = S(382, 467);
constexpr Score bishopScore = S(417, 490);
constexpr Score rookScore = S(551, 866);
constexpr Score queenScore = S(1158, 1561);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(27, 107), S(22, 90),
      S(-17, 99),   S(30, 39),  S(-32, 56), S(-16, 37), S(-141, 123),
      S(-157, 143), S(0, 94),   S(3, 101),  S(33, 44),  S(41, -5),
      S(56, -15),   S(98, 14),  S(64, 70),  S(39, 70),  S(-19, 61),
      S(-13, 43),   S(3, 20),   S(10, -10), S(35, -3),  S(34, 10),
      S(18, 30),    S(14, 25),  S(-35, 33), S(-28, 25), S(-13, 10),
      S(4, -7),     S(1, -2),   S(5, 6),    S(-3, 9),   S(-7, 5),
      S(-29, 23),   S(-22, 19), S(-11, 11), S(-6, 11),  S(13, 12),
      S(6, 14),     S(25, 5),   S(6, -2),   S(-25, 26), S(-20, 25),
      S(-9, 13),    S(-7, 16),  S(5, 15),   S(35, 10),  S(40, 2),
      S(1, -2),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-165, -77), S(-129, -44), S(-53, -11),  S(1, -20),   S(24, -6),
      S(-81, -10),  S(-74, -23),  S(-93, -119), S(-27, -18), S(16, 3),
      S(59, -15),   S(78, -10),   S(67, -10),   S(115, -5),  S(40, -17),
      S(71, -29),   S(-14, 10),   S(48, -2),    S(71, 21),   S(90, 15),
      S(96, 13),    S(122, -3),   S(70, -17),   S(28, -12),  S(9, 10),
      S(9, 23),     S(33, 45),    S(72, 41),    S(37, 43),   S(76, 36),
      S(16, 22),    S(55, -6),    S(-6, 21),    S(10, 11),   S(18, 46),
      S(29, 36),    S(40, 49),    S(32, 34),    S(54, 10),   S(20, 10),
      S(-23, -15),  S(-16, 7),    S(2, 11),     S(14, 27),   S(30, 35),
      S(9, 3),      S(13, -5),    S(-3, -11),   S(-49, -2),  S(-24, 9),
      S(-10, -4),   S(2, 3),      S(7, 2),      S(15, -6),   S(6, -30),
      S(-4, -8),    S(-102, -25), S(-22, -23),  S(-33, -8),  S(-17, 0),
      S(-4, -12),   S(-6, -17),   S(-15, -4),   S(-45, -24),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(10, 10),  S(-82, 19), S(-24, 7), S(-104, 15), S(-61, -6), S(-85, 4),
      S(-27, 9),  S(-42, -2), S(-7, -7), S(29, -4),   S(7, -9),   S(-15, 12),
      S(28, -12), S(10, -6),  S(32, 1),  S(-3, -13),  S(-3, 14),  S(32, 7),
      S(40, 7),   S(38, 3),   S(35, 3),  S(55, 8),    S(45, 3),   S(15, 5),
      S(-15, 2),  S(10, 12),  S(17, 27), S(40, 33),   S(32, 31),  S(30, 24),
      S(14, 18),  S(-15, 11), S(-10, 3), S(-16, 26),  S(1, 29),   S(35, 26),
      S(28, 33),  S(10, 17),  S(0, 26),  S(25, -13),  S(5, 15),   S(16, 18),
      S(13, 23),  S(18, 21),  S(28, 29), S(21, 23),   S(20, 17),  S(22, 1),
      S(16, 4),   S(16, 0),   S(30, 0),  S(5, 19),    S(15, 12),  S(34, 6),
      S(38, 2),   S(18, 9),   S(-1, -5), S(26, 24),   S(12, 18),  S(-2, 14),
      S(5, 12),   S(7, 23),   S(36, -3), S(24, -9),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(20, 38),  S(42, 18),  S(12, 54),  S(18, 36),  S(49, 27),  S(47, 37),
      S(67, 27),  S(89, 20),  S(7, 30),   S(-4, 40),  S(20, 45),  S(53, 25),
      S(18, 27),  S(55, 19),  S(35, 14),  S(43, 23),  S(4, 29),   S(37, 26),
      S(30, 28),  S(37, 34),  S(71, 4),   S(71, 4),   S(119, 2),  S(71, 12),
      S(-14, 34), S(27, 21),  S(9, 36),   S(19, 32),  S(27, 7),   S(23, 8),
      S(32, 17),  S(28, 0),   S(-33, 20), S(-28, 26), S(-12, 35), S(-6, 29),
      S(1, 19),   S(-33, 22), S(0, 14),   S(-13, 17), S(-28, 19), S(-27, 28),
      S(-18, 22), S(-13, 19), S(-9, 21),  S(-15, 1),  S(27, -13), S(-4, -8),
      S(-46, 17), S(-18, 10), S(-6, 17),  S(-6, 14),  S(-2, 7),   S(6, -7),
      S(24, -8),  S(-27, 3),  S(-16, 17), S(-12, 18), S(0, 19),   S(9, 16),
      S(14, 4),   S(0, 6),    S(6, 0),    S(-15, 0),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-68, 66),  S(-40, 23), S(-37, 63), S(3, 66),   S(14, 39),  S(27, 44),
      S(90, -6),   S(6, 49),   S(-17, 36), S(-36, 41), S(-20, 81), S(-48, 122),
      S(-43, 134), S(15, 45),  S(-4, 45),  S(53, 40),  S(-2, 40),  S(-12, 49),
      S(-1, 75),   S(-21, 93), S(19, 74),  S(56, 50),  S(69, 4),   S(21, 64),
      S(-20, 50),  S(-5, 48),  S(-13, 54), S(-19, 89), S(1, 75),   S(-5, 97),
      S(7, 78),    S(12, 46),  S(-8, 17),  S(-20, 46), S(-23, 67), S(-16, 85),
      S(-17, 95),  S(-16, 87), S(2, 53),   S(8, 65),   S(-9, 18),  S(-6, 35),
      S(-5, 46),   S(-8, 57),  S(-9, 85),  S(10, 33),  S(11, 32),  S(10, 35),
      S(-6, 1),    S(-9, 3),   S(8, 7),    S(21, 11),  S(14, 31),  S(24, -7),
      S(16, -20),  S(37, -62), S(-5, 6),   S(-2, -1),  S(9, -5),   S(21, 11),
      S(9, 9),     S(-10, 12), S(-3, -25), S(-2, -11),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-58, -107), S(31, -67),  S(29, -37),  S(-51, -15), S(-56, -5),
      S(-14, -6),   S(50, 16),   S(29, -125), S(-56, -20), S(-23, 15),
      S(-93, 30),   S(16, 23),   S(5, 28),    S(-15, 50),  S(95, 40),
      S(61, -14),   S(-134, 9),  S(-18, 19),  S(-49, 41),  S(-50, 66),
      S(-35, 74),   S(43, 64),   S(60, 56),   S(18, 4),    S(-48, -16),
      S(-36, 29),   S(-107, 45), S(-192, 67), S(-169, 67), S(-126, 60),
      S(-111, 49),  S(-125, 4),  S(-149, -9), S(-88, 14),  S(-149, 43),
      S(-186, 62),  S(-214, 69), S(-136, 41), S(-140, 27), S(-193, 14),
      S(-51, -31),  S(-52, -4),  S(-89, 22),  S(-141, 40), S(-105, 38),
      S(-109, 29),  S(-46, 0),   S(-85, -10), S(57, -49),  S(10, -15),
      S(-14, 0),    S(-54, 11),  S(-57, 19),  S(-34, 6),   S(31, -17),
      S(30, -43),   S(53, -113), S(73, -69),  S(38, -47),  S(-77, -20),
      S(-3, -36),   S(-40, -25), S(48, -60),  S(42, -106),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-2, 5),   S(-3, 13),  S(-5, 45),
      S(2, 89), S(-3, 167), S(72, 189), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-5, -29),  S(-5, -22),  S(-1, -34),
      S(15, -62),  S(-61, -83), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-16, -8), S(-23, -15), S(-18, -12),
      S(-3, -27), S(7, -32),  S(23, -18),  S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-18, -13), S(-3, -6),   S(-8, -4),
      S(-19, -5),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-28, -41), S(-15, -7), S(-5, 3), S(4, 8),   S(13, 22),
      S(25, 19),   S(35, 19),  S(41, 1), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-29, -74), S(-7, -46), S(0, -21), S(11, -8), S(18, 5),
      S(23, 15),   S(24, 19),  S(29, 24), S(30, 22), S(34, 20),
      S(39, 14),   S(9, 35),   S(37, -7), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-42, -59), S(-35, -46), S(-30, -34), S(-26, -26), S(-25, -21),
      S(-19, -7),  S(-12, -5),  S(-4, -2),   S(6, 5),     S(7, 9),
      S(16, 14),   S(14, 26),   S(0, 30),    S(-18, 35),  S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-119, -155), S(-53, -93), S(-30, -81), S(-19, -54), S(-15, -59),
      S(-4, -80),    S(-2, -39),  S(9, -43),   S(14, -47),  S(16, -22),
      S(20, -22),    S(24, -19),  S(27, -11),  S(23, 2),    S(29, 5),
      S(17, 25),     S(15, 34),   S(14, 38),   S(18, 56),   S(1, 63),
      S(39, 36),     S(37, 55),   S(11, 38),   S(73, 49),   S(32, 33),
      S(55, 102),    S(47, 123),  S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-19, 19), S(-12, 4), S(-24, 5), S(-37, 8), S(-19, -18),
};
constexpr Score trappedRookWeight = {
      S(-4, -6),
};
constexpr Score rookOnOpenFile[2] = {
      S(16, 1),
      S(30, 11),
};
constexpr Score openFileNextToKing[3] = {S(10, 30), S(-20, -15), S(-90, -100)};
constexpr Score pawnShieldStrength[4] = {S(-100, -100), S(-50, -50), S(-5, -5),
                                         S(100, 100)};

constexpr Score pushedPawnShieldStrength[4] = {S(0, 0), S(-50, -50), S(-5, -5),
                                               S(100, 100)};

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
    int openKingFiles[3][NUM_COLOR];
    int pawnShieldStrength[4][NUM_COLOR];
    int pushedPawnShieldStrength[4][NUM_COLOR];
};

struct EvalWeights {
    const Score pawnScore = S(93, 134);
    const Score knightScore = S(382, 467);
    const Score bishopScore = S(417, 490);
    const Score rookScore = S(551, 866);
    const Score queenScore = S(1158, 1561);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(27, 107), S(22, 90),
          S(-17, 99),   S(30, 39),  S(-32, 56), S(-16, 37), S(-141, 123),
          S(-157, 143), S(0, 94),   S(3, 101),  S(33, 44),  S(41, -5),
          S(56, -15),   S(98, 14),  S(64, 70),  S(39, 70),  S(-19, 61),
          S(-13, 43),   S(3, 20),   S(10, -10), S(35, -3),  S(34, 10),
          S(18, 30),    S(14, 25),  S(-35, 33), S(-28, 25), S(-13, 10),
          S(4, -7),     S(1, -2),   S(5, 6),    S(-3, 9),   S(-7, 5),
          S(-29, 23),   S(-22, 19), S(-11, 11), S(-6, 11),  S(13, 12),
          S(6, 14),     S(25, 5),   S(6, -2),   S(-25, 26), S(-20, 25),
          S(-9, 13),    S(-7, 16),  S(5, 15),   S(35, 10),  S(40, 2),
          S(1, -2),     S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-165, -77), S(-129, -44), S(-53, -11),  S(1, -20),   S(24, -6),
          S(-81, -10),  S(-74, -23),  S(-93, -119), S(-27, -18), S(16, 3),
          S(59, -15),   S(78, -10),   S(67, -10),   S(115, -5),  S(40, -17),
          S(71, -29),   S(-14, 10),   S(48, -2),    S(71, 21),   S(90, 15),
          S(96, 13),    S(122, -3),   S(70, -17),   S(28, -12),  S(9, 10),
          S(9, 23),     S(33, 45),    S(72, 41),    S(37, 43),   S(76, 36),
          S(16, 22),    S(55, -6),    S(-6, 21),    S(10, 11),   S(18, 46),
          S(29, 36),    S(40, 49),    S(32, 34),    S(54, 10),   S(20, 10),
          S(-23, -15),  S(-16, 7),    S(2, 11),     S(14, 27),   S(30, 35),
          S(9, 3),      S(13, -5),    S(-3, -11),   S(-49, -2),  S(-24, 9),
          S(-10, -4),   S(2, 3),      S(7, 2),      S(15, -6),   S(6, -30),
          S(-4, -8),    S(-102, -25), S(-22, -23),  S(-33, -8),  S(-17, 0),
          S(-4, -12),   S(-6, -17),   S(-15, -4),   S(-45, -24),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(10, 10),  S(-82, 19), S(-24, 7),  S(-104, 15), S(-61, -6),
          S(-85, 4),  S(-27, 9),  S(-42, -2), S(-7, -7),   S(29, -4),
          S(7, -9),   S(-15, 12), S(28, -12), S(10, -6),   S(32, 1),
          S(-3, -13), S(-3, 14),  S(32, 7),   S(40, 7),    S(38, 3),
          S(35, 3),   S(55, 8),   S(45, 3),   S(15, 5),    S(-15, 2),
          S(10, 12),  S(17, 27),  S(40, 33),  S(32, 31),   S(30, 24),
          S(14, 18),  S(-15, 11), S(-10, 3),  S(-16, 26),  S(1, 29),
          S(35, 26),  S(28, 33),  S(10, 17),  S(0, 26),    S(25, -13),
          S(5, 15),   S(16, 18),  S(13, 23),  S(18, 21),   S(28, 29),
          S(21, 23),  S(20, 17),  S(22, 1),   S(16, 4),    S(16, 0),
          S(30, 0),   S(5, 19),   S(15, 12),  S(34, 6),    S(38, 2),
          S(18, 9),   S(-1, -5),  S(26, 24),  S(12, 18),   S(-2, 14),
          S(5, 12),   S(7, 23),   S(36, -3),  S(24, -9),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(20, 38),  S(42, 18),  S(12, 54),  S(18, 36),  S(49, 27),
          S(47, 37),  S(67, 27),  S(89, 20),  S(7, 30),   S(-4, 40),
          S(20, 45),  S(53, 25),  S(18, 27),  S(55, 19),  S(35, 14),
          S(43, 23),  S(4, 29),   S(37, 26),  S(30, 28),  S(37, 34),
          S(71, 4),   S(71, 4),   S(119, 2),  S(71, 12),  S(-14, 34),
          S(27, 21),  S(9, 36),   S(19, 32),  S(27, 7),   S(23, 8),
          S(32, 17),  S(28, 0),   S(-33, 20), S(-28, 26), S(-12, 35),
          S(-6, 29),  S(1, 19),   S(-33, 22), S(0, 14),   S(-13, 17),
          S(-28, 19), S(-27, 28), S(-18, 22), S(-13, 19), S(-9, 21),
          S(-15, 1),  S(27, -13), S(-4, -8),  S(-46, 17), S(-18, 10),
          S(-6, 17),  S(-6, 14),  S(-2, 7),   S(6, -7),   S(24, -8),
          S(-27, 3),  S(-16, 17), S(-12, 18), S(0, 19),   S(9, 16),
          S(14, 4),   S(0, 6),    S(6, 0),    S(-15, 0),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-68, 66), S(-40, 23),  S(-37, 63),  S(3, 66),   S(14, 39),
          S(27, 44),  S(90, -6),   S(6, 49),    S(-17, 36), S(-36, 41),
          S(-20, 81), S(-48, 122), S(-43, 134), S(15, 45),  S(-4, 45),
          S(53, 40),  S(-2, 40),   S(-12, 49),  S(-1, 75),  S(-21, 93),
          S(19, 74),  S(56, 50),   S(69, 4),    S(21, 64),  S(-20, 50),
          S(-5, 48),  S(-13, 54),  S(-19, 89),  S(1, 75),   S(-5, 97),
          S(7, 78),   S(12, 46),   S(-8, 17),   S(-20, 46), S(-23, 67),
          S(-16, 85), S(-17, 95),  S(-16, 87),  S(2, 53),   S(8, 65),
          S(-9, 18),  S(-6, 35),   S(-5, 46),   S(-8, 57),  S(-9, 85),
          S(10, 33),  S(11, 32),   S(10, 35),   S(-6, 1),   S(-9, 3),
          S(8, 7),    S(21, 11),   S(14, 31),   S(24, -7),  S(16, -20),
          S(37, -62), S(-5, 6),    S(-2, -1),   S(9, -5),   S(21, 11),
          S(9, 9),    S(-10, 12),  S(-3, -25),  S(-2, -11),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-58, -107), S(31, -67),  S(29, -37),  S(-51, -15), S(-56, -5),
          S(-14, -6),   S(50, 16),   S(29, -125), S(-56, -20), S(-23, 15),
          S(-93, 30),   S(16, 23),   S(5, 28),    S(-15, 50),  S(95, 40),
          S(61, -14),   S(-134, 9),  S(-18, 19),  S(-49, 41),  S(-50, 66),
          S(-35, 74),   S(43, 64),   S(60, 56),   S(18, 4),    S(-48, -16),
          S(-36, 29),   S(-107, 45), S(-192, 67), S(-169, 67), S(-126, 60),
          S(-111, 49),  S(-125, 4),  S(-149, -9), S(-88, 14),  S(-149, 43),
          S(-186, 62),  S(-214, 69), S(-136, 41), S(-140, 27), S(-193, 14),
          S(-51, -31),  S(-52, -4),  S(-89, 22),  S(-141, 40), S(-105, 38),
          S(-109, 29),  S(-46, 0),   S(-85, -10), S(57, -49),  S(10, -15),
          S(-14, 0),    S(-54, 11),  S(-57, 19),  S(-34, 6),   S(31, -17),
          S(30, -43),   S(53, -113), S(73, -69),  S(38, -47),  S(-77, -20),
          S(-3, -36),   S(-40, -25), S(48, -60),  S(42, -106),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-2, 5),   S(-3, 13),  S(-5, 45),
          S(2, 89), S(-3, 167), S(72, 189), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-5, -29),  S(-5, -22),  S(-1, -34),
          S(15, -62),  S(-61, -83), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-16, -8), S(-23, -15), S(-18, -12),
          S(-3, -27), S(7, -32),  S(23, -18),  S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-18, -13), S(-3, -6),   S(-8, -4),
          S(-19, -5),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-28, -41), S(-15, -7), S(-5, 3), S(4, 8),   S(13, 22),
          S(25, 19),   S(35, 19),  S(41, 1), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-29, -74), S(-7, -46), S(0, -21), S(11, -8), S(18, 5),
          S(23, 15),   S(24, 19),  S(29, 24), S(30, 22), S(34, 20),
          S(39, 14),   S(9, 35),   S(37, -7), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-42, -59), S(-35, -46), S(-30, -34), S(-26, -26), S(-25, -21),
          S(-19, -7),  S(-12, -5),  S(-4, -2),   S(6, 5),     S(7, 9),
          S(16, 14),   S(14, 26),   S(0, 30),    S(-18, 35),  S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-119, -155), S(-53, -93), S(-30, -81), S(-19, -54), S(-15, -59),
          S(-4, -80),    S(-2, -39),  S(9, -43),   S(14, -47),  S(16, -22),
          S(20, -22),    S(24, -19),  S(27, -11),  S(23, 2),    S(29, 5),
          S(17, 25),     S(15, 34),   S(14, 38),   S(18, 56),   S(1, 63),
          S(39, 36),     S(37, 55),   S(11, 38),   S(73, 49),   S(32, 33),
          S(55, 102),    S(47, 123),  S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-19, 19), S(-12, 4), S(-24, 5), S(-37, 8), S(-19, -18),
    };
    const Score trappedRookWeight = {
          S(-4, -6),
    };
    const Score rookOnOpenFile[2] = {
          S(16, 1),
          S(30, 11),
    };
    const Score openFileNextToKing[3] = {S(10, 30), S(-20, -15), S(-90, -100)};
    const Score pawnShieldStrength[4] = {S(-100, -100), S(-50, -50), S(-5, -5),
                                         S(100, 100)};
    const Score pushedPawnShieldStrength[4] = {S(0, 0), S(-50, -50), S(-5, -5),
                                               S(100, 100)};
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

extern std::unordered_map<std::uint64_t, int> pawnTable;

enum Tracing : bool { NO_TRACE, TRACE };

template <Tracing T = NO_TRACE> class Eval {
  public:
    Eval(Board &b) : board(b), trace(tempTrace) { init(); };
    Eval(Board &b, Trace &t) : board(b), trace(t) { init(); }

    int eval() {
        if (!T) {
            evalHashTable.prefetch(board.key);

            HashTable::EvalHash entry = {0};
            if (evalHashTable.probe(board.key, entry)) {
                // return entry.score();
            }
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

        const Score wKS = kingSafety<WHITE>();
        const Score bKS = kingSafety<BLACK>();
        const int mgKS = MgScore(wKS) - MgScore(bKS);
        const int egKS = EgScore(wKS) - EgScore(bKS);

        const Score wRookEval = rookEval<WHITE>();
        const Score bRookEval = rookEval<BLACK>();
        const int mgRookEval = MgScore(wRookEval) - MgScore(bRookEval);
        const int egRookEval = EgScore(wRookEval) - EgScore(bRookEval);

        pawnHashTable.prefetch(board.pawnKey);
        const auto color = (board.turn == WHITE) ? 1 : -1;
        const auto materialScore = materialEval();
        const auto pawnStructureEval = pawnStructure();

        const int pcSqEval = (mgPcSq * mgPhase + egPcSq * egPhase) / 24;
        const int mobilityEval =
              (mgMobility * mgPhase + egMobility * egPhase) / 24;
        const int safetyEval = (mgSafety * mgPhase + egSafety * egPhase) / 24;
        const int KSEval = (mgKS * mgPhase + egKS * egPhase) / 24;
        const int rookEval = (mgRookEval * mgPhase + egRookEval * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + pawnStructureEval + mobilityEval +
                rookEval + safetyEval + KSEval;

        if (!T) {
            evalHashTable.record(board.key, eval * color);
        }

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
    template <Color C> constexpr Score kingSafety();
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

        if (popcount(getRookAttacks(rookSquare, board.pieces())) > 3) {
            temp &= temp - 1;
            continue;
        }

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

    return S(mgScore, egScore);
}

template <Tracing T> template <Color C> constexpr Score Eval<T>::kingSafety() {
    constexpr Direction Up = C == WHITE ? NORTH : SOUTH;
    Bitboard ourKing = board.pieces(KING, C);
    Square kingSquare = Sq(ourKing);

    int mgScore = 0, egScore = 0;
    if (FILE_OF(kingSquare) < 3 || FILE_OF(kingSquare) >= 5) {
        Bitboard shieldMask =
              FILE_OF(kingSquare) < 3
                    ? (SQUARE_BB(A2) | SQUARE_BB(B2) | SQUARE_BB(C2))
                    : (SQUARE_BB(F2) | SQUARE_BB(G2) | SQUARE_BB(H2));

        if (C)
            shieldMask >>= 8 * 5;

        Bitboard pawnShield = board.pieces(PAWN, C) & shieldMask;
        mgScore += MgScore(pawnShieldStrength[popcount(pawnShield)]);
        egScore += EgScore(pawnShieldStrength[popcount(pawnShield)]);

        shieldMask = shift<Up>(shieldMask);
        Bitboard pushedPawnShield = board.pieces(PAWN, C) & pushedPawnShield;
        mgScore +=
              MgScore(pushedPawnShieldStrength[popcount(pushedPawnShield)]);
        egScore +=
              EgScore(pushedPawnShieldStrength[popcount(pushedPawnShield)]);

        if (T) {
            trace.pushedPawnShieldStrength[popcount(pushedPawnShield)][C]++;
            trace.pawnShieldStrength[popcount(pawnShield)][C]++;
        }
    }

    int emptyFiles = 0;
    Bitboard rightShift = shift<EAST>(ourKing);
    Bitboard leftShift = shift<WEST>(ourKing);
    Bitboard rightFile = FILE_BB(FILE_OF(Sq(rightShift)));
    Bitboard leftFile = FILE_BB(FILE_OF(Sq(leftShift)));

    if (!(board.pieces(PAWN) & rightFile))
        emptyFiles++;

    if (!(board.pieces(PAWN) & leftFile))
        emptyFiles++;

    mgScore += MgScore(openFileNextToKing[emptyFiles]);
    egScore += EgScore(openFileNextToKing[emptyFiles]);

    if (T) {
        trace.openKingFiles[emptyFiles][C]++;
    }

    return S(mgScore, egScore);
}

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::kingAttackers() {

    const Bitboard king = board.pieces(KING, C);
    const Square kingSquare = Sq(king);
    Bitboard kingArea = kingAttacks[kingSquare];

    int mgScore = 0, egScore = 0;
    while (kingArea) {
        Square areaSq = Square(lsb_index(kingArea));
        Bitboard attackPcs = board.attacksToKing<~C>(areaSq, board.pieces());

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

    return S(mgScore, egScore);
}

template <Tracing T> constexpr int Eval<T>::pawnStructure() {
    if (!T) {
        HashTable::EvalHash entry = {0};
        if (pawnHashTable.probe(board.pawnKey, entry)) {
            // return entry.score();
        }
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

    pawnHashTable.record(board.pawnKey, pawnStructure);

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
        Piece &p = board.board[i];
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

    return S(mgScore, egScore);
}

} // namespace Yayo
#endif // SEARCH_H_
