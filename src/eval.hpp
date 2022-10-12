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

constexpr Score pawnScore = S(71, 161);
constexpr Score knightScore = S(391, 552);
constexpr Score bishopScore = S(437, 566);
constexpr Score rookScore = S(553, 929);
constexpr Score queenScore = S(1282, 1753);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),     S(32, 128), S(54, 156),
      S(31, 110),   S(7, -3),    S(-38, 54),  S(10, 70),  S(-129, 147),
      S(-200, 162), S(-21, 98),  S(18, 126),  S(79, 42),  S(26, -55),
      S(55, -13),   S(96, -34),  S(24, 87),   S(-20, 53), S(-11, 64),
      S(-38, 43),   S(-7, 31),   S(43, 12),   S(25, -21), S(72, 37),
      S(16, 0),     S(-25, 45),  S(-7, 45),   S(-48, 30), S(-45, -41),
      S(-8, 13),    S(-13, -25), S(14, 26),   S(-11, 12), S(-59, 27),
      S(-17, 37),   S(-18, 19),  S(-17, 17),  S(6, 12),   S(15, -9),
      S(18, 17),    S(53, -1),   S(17, -25),  S(-22, 23), S(-11, 39),
      S(8, 1),      S(-5, 65),   S(-22, -20), S(17, 26),  S(44, 27),
      S(-27, 18),   S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-157, -67), S(-148, -1),  S(-84, -23),  S(20, -14),  S(21, 5),
      S(-84, 1),    S(-100, -10), S(-73, -134), S(-4, -61),  S(2, -15),
      S(55, 30),    S(56, -3),    S(50, -8),    S(105, -22), S(5, -5),
      S(102, -2),   S(-27, -4),   S(94, 50),    S(40, 2),    S(77, 52),
      S(78, 17),    S(61, 20),    S(16, -25),   S(41, 13),   S(-13, -13),
      S(5, -10),    S(21, 40),    S(55, 62),    S(15, 46),   S(66, 37),
      S(24, 51),    S(76, 34),    S(-12, 72),   S(22, 23),   S(23, 59),
      S(25, 48),    S(17, 57),    S(37, 72),    S(52, 24),   S(43, -51),
      S(-62, -15),  S(-7, 16),    S(1, -24),    S(27, 24),   S(42, 58),
      S(14, 54),    S(-23, 9),    S(11, 4),     S(-55, -19), S(-74, 40),
      S(2, -28),    S(29, 48),    S(18, 23),    S(25, -12),  S(-16, -59),
      S(-29, 11),   S(-114, -26), S(-7, -14),   S(7, 10),    S(-22, -34),
      S(4, -33),    S(10, -41),   S(12, 5),     S(1, 12),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-48, 10),  S(-96, 18),  S(-92, 15), S(-133, 8),  S(-122, 19),
      S(-150, 6),  S(-44, -2),  S(-83, 15), S(29, -6),   S(-7, 3),
      S(-24, 19),  S(-48, -30), S(16, 23),  S(-32, -15), S(-29, 16),
      S(-20, -41), S(-12, 42),  S(22, 10),  S(-30, 15),  S(22, 11),
      S(9, 18),    S(3, 47),    S(-19, 6),  S(24, 34),   S(2, 19),
      S(24, 23),   S(33, 9),    S(27, 42),  S(12, 27),   S(-7, -9),
      S(-1, 76),   S(-50, 21),  S(-37, -1), S(-7, 52),   S(39, 16),
      S(37, 93),   S(39, 32),   S(17, 38),  S(-14, 12),  S(45, -3),
      S(9, -2),    S(-16, 11),  S(-17, -9), S(-2, 26),   S(42, 90),
      S(6, 44),    S(34, 19),   S(48, -7),  S(23, 32),   S(32, -4),
      S(-9, -35),  S(6, 31),    S(25, 53),  S(136, 41),  S(52, -40),
      S(18, 68),   S(14, 2),    S(135, 51), S(38, 2),    S(-24, -16),
      S(-35, 11),  S(-1, 7),    S(12, 0),   S(38, -40),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(-26, 47), S(19, 59),   S(-9, 70),   S(-18, 47), S(16, 26),  S(42, 49),
      S(45, 45),  S(114, 57),  S(19, 45),   S(30, 42),  S(11, 69),  S(42, 32),
      S(6, 60),   S(50, 25),   S(25, -7),   S(47, -1),  S(-2, 64),  S(59, 59),
      S(21, 57),  S(1, 18),    S(66, 8),    S(63, 16),  S(121, 1),  S(122, -2),
      S(2, 16),   S(13, 31),   S(-20, 20),  S(-3, -23), S(-15, -3), S(30, 8),
      S(16, 13),  S(62, -6),   S(-28, 41),  S(-49, 41), S(-61, 75), S(-47, -5),
      S(-11, 10), S(-75, 0),   S(-13, -12), S(-22, 27), S(-97, -4), S(-7, 25),
      S(7, 56),   S(0, 15),    S(-7, 8),    S(3, 19),   S(20, -13), S(-24, -4),
      S(-68, 18), S(-27, -7),  S(9, 9),     S(-10, 18), S(-26, 4),  S(-18, -7),
      S(54, -1),  S(-39, -23), S(2, 30),    S(-39, 22), S(11, 21),  S(25, -11),
      S(20, 1),   S(14, 24),   S(-24, -39), S(-9, 22),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-11, 132), S(-62, 16),  S(-51, 54),  S(-21, 73),  S(15, 56),
      S(-3, 18),   S(81, -59),  S(-39, 14),  S(25, 68),   S(-40, 52),
      S(-57, 91),  S(-92, 117), S(-97, 140), S(25, 57),   S(31, 62),
      S(70, 72),   S(-6, 29),   S(-20, 56),  S(0, 71),    S(-44, 100),
      S(35, 83),   S(45, 66),   S(64, -7),   S(80, 86),   S(-31, 58),
      S(-25, 71),  S(-17, 63),  S(-61, 97),  S(-20, 86),  S(-22, 102),
      S(5, 83),    S(-14, 35),  S(-38, 31),  S(-52, 60),  S(-33, 124),
      S(-4, 123),  S(-21, 96),  S(40, 106),  S(-4, 70),   S(4, 52),
      S(-10, 7),   S(24, 68),   S(-20, 60),  S(20, 106),  S(-12, 92),
      S(14, 104),  S(27, 30),   S(-1, 51),   S(16, 22),   S(-39, 16),
      S(27, 21),   S(11, -9),   S(18, 49),   S(6, -31),   S(6, -31),
      S(11, -37),  S(21, 28),   S(2, -6),    S(11, 7),    S(23, 39),
      S(15, -34),  S(47, 33),   S(2, -15),   S(-28, -34),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-64, -108), S(19, -79),  S(20, -57),   S(-105, -31), S(-93, -31),
      S(-38, -42),  S(28, -13),  S(86, -118),  S(-116, -48), S(-28, 37),
      S(-82, 50),   S(17, 3),    S(14, 2),     S(-30, 23),   S(85, 34),
      S(60, -11),   S(-134, 3),  S(51, 80),    S(-7, 70),    S(-67, 34),
      S(-71, 43),   S(44, 31),   S(67, 48),    S(13, 19),    S(-80, 1),
      S(-28, 86),   S(-125, 63), S(-264, 15),  S(-214, 89),  S(-165, 62),
      S(-112, -12), S(-116, 34), S(-131, -19), S(-94, 8),    S(-121, 86),
      S(-244, 40),  S(-231, 56), S(-131, 20),  S(-149, 26),  S(-203, 15),
      S(-41, -21),  S(-5, 29),   S(-136, 13),  S(-144, 45),  S(-138, 56),
      S(-106, 36),  S(-20, 27),  S(-84, -11),  S(49, -48),   S(-10, -24),
      S(15, 45),    S(-89, -8),  S(-64, 36),   S(-22, 8),    S(76, -25),
      S(44, -52),   S(74, -125), S(39, -71),   S(18, -45),   S(-149, -53),
      S(4, -58),    S(-16, 8),   S(62, -65),   S(8, -64),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),     S(29, 2),   S(0, 55),   S(-19, 86),
      S(-10, 127), S(39, 206), S(88, 236), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-11, -15),  S(-25, -29), S(18, -73),
      S(1, -117),  S(-72, -121), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-24, -3), S(-26, 14), S(4, 0),
      S(22, -25), S(23, -42), S(65, -23), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(7, 0),     S(-5, -27),  S(-18, -17),
      S(-9, -16),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-39, -39), S(-16, 7), S(-8, -15), S(16, 28), S(21, 54),
      S(18, 53),   S(40, 66), S(49, 23),  S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-19, -66), S(3, -18), S(-14, -21), S(19, -3), S(19, 44),
      S(23, 39),   S(20, 45), S(53, 33),   S(34, 18), S(29, 30),
      S(81, 21),   S(64, 68), S(27, 8),    S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-62, -107), S(-30, -46), S(-25, -11), S(-30, 2), S(-26, -10),
      S(-17, 8),    S(-9, -2),   S(-8, 9),    S(22, 11), S(-12, 23),
      S(18, 19),    S(19, 21),   S(45, 16),   S(30, 34), S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-155, -221), S(-50, -125), S(-22, -108), S(-9, -82), S(-24, -62),
      S(-12, -19),   S(-8, 2),     S(15, 2),     S(17, -8),  S(16, 12),
      S(16, 26),     S(26, 25),    S(23, 62),    S(20, 14),  S(36, 14),
      S(35, 36),     S(3, 27),     S(42, 56),    S(31, 52),  S(20, 51),
      S(72, 37),     S(66, 59),    S(60, 63),    S(73, 19),  S(72, 43),
      S(34, 61),     S(14, 65),    S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-2, 39), S(17, 6), S(-3, 23), S(-24, 34), S(-23, 24),
};
constexpr Score trappedRookWeight = {
      S(-6, 7),
};
constexpr Score rookOnOpenFile[2] = {
      S(-3, 40),
      S(8, -3),
};
constexpr Score openFileNextToKing[3] = {
      S(15, 16),
      S(-32, -26),
      S(-60, -73),
};
constexpr Score pawnShieldStrength[4] = {
      S(-74, 4),
      S(-10, -24),
      S(14, -51),
      S(38, -47),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(18, 48),
      S(-50, -50),
      S(-5, -5),
      S(100, 100),
};
constexpr Score kingAttackersDistance[8] = {
      S(-18, -12), S(-18, -21), S(-18, -13), S(-10, -23),
      S(9, -6),    S(-28, -23), S(-45, -32), S(0, 0),
};
constexpr Score xRayKingAttackersDistance[8] = {
      S(0, 0),   S(-17, 17),  S(-6, 12),   S(-22, -29),
      S(7, -38), S(-22, -20), S(-54, -28), S(0, 0),
};
constexpr Score xRayKingAttackPieceWeight[5] = {
      S(-23, 22), S(-16, 6), S(-12, 38), S(-2, 28), S(-9, 38),
};
constexpr Score hangingPieceWeight[5] = {
      S(-10, -10), S(-25, -25), S(-27, -27), S(-50, -50), S(-100, -100),
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
    int openKingFiles[3][NUM_COLOR] = {{0}};
    int pawnShieldStrength[4][NUM_COLOR] = {{0}};
    int pushedPawnShieldStrength[4][NUM_COLOR] = {{0}};
    int kingAttackersDistance[8][NUM_COLOR] = {{0}};
    int xRayKingAttackersDistance[8][NUM_COLOR] = {{0}};
    int xRayKingAttackPieceWeight[5][NUM_COLOR] = {{0}};
};

