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

constexpr Score pawnScore = S(95, 143);
constexpr Score knightScore = S(397, 485);
constexpr Score bishopScore = S(438, 510);
constexpr Score rookScore = S(578, 905);
constexpr Score queenScore = S(1191, 1617);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(26, 113), S(23, 92),
      S(-12, 104),  S(31, 39),  S(-32, 55), S(-16, 40), S(-141, 126),
      S(-163, 148), S(0, 98),   S(3, 104),  S(39, 47),  S(43, -8),
      S(54, -13),   S(103, 15), S(64, 72),  S(32, 72),  S(-20, 65),
      S(-13, 48),   S(4, 22),   S(10, -10), S(37, -4),  S(40, 10),
      S(17, 31),    S(17, 25),  S(-36, 33), S(-26, 28), S(-11, 10),
      S(4, -4),     S(6, -2),   S(9, 7),    S(2, 11),   S(-3, 5),
      S(-28, 24),   S(-19, 22), S(-9, 13),  S(-1, 13),  S(17, 13),
      S(8, 14),     S(33, 7),   S(14, -2),  S(-26, 25), S(-20, 24),
      S(-12, 10),   S(-4, 16),  S(8, 20),   S(16, 5),   S(28, -2),
      S(-18, -5),   S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-165, -75), S(-130, -39), S(-53, -8),   S(1, -20),   S(24, -4),
      S(-81, -11),  S(-77, -20),  S(-92, -117), S(-23, -11), S(17, 2),
      S(59, -14),   S(78, -9),    S(68, -11),   S(113, -12), S(38, -14),
      S(71, -30),   S(-12, 10),   S(47, -1),    S(72, 22),   S(88, 19),
      S(99, 16),    S(122, -2),   S(69, -16),   S(31, -12),  S(10, 10),
      S(9, 25),     S(39, 48),    S(79, 45),    S(38, 46),   S(77, 40),
      S(13, 24),    S(55, -5),    S(-3, 21),    S(11, 13),   S(20, 49),
      S(32, 40),    S(42, 52),    S(36, 35),    S(55, 11),   S(22, 12),
      S(-25, -14),  S(-17, 6),    S(2, 11),     S(11, 30),   S(31, 35),
      S(12, 5),     S(15, -4),    S(1, -7),     S(-47, -3),  S(-25, 9),
      S(-13, -3),   S(2, 5),      S(11, 2),     S(14, -5),   S(5, -25),
      S(-1, -5),    S(-100, -22), S(-21, -24),  S(-33, -8),  S(-16, 0),
      S(-4, -9),    S(-4, -15),   S(-16, -4),   S(-44, -19),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(6, 10),   S(-80, 19), S(-28, 7), S(-104, 18), S(-62, -3), S(-85, 7),
      S(-28, 9),  S(-42, -2), S(-8, -7), S(29, -4),   S(10, -4),  S(-15, 11),
      S(27, -11), S(9, -3),   S(29, 2),  S(-8, -12),  S(-3, 19),  S(32, 7),
      S(39, 11),  S(37, 4),   S(34, 4),  S(56, 12),   S(40, 7),   S(11, 10),
      S(-14, 8),  S(13, 15),  S(17, 25), S(41, 36),   S(33, 33),  S(32, 26),
      S(14, 20),  S(-16, 12), S(-6, 4),  S(-13, 27),  S(3, 31),   S(38, 30),
      S(28, 33),  S(11, 23),  S(-1, 26), S(27, -11),  S(0, 15),   S(20, 18),
      S(17, 23),  S(17, 25),  S(30, 31), S(21, 23),   S(24, 17),  S(27, 2),
      S(17, 9),   S(18, 2),   S(32, 0),  S(6, 18),    S(15, 16),  S(36, 8),
      S(46, 6),   S(21, 9),   S(-1, -5), S(31, 24),   S(13, 17),  S(-3, 14),
      S(5, 10),   S(8, 25),   S(34, -4), S(27, -9),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(20, 38),  S(42, 26),  S(8, 57),   S(16, 39),  S(47, 27),  S(47, 36),
      S(65, 25),  S(89, 20),  S(5, 32),   S(-4, 46),  S(17, 51),  S(51, 29),
      S(18, 32),  S(55, 23),  S(36, 18),  S(43, 23),  S(4, 30),   S(36, 32),
      S(30, 30),  S(37, 33),  S(73, 11),  S(72, 4),   S(119, 4),  S(69, 9),
      S(-13, 38), S(24, 23),  S(11, 43),  S(21, 34),  S(30, 12),  S(26, 10),
      S(32, 17),  S(28, 5),   S(-33, 25), S(-28, 30), S(-14, 36), S(-6, 29),
      S(0, 22),   S(-33, 25), S(0, 18),   S(-16, 17), S(-31, 22), S(-29, 28),
      S(-18, 19), S(-14, 20), S(-9, 21),  S(-12, 6),  S(27, -14), S(-6, -8),
      S(-46, 17), S(-21, 13), S(-5, 18),  S(-4, 16),  S(0, 7),    S(10, -4),
      S(30, -9),  S(-30, 0),  S(-15, 17), S(-13, 17), S(2, 19),   S(9, 17),
      S(16, 5),   S(3, 6),    S(5, 0),    S(-11, 4),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-68, 67),  S(-40, 29), S(-37, 70), S(2, 66),   S(11, 42),  S(23, 44),
      S(90, -9),   S(6, 49),   S(-18, 36), S(-37, 43), S(-23, 81), S(-48, 122),
      S(-48, 134), S(14, 48),  S(-3, 46),  S(57, 45),  S(-3, 41),  S(-14, 51),
      S(-1, 78),   S(-17, 99), S(18, 75),  S(53, 49),  S(67, 6),   S(20, 65),
      S(-19, 54),  S(-11, 51), S(-12, 60), S(-19, 93), S(4, 80),   S(-4, 97),
      S(10, 81),   S(9, 48),   S(-8, 21),  S(-20, 52), S(-23, 73), S(-13, 89),
      S(-17, 99),  S(-16, 89), S(2, 57),   S(8, 65),   S(-12, 18), S(-5, 39),
      S(-5, 48),   S(-11, 60), S(-7, 91),  S(11, 39),  S(15, 36),  S(9, 35),
      S(-6, 2),    S(-10, 7),  S(8, 11),   S(21, 17),  S(14, 35),  S(27, -6),
      S(20, -15),  S(38, -60), S(-9, 4),   S(-3, -1),  S(10, -4),  S(22, 18),
      S(13, 11),   S(-11, 12), S(-1, -25), S(-2, -9),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-56, -107), S(31, -65),  S(32, -36),   S(-54, -14), S(-60, -13),
      S(-13, -7),   S(49, 14),   S(35, -127),  S(-56, -19), S(-19, 20),
      S(-91, 38),   S(17, 20),   S(1, 22),     S(-15, 55),  S(97, 41),
      S(62, -11),   S(-136, 10), S(-9, 28),    S(-46, 54),  S(-57, 61),
      S(-42, 67),   S(46, 69),   S(61, 56),    S(21, 8),    S(-49, -10),
      S(-34, 35),   S(-100, 60), S(-198, 67),  S(-175, 65), S(-126, 64),
      S(-111, 52),  S(-126, 13), S(-146, -10), S(-84, 20),  S(-146, 53),
      S(-194, 61),  S(-218, 63), S(-132, 44),  S(-137, 28), S(-194, 17),
      S(-51, -29),  S(-42, 3),   S(-85, 29),   S(-142, 37), S(-113, 30),
      S(-102, 30),  S(-39, 0),   S(-86, -10),  S(59, -49),  S(16, -12),
      S(-8, 5),     S(-62, 5),   S(-63, 9),    S(-28, 7),   S(38, -17),
      S(40, -45),   S(44, -112), S(67, -74),   S(27, -45),  S(-85, -30),
      S(0, -45),    S(-43, -25), S(48, -64),   S(40, -109),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-4, 5),  S(-3, 14),  S(-4, 51),
      S(6, 94), S(1, 177), S(72, 199), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-4, -31),  S(-5, -24),  S(3, -36),
      S(16, -63),  S(-64, -84), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-18, -5), S(-24, -17), S(-20, -14),
      S(-3, -28), S(7, -32),  S(23, -11),  S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-18, -13), S(-3, -6),   S(-9, -3),
      S(-19, -7),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-36, -44), S(-16, -6), S(-4, 4), S(7, 12),  S(17, 29),
      S(31, 25),   S(42, 26),  S(49, 8), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-29, -76), S(-7, -44), S(1, -21), S(15, -5), S(22, 10),
      S(27, 19),   S(30, 24),  S(32, 31), S(35, 28), S(40, 23),
      S(45, 20),   S(11, 35),  S(40, -2), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-47, -63), S(-34, -46), S(-33, -35), S(-26, -26), S(-26, -19),
      S(-18, -5),  S(-9, -1),   S(-1, 5),    S(12, 10),   S(15, 15),
      S(22, 21),   S(22, 32),   S(13, 33),   S(-8, 40),   S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-128, -165), S(-55, -101), S(-35, -90), S(-26, -62), S(-17, -65),
      S(-6, -80),    S(-3, -39),   S(6, -39),   S(13, -38),  S(16, -16),
      S(21, -16),    S(25, -10),   S(28, -1),   S(25, 8),    S(32, 13),
      S(23, 32),     S(21, 42),    S(21, 45),   S(25, 57),   S(11, 67),
      S(46, 39),     S(46, 56),    S(20, 41),   S(74, 47),   S(37, 35),
      S(49, 94),     S(42, 116),   S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-22, 21), S(-15, 5), S(-29, 6), S(-36, 8), S(-19, -24),
};
constexpr Score trappedRookWeight = {
      S(-5, -1),
};
constexpr Score rookOnOpenFile[2] = {
      S(16, 1),
      S(31, 12),
};
constexpr Score openFileNextToKing[3] = {
      S(-1, -19),
      S(-17, -15),
      S(-58, -38),
};
constexpr Score pawnShieldStrength[4] = {
      S(-76, -31),
      S(-26, -32),
      S(14, -32),
      S(39, -25),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(4, 27),
      S(-50, -50),
      S(-5, -5),
      S(100, 100),
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
    int openKingFiles[3][NUM_COLOR];
    int pawnShieldStrength[4][NUM_COLOR];
    int pushedPawnShieldStrength[4][NUM_COLOR];
};

