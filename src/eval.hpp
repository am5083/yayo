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
#include "src/tt.hpp"
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

constexpr Score pawnScore = S(97, 147);
constexpr Score knightScore = S(409, 503);
constexpr Score bishopScore = S(449, 529);
constexpr Score rookScore = S(593, 940);
constexpr Score queenScore = S(1225, 1716);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(8, 123),  S(22, 95),
      S(-15, 112),  S(28, 35),  S(-24, 49), S(-9, 48),  S(-140, 127),
      S(-172, 152), S(0, 100),  S(1, 103),  S(41, 41),  S(43, -13),
      S(45, -13),   S(101, 14), S(54, 74),  S(15, 78),  S(-21, 65),
      S(-14, 47),   S(4, 22),   S(10, -10), S(37, -4),  S(42, 9),
      S(17, 31),    S(18, 25),  S(-36, 32), S(-26, 27), S(-9, 9),
      S(5, -5),     S(8, -3),   S(10, 6),   S(1, 11),   S(-6, 5),
      S(-28, 22),   S(-19, 18), S(-6, 10),  S(1, 10),   S(19, 10),
      S(9, 11),     S(31, 2),   S(13, -5),  S(-26, 27), S(-20, 24),
      S(-9, 14),    S(0, 12),   S(9, 20),   S(16, 19),  S(27, 7),
      S(-20, 7),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-173, -82), S(-141, -26), S(-66, 2),    S(-7, -18),  S(15, -4),
      S(-77, -20),  S(-89, -18),  S(-97, -111), S(-8, -6),   S(19, 1),
      S(51, -9),    S(71, 0),     S(65, -14),   S(110, -24), S(26, -6),
      S(75, -38),   S(-1, 2),     S(41, 1),     S(65, 24),   S(68, 27),
      S(99, 21),    S(116, -1),   S(60, -12),   S(39, -14),  S(13, 12),
      S(12, 23),    S(40, 48),    S(75, 45),    S(35, 49),   S(71, 43),
      S(10, 27),    S(54, 5),     S(1, 21),     S(13, 13),   S(20, 49),
      S(32, 42),    S(44, 52),    S(38, 34),    S(55, 11),   S(23, 14),
      S(-25, -9),   S(-14, 3),    S(2, 13),     S(10, 37),   S(30, 35),
      S(12, 5),     S(15, -4),    S(11, -2),    S(-38, -9),  S(-21, 6),
      S(-13, 1),    S(2, 6),      S(14, 2),     S(13, -4),   S(4, -13),
      S(3, -4),     S(-88, -13),  S(-15, -19),  S(-33, -6),  S(-6, 0),
      S(2, -1),     S(-1, -12),   S(-11, -10),  S(-36, -7),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-12, 5),  S(-75, 19), S(-54, 7),  S(-112, 25), S(-86, 14), S(-93, 7),
      S(-37, 8),  S(-50, -3), S(-15, -6), S(24, -2),   S(9, 5),    S(-21, 9),
      S(18, -6),  S(4, -1),   S(2, 10),   S(-17, -7),  S(-5, 25),  S(32, 6),
      S(28, 15),  S(34, 8),   S(22, 14),  S(54, 21),   S(26, 11),  S(4, 23),
      S(-12, 13), S(16, 20),  S(17, 22),  S(40, 39),   S(27, 34),  S(28, 28),
      S(14, 22),  S(-20, 15), S(0, 6),    S(-11, 28),  S(4, 32),   S(37, 35),
      S(30, 33),  S(12, 29),  S(-1, 25),  S(29, -3),   S(-2, 20),  S(24, 17),
      S(18, 23),  S(17, 29),  S(30, 32),  S(23, 22),   S(29, 12),  S(32, 0),
      S(19, 18),  S(19, 2),   S(34, -1),  S(8, 17),    S(17, 16),  S(40, 5),
      S(50, 3),   S(26, 1),   S(0, -3),   S(38, 24),   S(16, 15),  S(-3, 14),
      S(8, 8),    S(11, 23),  S(31, -7),  S(33, -21),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(14, 39),  S(25, 39),  S(-9, 63),  S(-5, 48),  S(33, 38),  S(45, 32),
      S(53, 32),  S(83, 22),  S(-3, 36),  S(-10, 51), S(7, 57),   S(40, 38),
      S(14, 39),  S(49, 24),  S(41, 21),  S(50, 17),  S(-4, 33),  S(30, 35),
      S(27, 33),  S(37, 32),  S(77, 15),  S(69, 4),   S(119, 2),  S(68, 4),
      S(-14, 40), S(10, 32),  S(8, 45),   S(21, 37),  S(29, 16),  S(29, 9),
      S(33, 17),  S(28, 8),   S(-37, 32), S(-33, 38), S(-21, 37), S(-10, 30),
      S(-4, 26),  S(-32, 25), S(1, 20),   S(-18, 14), S(-39, 27), S(-29, 27),
      S(-18, 19), S(-15, 25), S(-8, 19),  S(-11, 7),  S(29, -14), S(-6, -8),
      S(-42, 17), S(-24, 19), S(0, 18),   S(0, 17),   S(5, 4),    S(11, -4),
      S(40, -18), S(-26, -2), S(-15, 16), S(-13, 17), S(3, 21),   S(11, 17),
      S(18, 5),   S(6, 5),    S(10, 0),   S(-4, -4),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-78, 65), S(-46, 44),  S(-40, 82),  S(-2, 68),  S(0, 49),
      S(8, 47),   S(80, -25),  S(0, 44),    S(-17, 31), S(-48, 50),
      S(-33, 86), S(-59, 132), S(-65, 140), S(-3, 56),  S(-4, 43),
      S(62, 45),  S(-5, 42),   S(-18, 57),  S(-8, 81),  S(-17, 107),
      S(13, 85),  S(47, 51),   S(57, 8),    S(20, 58),  S(-25, 62),
      S(-18, 67), S(-13, 66),  S(-26, 102), S(-1, 91),  S(-5, 98),
      S(13, 86),  S(3, 65),    S(-8, 29),   S(-26, 67), S(-26, 82),
      S(-11, 89), S(-16, 102), S(-14, 90),  S(1, 69),   S(12, 65),
      S(-11, 14), S(-4, 46),   S(-4, 55),   S(-13, 70), S(-5, 92),
      S(10, 53),  S(19, 36),   S(13, 37),   S(-2, 6),   S(-7, 17),
      S(9, 22),   S(21, 28),   S(15, 40),   S(32, -9),  S(34, -25),
      S(44, -60), S(-9, 8),    S(2, -1),    S(17, 2),   S(24, 30),
      S(20, 12),  S(2, 3),     S(10, -17),  S(4, -20),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-49, -110), S(26, -63),  S(41, -36),   S(-68, -3),  S(-72, -17),
      S(-4, -7),    S(40, -2),   S(60, -138),  S(-77, -22), S(-15, 22),
      S(-88, 43),   S(32, 21),   S(0, 34),     S(-9, 62),   S(97, 34),
      S(58, -6),    S(-146, 10), S(19, 31),    S(-51, 56),  S(-73, 64),
      S(-50, 72),   S(63, 66),   S(60, 52),    S(15, 12),   S(-68, -11),
      S(-51, 29),   S(-100, 61), S(-218, 76),  S(-194, 71), S(-127, 64),
      S(-111, 50),  S(-149, 23), S(-133, -20), S(-84, 14),  S(-148, 50),
      S(-224, 70),  S(-232, 66), S(-121, 42),  S(-131, 24), S(-205, 17),
      S(-56, -33),  S(-14, -2),  S(-85, 26),   S(-146, 40), S(-125, 34),
      S(-97, 28),   S(-26, -4),  S(-85, -12),  S(65, -62),  S(23, -21),
      S(-3, 0),     S(-70, 6),   S(-68, 9),    S(-27, 6),   S(43, -21),
      S(43, -47),   S(28, -110), S(64, -68),   S(22, -30),  S(-93, -31),
      S(4, -53),    S(-43, -20), S(49, -60),   S(40, -109),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-3, 6),  S(-2, 14),  S(-7, 52),
      S(6, 94), S(0, 187), S(65, 209), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-4, -33),   S(-8, -24),  S(1, -36),
      S(21, -69),  S(-70, -105), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-18, -9), S(-25, -16), S(-21, -15),
      S(-4, -30), S(7, -39),  S(23, -12),  S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-18, -16), S(-4, -6),   S(-9, -3),
      S(-19, -10), S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-39, -40), S(-16, -5), S(-2, 7),  S(10, 17), S(20, 33),
      S(33, 30),   S(44, 30),  S(53, 17), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-29, -72), S(-7, -38), S(3, -16), S(16, -2), S(22, 13),
      S(28, 23),   S(32, 26),  S(34, 33), S(38, 31), S(49, 27),
      S(53, 23),   S(26, 37),  S(52, 12), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-47, -74), S(-33, -50), S(-33, -31), S(-26, -20), S(-26, -11),
      S(-18, 1),   S(-9, 4),    S(-1, 11),   S(13, 15),   S(16, 19),
      S(24, 26),   S(28, 34),   S(30, 34),   S(19, 39),   S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-153, -200), S(-57, -128), S(-42, -113), S(-26, -86), S(-17, -75),
      S(-8, -64),    S(-4, -24),   S(3, -15),    S(9, -6),    S(16, 1),
      S(21, 2),      S(25, 7),     S(28, 18),    S(26, 22),   S(33, 24),
      S(28, 42),     S(27, 50),    S(30, 52),    S(46, 54),   S(36, 63),
      S(70, 37),     S(79, 48),    S(62, 41),    S(97, 47),   S(61, 37),
      S(34, 68),     S(16, 76),    S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-23, 22), S(-16, 6), S(-29, 8), S(-36, 8), S(-21, -20),
};
constexpr Score trappedRookWeight = {
      S(-6, 10),
};
constexpr Score rookOnOpenFile[2] = {
      S(20, 17),
      S(28, 26),
};
constexpr Score openFileNextToKing[3] = {
      S(-2, -18),
      S(-17, -15),
      S(-53, -40),
};
constexpr Score pawnShieldStrength[4] = {
      S(-78, -13),
      S(-27, -27),
      S(15, -51),
      S(45, -91),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(9, 16),
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
    const Score pawnScore = S(97, 147);
    const Score knightScore = S(409, 503);
    const Score bishopScore = S(449, 529);
    const Score rookScore = S(593, 940);
    const Score queenScore = S(1225, 1716);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(8, 123),  S(22, 95),
          S(-15, 112),  S(28, 35),  S(-24, 49), S(-9, 48),  S(-140, 127),
          S(-172, 152), S(0, 100),  S(1, 103),  S(41, 41),  S(43, -13),
          S(45, -13),   S(101, 14), S(54, 74),  S(15, 78),  S(-21, 65),
          S(-14, 47),   S(4, 22),   S(10, -10), S(37, -4),  S(42, 9),
          S(17, 31),    S(18, 25),  S(-36, 32), S(-26, 27), S(-9, 9),
          S(5, -5),     S(8, -3),   S(10, 6),   S(1, 11),   S(-6, 5),
          S(-28, 22),   S(-19, 18), S(-6, 10),  S(1, 10),   S(19, 10),
          S(9, 11),     S(31, 2),   S(13, -5),  S(-26, 27), S(-20, 24),
          S(-9, 14),    S(0, 12),   S(9, 20),   S(16, 19),  S(27, 7),
          S(-20, 7),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-173, -82), S(-141, -26), S(-66, 2),    S(-7, -18),  S(15, -4),
          S(-77, -20),  S(-89, -18),  S(-97, -111), S(-8, -6),   S(19, 1),
          S(51, -9),    S(71, 0),     S(65, -14),   S(110, -24), S(26, -6),
          S(75, -38),   S(-1, 2),     S(41, 1),     S(65, 24),   S(68, 27),
          S(99, 21),    S(116, -1),   S(60, -12),   S(39, -14),  S(13, 12),
          S(12, 23),    S(40, 48),    S(75, 45),    S(35, 49),   S(71, 43),
          S(10, 27),    S(54, 5),     S(1, 21),     S(13, 13),   S(20, 49),
          S(32, 42),    S(44, 52),    S(38, 34),    S(55, 11),   S(23, 14),
          S(-25, -9),   S(-14, 3),    S(2, 13),     S(10, 37),   S(30, 35),
          S(12, 5),     S(15, -4),    S(11, -2),    S(-38, -9),  S(-21, 6),
          S(-13, 1),    S(2, 6),      S(14, 2),     S(13, -4),   S(4, -13),
          S(3, -4),     S(-88, -13),  S(-15, -19),  S(-33, -6),  S(-6, 0),
          S(2, -1),     S(-1, -12),   S(-11, -10),  S(-36, -7),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-12, 5),  S(-75, 19), S(-54, 7),  S(-112, 25), S(-86, 14),
          S(-93, 7),  S(-37, 8),  S(-50, -3), S(-15, -6),  S(24, -2),
          S(9, 5),    S(-21, 9),  S(18, -6),  S(4, -1),    S(2, 10),
          S(-17, -7), S(-5, 25),  S(32, 6),   S(28, 15),   S(34, 8),
          S(22, 14),  S(54, 21),  S(26, 11),  S(4, 23),    S(-12, 13),
          S(16, 20),  S(17, 22),  S(40, 39),  S(27, 34),   S(28, 28),
          S(14, 22),  S(-20, 15), S(0, 6),    S(-11, 28),  S(4, 32),
          S(37, 35),  S(30, 33),  S(12, 29),  S(-1, 25),   S(29, -3),
          S(-2, 20),  S(24, 17),  S(18, 23),  S(17, 29),   S(30, 32),
          S(23, 22),  S(29, 12),  S(32, 0),   S(19, 18),   S(19, 2),
          S(34, -1),  S(8, 17),   S(17, 16),  S(40, 5),    S(50, 3),
          S(26, 1),   S(0, -3),   S(38, 24),  S(16, 15),   S(-3, 14),
          S(8, 8),    S(11, 23),  S(31, -7),  S(33, -21),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(14, 39),  S(25, 39),  S(-9, 63),  S(-5, 48),  S(33, 38),
          S(45, 32),  S(53, 32),  S(83, 22),  S(-3, 36),  S(-10, 51),
          S(7, 57),   S(40, 38),  S(14, 39),  S(49, 24),  S(41, 21),
          S(50, 17),  S(-4, 33),  S(30, 35),  S(27, 33),  S(37, 32),
          S(77, 15),  S(69, 4),   S(119, 2),  S(68, 4),   S(-14, 40),
          S(10, 32),  S(8, 45),   S(21, 37),  S(29, 16),  S(29, 9),
          S(33, 17),  S(28, 8),   S(-37, 32), S(-33, 38), S(-21, 37),
          S(-10, 30), S(-4, 26),  S(-32, 25), S(1, 20),   S(-18, 14),
          S(-39, 27), S(-29, 27), S(-18, 19), S(-15, 25), S(-8, 19),
          S(-11, 7),  S(29, -14), S(-6, -8),  S(-42, 17), S(-24, 19),
          S(0, 18),   S(0, 17),   S(5, 4),    S(11, -4),  S(40, -18),
          S(-26, -2), S(-15, 16), S(-13, 17), S(3, 21),   S(11, 17),
          S(18, 5),   S(6, 5),    S(10, 0),   S(-4, -4),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-78, 65), S(-46, 44),  S(-40, 82),  S(-2, 68),  S(0, 49),
          S(8, 47),   S(80, -25),  S(0, 44),    S(-17, 31), S(-48, 50),
          S(-33, 86), S(-59, 132), S(-65, 140), S(-3, 56),  S(-4, 43),
          S(62, 45),  S(-5, 42),   S(-18, 57),  S(-8, 81),  S(-17, 107),
          S(13, 85),  S(47, 51),   S(57, 8),    S(20, 58),  S(-25, 62),
          S(-18, 67), S(-13, 66),  S(-26, 102), S(-1, 91),  S(-5, 98),
          S(13, 86),  S(3, 65),    S(-8, 29),   S(-26, 67), S(-26, 82),
          S(-11, 89), S(-16, 102), S(-14, 90),  S(1, 69),   S(12, 65),
          S(-11, 14), S(-4, 46),   S(-4, 55),   S(-13, 70), S(-5, 92),
          S(10, 53),  S(19, 36),   S(13, 37),   S(-2, 6),   S(-7, 17),
          S(9, 22),   S(21, 28),   S(15, 40),   S(32, -9),  S(34, -25),
          S(44, -60), S(-9, 8),    S(2, -1),    S(17, 2),   S(24, 30),
          S(20, 12),  S(2, 3),     S(10, -17),  S(4, -20),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-49, -110), S(26, -63),  S(41, -36),   S(-68, -3),  S(-72, -17),
          S(-4, -7),    S(40, -2),   S(60, -138),  S(-77, -22), S(-15, 22),
          S(-88, 43),   S(32, 21),   S(0, 34),     S(-9, 62),   S(97, 34),
          S(58, -6),    S(-146, 10), S(19, 31),    S(-51, 56),  S(-73, 64),
          S(-50, 72),   S(63, 66),   S(60, 52),    S(15, 12),   S(-68, -11),
          S(-51, 29),   S(-100, 61), S(-218, 76),  S(-194, 71), S(-127, 64),
          S(-111, 50),  S(-149, 23), S(-133, -20), S(-84, 14),  S(-148, 50),
          S(-224, 70),  S(-232, 66), S(-121, 42),  S(-131, 24), S(-205, 17),
          S(-56, -33),  S(-14, -2),  S(-85, 26),   S(-146, 40), S(-125, 34),
          S(-97, 28),   S(-26, -4),  S(-85, -12),  S(65, -62),  S(23, -21),
          S(-3, 0),     S(-70, 6),   S(-68, 9),    S(-27, 6),   S(43, -21),
          S(43, -47),   S(28, -110), S(64, -68),   S(22, -30),  S(-93, -31),
          S(4, -53),    S(-43, -20), S(49, -60),   S(40, -109),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-3, 6),  S(-2, 14),  S(-7, 52),
          S(6, 94), S(0, 187), S(65, 209), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-4, -33),   S(-8, -24),  S(1, -36),
          S(21, -69),  S(-70, -105), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-18, -9), S(-25, -16), S(-21, -15),
          S(-4, -30), S(7, -39),  S(23, -12),  S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-18, -16), S(-4, -6),   S(-9, -3),
          S(-19, -10), S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-39, -40), S(-16, -5), S(-2, 7),  S(10, 17), S(20, 33),
          S(33, 30),   S(44, 30),  S(53, 17), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-29, -72), S(-7, -38), S(3, -16), S(16, -2), S(22, 13),
          S(28, 23),   S(32, 26),  S(34, 33), S(38, 31), S(49, 27),
          S(53, 23),   S(26, 37),  S(52, 12), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-47, -74), S(-33, -50), S(-33, -31), S(-26, -20), S(-26, -11),
          S(-18, 1),   S(-9, 4),    S(-1, 11),   S(13, 15),   S(16, 19),
          S(24, 26),   S(28, 34),   S(30, 34),   S(19, 39),   S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-153, -200), S(-57, -128), S(-42, -113), S(-26, -86), S(-17, -75),
          S(-8, -64),    S(-4, -24),   S(3, -15),    S(9, -6),    S(16, 1),
          S(21, 2),      S(25, 7),     S(28, 18),    S(26, 22),   S(33, 24),
          S(28, 42),     S(27, 50),    S(30, 52),    S(46, 54),   S(36, 63),
          S(70, 37),     S(79, 48),    S(62, 41),    S(97, 47),   S(61, 37),
          S(34, 68),     S(16, 76),    S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-23, 22), S(-16, 6), S(-29, 8), S(-36, 8), S(-21, -20),
    };
    const Score trappedRookWeight = {
          S(-6, 10),
    };
    const Score rookOnOpenFile[2] = {
          S(20, 17),
          S(28, 26),
    };
    const Score openFileNextToKing[3] = {
          S(-2, -18),
          S(-17, -15),
          S(-53, -40),
    };
    const Score pawnShieldStrength[4] = {
          S(-78, -13),
          S(-27, -27),
          S(15, -51),
          S(45, -91),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(9, 16),
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

enum Tracing : bool { NO_TRACE, TRACE };

template <Tracing T = NO_TRACE> class Eval {
  public:
    Eval(Board &b) : board(b), trace(tempTrace) { init(); };
    Eval(Board &b, Trace &t) : board(b), trace(t) { init(); }

    int eval() {
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