struct EvalWeights {
    const Score pawnScore = S(71, 161);
    const Score knightScore = S(391, 552);
    const Score bishopScore = S(437, 566);
    const Score rookScore = S(553, 929);
    const Score queenScore = S(1282, 1753);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),     S(32, 128), S(54, 156),
          S(31, 110),   S(7, -3),    S(-38, 54),  S(10, 70),  S(-129, 147),
          S(-200, 162), S(-21, 98),  S(18, 126),  S(79, 42),  S(26, -55),
          S(55, -13),   S(96, -34),  S(24, 87),   S(-20, 53), S(-11, 64),
          S(-38, 43),   S(-7, 31),   S(43, 12),   S(25, -21), S(72, 37),
          S(16, 0),     S(-25, 45),  S(-7, 45),   S(-48, 30), S(-45, -41),
          S(-8, 13),    S(-13, -25), S(14, 26),   S(-11, 12), S(-59, 27),
          S(-17, 37),   S(-18, 19),  S(-17, 17),  S(6, 12),   S(15, -9),
          S(18, 17),    S(53, -1),   S(17, -25),  S(-22, 23), S(-11, 39),
          S(8, 1),      S(-5, 65),   S(-22, -20), S(17, 26),  S(44, 27),
          S(-27, 18),   S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-157, -67), S(-148, -1),  S(-84, -23),  S(20, -14),  S(21, 5),
          S(-84, 1),    S(-100, -10), S(-73, -134), S(-4, -61),  S(2, -15),
          S(55, 30),    S(56, -3),    S(50, -8),    S(105, -22), S(5, -5),
          S(102, -2),   S(-27, -4),   S(94, 50),    S(40, 2),    S(77, 52),
          S(78, 17),    S(61, 20),    S(16, -25),   S(41, 13),   S(-13, -13),
          S(5, -10),    S(21, 40),    S(55, 62),    S(15, 46),   S(66, 37),
          S(24, 51),    S(76, 34),    S(-12, 72),   S(22, 23),   S(23, 59),
          S(25, 48),    S(17, 57),    S(37, 72),    S(52, 24),   S(43, -51),
          S(-62, -15),  S(-7, 16),    S(1, -24),    S(27, 24),   S(42, 58),
          S(14, 54),    S(-23, 9),    S(11, 4),     S(-55, -19), S(-74, 40),
          S(2, -28),    S(29, 48),    S(18, 23),    S(25, -12),  S(-16, -59),
          S(-29, 11),   S(-114, -26), S(-7, -14),   S(7, 10),    S(-22, -34),
          S(4, -33),    S(10, -41),   S(12, 5),     S(1, 12),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-48, 10),  S(-96, 18),  S(-92, 15), S(-133, 8),  S(-122, 19),
          S(-150, 6),  S(-44, -2),  S(-83, 15), S(29, -6),   S(-7, 3),
          S(-24, 19),  S(-48, -30), S(16, 23),  S(-32, -15), S(-29, 16),
          S(-20, -41), S(-12, 42),  S(22, 10),  S(-30, 15),  S(22, 11),
          S(9, 18),    S(3, 47),    S(-19, 6),  S(24, 34),   S(2, 19),
          S(24, 23),   S(33, 9),    S(27, 42),  S(12, 27),   S(-7, -9),
          S(-1, 76),   S(-50, 21),  S(-37, -1), S(-7, 52),   S(39, 16),
          S(37, 93),   S(39, 32),   S(17, 38),  S(-14, 12),  S(45, -3),
          S(9, -2),    S(-16, 11),  S(-17, -9), S(-2, 26),   S(42, 90),
          S(6, 44),    S(34, 19),   S(48, -7),  S(23, 32),   S(32, -4),
          S(-9, -35),  S(6, 31),    S(25, 53),  S(136, 41),  S(52, -40),
          S(18, 68),   S(14, 2),    S(135, 51), S(38, 2),    S(-24, -16),
          S(-35, 11),  S(-1, 7),    S(12, 0),   S(38, -40),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(-26, 47),  S(19, 59),  S(-9, 70),   S(-18, 47),  S(16, 26),
          S(42, 49),   S(45, 45),  S(114, 57),  S(19, 45),   S(30, 42),
          S(11, 69),   S(42, 32),  S(6, 60),    S(50, 25),   S(25, -7),
          S(47, -1),   S(-2, 64),  S(59, 59),   S(21, 57),   S(1, 18),
          S(66, 8),    S(63, 16),  S(121, 1),   S(122, -2),  S(2, 16),
          S(13, 31),   S(-20, 20), S(-3, -23),  S(-15, -3),  S(30, 8),
          S(16, 13),   S(62, -6),  S(-28, 41),  S(-49, 41),  S(-61, 75),
          S(-47, -5),  S(-11, 10), S(-75, 0),   S(-13, -12), S(-22, 27),
          S(-97, -4),  S(-7, 25),  S(7, 56),    S(0, 15),    S(-7, 8),
          S(3, 19),    S(20, -13), S(-24, -4),  S(-68, 18),  S(-27, -7),
          S(9, 9),     S(-10, 18), S(-26, 4),   S(-18, -7),  S(54, -1),
          S(-39, -23), S(2, 30),   S(-39, 22),  S(11, 21),   S(25, -11),
          S(20, 1),    S(14, 24),  S(-24, -39), S(-9, 22),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-11, 132), S(-62, 16),  S(-51, 54),  S(-21, 73),  S(15, 56),
          S(-3, 18),   S(81, -59),  S(-39, 14),  S(25, 68),   S(-40, 52),
          S(-57, 91),  S(-92, 117), S(-97, 140), S(25, 57),   S(31, 62),
          S(70, 72),   S(-6, 29),   S(-20, 56),  S(0, 71),    S(-44, 100),
          S(35, 83),   S(45, 66),   S(64, -7),   S(80, 86),   S(-31, 58),
          S(-25, 71),  S(-17, 63),  S(-61, 97),  S(-20, 86),  S(-22, 102),
          S(5, 83),    S(-14, 35),  S(-38, 31),  S(-52, 60),  S(-33, 124),
          S(-4, 123),  S(-21, 96),  S(40, 106),  S(-4, 70),   S(4, 52),
          S(-10, 7),   S(24, 68),   S(-20, 60),  S(20, 106),  S(-12, 92),
          S(14, 104),  S(27, 30),   S(-1, 51),   S(16, 22),   S(-39, 16),
          S(27, 21),   S(11, -9),   S(18, 49),   S(6, -31),   S(6, -31),
          S(11, -37),  S(21, 28),   S(2, -6),    S(11, 7),    S(23, 39),
          S(15, -34),  S(47, 33),   S(2, -15),   S(-28, -34),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-64, -108), S(19, -79),  S(20, -57),   S(-105, -31), S(-93, -31),
          S(-38, -42),  S(28, -13),  S(86, -118),  S(-116, -48), S(-28, 37),
          S(-82, 50),   S(17, 3),    S(14, 2),     S(-30, 23),   S(85, 34),
          S(60, -11),   S(-134, 3),  S(51, 80),    S(-7, 70),    S(-67, 34),
          S(-71, 43),   S(44, 31),   S(67, 48),    S(13, 19),    S(-80, 1),
          S(-28, 86),   S(-125, 63), S(-264, 15),  S(-214, 89),  S(-165, 62),
          S(-112, -12), S(-116, 34), S(-131, -19), S(-94, 8),    S(-121, 86),
          S(-244, 40),  S(-231, 56), S(-131, 20),  S(-149, 26),  S(-203, 15),
          S(-41, -21),  S(-5, 29),   S(-136, 13),  S(-144, 45),  S(-138, 56),
          S(-106, 36),  S(-20, 27),  S(-84, -11),  S(49, -48),   S(-10, -24),
          S(15, 45),    S(-89, -8),  S(-64, 36),   S(-22, 8),    S(76, -25),
          S(44, -52),   S(74, -125), S(39, -71),   S(18, -45),   S(-149, -53),
          S(4, -58),    S(-16, 8),   S(62, -65),   S(8, -64),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),     S(29, 2),   S(0, 55),   S(-19, 86),
          S(-10, 127), S(39, 206), S(88, 236), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-11, -15),  S(-25, -29), S(18, -73),
          S(1, -117),  S(-72, -121), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-24, -3), S(-26, 14), S(4, 0),
          S(22, -25), S(23, -42), S(65, -23), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(7, 0),     S(-5, -27),  S(-18, -17),
          S(-9, -16),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-39, -39), S(-16, 7), S(-8, -15), S(16, 28), S(21, 54),
          S(18, 53),   S(40, 66), S(49, 23),  S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-19, -66), S(3, -18), S(-14, -21), S(19, -3), S(19, 44),
          S(23, 39),   S(20, 45), S(53, 33),   S(34, 18), S(29, 30),
          S(81, 21),   S(64, 68), S(27, 8),    S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-62, -107), S(-30, -46), S(-25, -11), S(-30, 2), S(-26, -10),
          S(-17, 8),    S(-9, -2),   S(-8, 9),    S(22, 11), S(-12, 23),
          S(18, 19),    S(19, 21),   S(45, 16),   S(30, 34), S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-155, -221), S(-50, -125), S(-22, -108), S(-9, -82), S(-24, -62),
          S(-12, -19),   S(-8, 2),     S(15, 2),     S(17, -8),  S(16, 12),
          S(16, 26),     S(26, 25),    S(23, 62),    S(20, 14),  S(36, 14),
          S(35, 36),     S(3, 27),     S(42, 56),    S(31, 52),  S(20, 51),
          S(72, 37),     S(66, 59),    S(60, 63),    S(73, 19),  S(72, 43),
          S(34, 61),     S(14, 65),    S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-2, 39), S(17, 6), S(-3, 23), S(-24, 34), S(-23, 24),
    };
    const Score trappedRookWeight = {
          S(-6, 7),
    };
    const Score rookOnOpenFile[2] = {
          S(-3, 40),
          S(8, -3),
    };
    const Score openFileNextToKing[3] = {
          S(15, 16),
          S(-32, -26),
          S(-60, -73),
    };
    const Score pawnShieldStrength[4] = {
          S(-74, 4),
          S(-10, -24),
          S(14, -51),
          S(38, -47),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(18, 48),
          S(-50, -50),
          S(-5, -5),
          S(100, 100),
    };
    const Score kingAttackersDistance[8] = {
          S(-18, -12), S(-18, -21), S(-18, -13), S(-10, -23),
          S(9, -6),    S(-28, -23), S(-45, -32), S(0, 0),
    };
    const Score xRayKingAttackersDistance[8] = {
          S(0, 0),   S(-17, 17),  S(-6, 12),   S(-22, -29),
          S(7, -38), S(-22, -20), S(-54, -28), S(0, 0),
    };
    const Score xRayKingAttackPieceWeight[5] = {
          S(-23, 22), S(-16, 6), S(-12, 38), S(-2, 28), S(-9, 38),
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
        const int mgSafety = MgScore(wKingSafety) - MgScore(bKingSafety);
        const int egSafety = EgScore(wKingSafety) - EgScore(bKingSafety);

        const Score wKingXrayAtks = kingXrayAttackers<WHITE>();
        const Score bKingXrayAtks = kingXrayAttackers<BLACK>();
        const int mgXrayAtks = MgScore(wKingXrayAtks) - MgScore(bKingXrayAtks);
        const int egXrayAtks = EgScore(wKingXrayAtks) - EgScore(bKingXrayAtks);

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
        const int kingXrayAtks =
              (mgXrayAtks * mgPhase + egXrayAtks * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + pawnStructureEval + mobilityEval +
                rookEval + safetyEval + KSEval + kingXrayAtks;

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
    template <Color C> constexpr Score kingXrayAttackers();
    template <Color C> constexpr Score kingSafety();
    template <Color C> constexpr Score rookEval();
    template <Color C> constexpr Score hangingPiecePenalty();

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
constexpr Score Eval<T>::kingXrayAttackers() {
    const Bitboard king = board.pieces(KING, C);
    const Square kingSquare = Sq(king);

    int mgScore = 0, egScore = 0;
    Bitboard xRayPieces = board.pieces(BISHOP, ~C) | board.pieces(ROOK, ~C) |
                          board.pieces(QUEEN, ~C);
    while (xRayPieces) {
        Square xRayPcSq = Square(lsb_index(xRayPieces));
        Bitboard betweenMask = rectArray[kingSquare][xRayPcSq];

        int n = popcount(betweenMask & board.pieces());
        if (n > 1 || n < 1) {
            xRayPieces &= xRayPieces - 1;
            continue;
        }

        int distanceToKing = popcount(betweenMask);
        PieceT xRayPc = getPcType(board.board[xRayPcSq]);

        mgScore += MgScore(xRayKingAttackersDistance[distanceToKing]);
        egScore += EgScore(xRayKingAttackersDistance[distanceToKing]);
        mgScore += MgScore(xRayKingAttackPieceWeight[xRayPc - 1]);
        egScore += EgScore(xRayKingAttackPieceWeight[xRayPc - 1]);

        if (T) {
            trace.xRayKingAttackersDistance[distanceToKing][C]++;
            trace.xRayKingAttackPieceWeight[xRayPc - 1][C]++;
        }

        xRayPieces &= xRayPieces - 1;
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

            const int distance = popcount(rectArray[kingSquare][attacker]);
            if (T == TRACE) {
                trace.kingAttackers[piece - 1][C]++;
                trace.kingAttackersDistance[distance][C]++;
            }

            mgScore += MgScore(kingAttackersWeight[piece - 1]);
            egScore += EgScore(kingAttackersWeight[piece - 1]);
            mgScore += MgScore(kingAttackersDistance[distance]);
            egScore += EgScore(kingAttackersDistance[distance]);

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