struct EvalWeights {
    const Score pawnScore = S(95, 143);
    const Score knightScore = S(397, 485);
    const Score bishopScore = S(438, 510);
    const Score rookScore = S(578, 905);
    const Score queenScore = S(1191, 1617);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(26, 113), S(23, 92),
          S(-12, 104),  S(31, 39),  S(-32, 55), S(-16, 40), S(-141, 126),
          S(-163, 148), S(0, 98),   S(3, 104),  S(39, 47),  S(43, -8),
          S(54, -13),   S(103, 15), S(64, 72),  S(32, 72),  S(-20, 65),
          S(-13, 48),   S(4, 22),   S(10, -10), S(37, -4),  S(40, 10),
          S(17, 31),    S(17, 25),  S(-36, 33), S(-26, 28), S(-11, 10),
          S(4, -4),     S(6, -2),   S(9, 7),    S(2, 11),   S(-3, 5),
          S(-28, 24),   S(-19, 22), S(-9, 13),  S(-1, 13),  S(17, 13),
          S(8, 14),     S(33, 7),   S(14, -2),  S(-26, 25), S(-20, 24),
          S(-12, 10),   S(-4, 16),  S(8, 20),   S(16, 5),   S(28, -2),
          S(-18, -5),   S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-165, -75), S(-130, -39), S(-53, -8),   S(1, -20),   S(24, -4),
          S(-81, -11),  S(-77, -20),  S(-92, -117), S(-23, -11), S(17, 2),
          S(59, -14),   S(78, -9),    S(68, -11),   S(113, -12), S(38, -14),
          S(71, -30),   S(-12, 10),   S(47, -1),    S(72, 22),   S(88, 19),
          S(99, 16),    S(122, -2),   S(69, -16),   S(31, -12),  S(10, 10),
          S(9, 25),     S(39, 48),    S(79, 45),    S(38, 46),   S(77, 40),
          S(13, 24),    S(55, -5),    S(-3, 21),    S(11, 13),   S(20, 49),
          S(32, 40),    S(42, 52),    S(36, 35),    S(55, 11),   S(22, 12),
          S(-25, -14),  S(-17, 6),    S(2, 11),     S(11, 30),   S(31, 35),
          S(12, 5),     S(15, -4),    S(1, -7),     S(-47, -3),  S(-25, 9),
          S(-13, -3),   S(2, 5),      S(11, 2),     S(14, -5),   S(5, -25),
          S(-1, -5),    S(-100, -22), S(-21, -24),  S(-33, -8),  S(-16, 0),
          S(-4, -9),    S(-4, -15),   S(-16, -4),   S(-44, -19),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(6, 10),   S(-80, 19), S(-28, 7),  S(-104, 18), S(-62, -3),
          S(-85, 7),  S(-28, 9),  S(-42, -2), S(-8, -7),   S(29, -4),
          S(10, -4),  S(-15, 11), S(27, -11), S(9, -3),    S(29, 2),
          S(-8, -12), S(-3, 19),  S(32, 7),   S(39, 11),   S(37, 4),
          S(34, 4),   S(56, 12),  S(40, 7),   S(11, 10),   S(-14, 8),
          S(13, 15),  S(17, 25),  S(41, 36),  S(33, 33),   S(32, 26),
          S(14, 20),  S(-16, 12), S(-6, 4),   S(-13, 27),  S(3, 31),
          S(38, 30),  S(28, 33),  S(11, 23),  S(-1, 26),   S(27, -11),
          S(0, 15),   S(20, 18),  S(17, 23),  S(17, 25),   S(30, 31),
          S(21, 23),  S(24, 17),  S(27, 2),   S(17, 9),    S(18, 2),
          S(32, 0),   S(6, 18),   S(15, 16),  S(36, 8),    S(46, 6),
          S(21, 9),   S(-1, -5),  S(31, 24),  S(13, 17),   S(-3, 14),
          S(5, 10),   S(8, 25),   S(34, -4),  S(27, -9),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(20, 38),  S(42, 26),  S(8, 57),   S(16, 39),  S(47, 27),
          S(47, 36),  S(65, 25),  S(89, 20),  S(5, 32),   S(-4, 46),
          S(17, 51),  S(51, 29),  S(18, 32),  S(55, 23),  S(36, 18),
          S(43, 23),  S(4, 30),   S(36, 32),  S(30, 30),  S(37, 33),
          S(73, 11),  S(72, 4),   S(119, 4),  S(69, 9),   S(-13, 38),
          S(24, 23),  S(11, 43),  S(21, 34),  S(30, 12),  S(26, 10),
          S(32, 17),  S(28, 5),   S(-33, 25), S(-28, 30), S(-14, 36),
          S(-6, 29),  S(0, 22),   S(-33, 25), S(0, 18),   S(-16, 17),
          S(-31, 22), S(-29, 28), S(-18, 19), S(-14, 20), S(-9, 21),
          S(-12, 6),  S(27, -14), S(-6, -8),  S(-46, 17), S(-21, 13),
          S(-5, 18),  S(-4, 16),  S(0, 7),    S(10, -4),  S(30, -9),
          S(-30, 0),  S(-15, 17), S(-13, 17), S(2, 19),   S(9, 17),
          S(16, 5),   S(3, 6),    S(5, 0),    S(-11, 4),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-68, 67), S(-40, 29),  S(-37, 70),  S(2, 66),   S(11, 42),
          S(23, 44),  S(90, -9),   S(6, 49),    S(-18, 36), S(-37, 43),
          S(-23, 81), S(-48, 122), S(-48, 134), S(14, 48),  S(-3, 46),
          S(57, 45),  S(-3, 41),   S(-14, 51),  S(-1, 78),  S(-17, 99),
          S(18, 75),  S(53, 49),   S(67, 6),    S(20, 65),  S(-19, 54),
          S(-11, 51), S(-12, 60),  S(-19, 93),  S(4, 80),   S(-4, 97),
          S(10, 81),  S(9, 48),    S(-8, 21),   S(-20, 52), S(-23, 73),
          S(-13, 89), S(-17, 99),  S(-16, 89),  S(2, 57),   S(8, 65),
          S(-12, 18), S(-5, 39),   S(-5, 48),   S(-11, 60), S(-7, 91),
          S(11, 39),  S(15, 36),   S(9, 35),    S(-6, 2),   S(-10, 7),
          S(8, 11),   S(21, 17),   S(14, 35),   S(27, -6),  S(20, -15),
          S(38, -60), S(-9, 4),    S(-3, -1),   S(10, -4),  S(22, 18),
          S(13, 11),  S(-11, 12),  S(-1, -25),  S(-2, -9),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-56, -107), S(31, -65),  S(32, -36),   S(-54, -14), S(-60, -13),
          S(-13, -7),   S(49, 14),   S(35, -127),  S(-56, -19), S(-19, 20),
          S(-91, 38),   S(17, 20),   S(1, 22),     S(-15, 55),  S(97, 41),
          S(62, -11),   S(-136, 10), S(-9, 28),    S(-46, 54),  S(-57, 61),
          S(-42, 67),   S(46, 69),   S(61, 56),    S(21, 8),    S(-49, -10),
          S(-34, 35),   S(-100, 60), S(-198, 67),  S(-175, 65), S(-126, 64),
          S(-111, 52),  S(-126, 13), S(-146, -10), S(-84, 20),  S(-146, 53),
          S(-194, 61),  S(-218, 63), S(-132, 44),  S(-137, 28), S(-194, 17),
          S(-51, -29),  S(-42, 3),   S(-85, 29),   S(-142, 37), S(-113, 30),
          S(-102, 30),  S(-39, 0),   S(-86, -10),  S(59, -49),  S(16, -12),
          S(-8, 5),     S(-62, 5),   S(-63, 9),    S(-28, 7),   S(38, -17),
          S(40, -45),   S(44, -112), S(67, -74),   S(27, -45),  S(-85, -30),
          S(0, -45),    S(-43, -25), S(48, -64),   S(40, -109),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-4, 5),  S(-3, 14),  S(-4, 51),
          S(6, 94), S(1, 177), S(72, 199), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-4, -31),  S(-5, -24),  S(3, -36),
          S(16, -63),  S(-64, -84), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-18, -5), S(-24, -17), S(-20, -14),
          S(-3, -28), S(7, -32),  S(23, -11),  S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-18, -13), S(-3, -6),   S(-9, -3),
          S(-19, -7),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-36, -44), S(-16, -6), S(-4, 4), S(7, 12),  S(17, 29),
          S(31, 25),   S(42, 26),  S(49, 8), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-29, -76), S(-7, -44), S(1, -21), S(15, -5), S(22, 10),
          S(27, 19),   S(30, 24),  S(32, 31), S(35, 28), S(40, 23),
          S(45, 20),   S(11, 35),  S(40, -2), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-47, -63), S(-34, -46), S(-33, -35), S(-26, -26), S(-26, -19),
          S(-18, -5),  S(-9, -1),   S(-1, 5),    S(12, 10),   S(15, 15),
          S(22, 21),   S(22, 32),   S(13, 33),   S(-8, 40),   S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-128, -165), S(-55, -101), S(-35, -90), S(-26, -62), S(-17, -65),
          S(-6, -80),    S(-3, -39),   S(6, -39),   S(13, -38),  S(16, -16),
          S(21, -16),    S(25, -10),   S(28, -1),   S(25, 8),    S(32, 13),
          S(23, 32),     S(21, 42),    S(21, 45),   S(25, 57),   S(11, 67),
          S(46, 39),     S(46, 56),    S(20, 41),   S(74, 47),   S(37, 35),
          S(49, 94),     S(42, 116),   S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-22, 21), S(-15, 5), S(-29, 6), S(-36, 8), S(-19, -24),
    };
    const Score trappedRookWeight = {
          S(-5, -1),
    };
    const Score rookOnOpenFile[2] = {
          S(16, 1),
          S(31, 12),
    };
    const Score openFileNextToKing[3] = {
          S(-1, -19),
          S(-17, -15),
          S(-58, -38),
    };
    const Score pawnShieldStrength[4] = {
          S(-76, -31),
          S(-26, -32),
          S(14, -32),
          S(39, -25),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(4, 27),
          S(-50, -50),
          S(-5, -5),
          S(100, 100),
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
