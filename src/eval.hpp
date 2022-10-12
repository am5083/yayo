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

constexpr Score pawnScore = S(107, 164);
constexpr Score knightScore = S(432, 528);
constexpr Score bishopScore = S(477, 556);
constexpr Score rookScore = S(634, 994);
constexpr Score queenScore = S(1282, 1788);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(8, 128),  S(26, 98),
      S(-14, 114),  S(30, 35),  S(-19, 49), S(-7, 50),  S(-135, 129),
      S(-176, 154), S(0, 104),  S(4, 107),  S(45, 44),  S(46, -13),
      S(44, -13),   S(109, 17), S(53, 77),  S(15, 82),  S(-21, 68),
      S(-14, 49),   S(5, 24),   S(12, -10), S(41, -4),  S(43, 10),
      S(17, 33),    S(17, 28),  S(-36, 32), S(-27, 28), S(-9, 9),
      S(7, -4),     S(10, -3),  S(9, 6),    S(0, 12),   S(-8, 4),
      S(-28, 23),   S(-19, 19), S(-6, 10),  S(1, 10),   S(20, 11),
      S(8, 11),     S(31, 2),   S(13, -5),  S(-26, 28), S(-20, 26),
      S(-9, 15),    S(0, 11),   S(12, 21),  S(16, 21),  S(28, 8),
      S(-22, 8),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-172, -82), S(-142, -25), S(-67, 3),    S(-8, -18),  S(14, -3),
      S(-75, -20),  S(-92, -18),  S(-97, -110), S(-8, -6),   S(21, 2),
      S(52, -7),    S(72, 1),     S(65, -14),   S(112, -24), S(26, -6),
      S(75, -39),   S(0, 3),      S(43, 3),     S(67, 27),   S(72, 32),
      S(96, 21),    S(115, -1),   S(54, -12),   S(39, -14),  S(15, 14),
      S(13, 25),    S(45, 52),    S(86, 50),    S(41, 55),   S(79, 46),
      S(4, 28),     S(56, 6),     S(1, 24),     S(15, 17),   S(23, 52),
      S(36, 49),    S(49, 56),    S(41, 37),    S(57, 13),   S(25, 15),
      S(-27, -8),   S(-15, 5),    S(3, 16),     S(11, 41),   S(31, 38),
      S(15, 7),     S(17, -2),    S(11, -1),    S(-39, -8),  S(-21, 8),
      S(-14, 2),    S(5, 8),      S(16, 5),     S(15, -2),   S(3, -11),
      S(4, -1),     S(-89, -11),  S(-16, -17),  S(-35, -4),  S(-7, 2),
      S(2, -1),     S(0, -10),    S(-11, -8),   S(-37, -6),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-10, 7),  S(-74, 21), S(-56, 9),  S(-114, 27), S(-92, 14), S(-94, 7),
      S(-39, 8),  S(-50, -2), S(-15, -6), S(26, 0),    S(10, 6),   S(-20, 9),
      S(18, -6),  S(2, -2),   S(1, 11),   S(-17, -8),  S(-3, 27),  S(32, 9),
      S(30, 18),  S(37, 8),   S(18, 14),  S(55, 21),   S(15, 10),  S(2, 23),
      S(-12, 16), S(15, 25),  S(19, 24),  S(32, 40),   S(30, 35),  S(28, 28),
      S(8, 22),   S(-26, 16), S(0, 8),    S(-10, 31),  S(2, 35),   S(40, 38),
      S(33, 35),  S(16, 30),  S(-4, 25),  S(31, -3),   S(0, 21),   S(26, 20),
      S(20, 27),  S(22, 32),  S(34, 38),  S(29, 25),   S(29, 14),  S(34, 3),
      S(19, 21),  S(23, 5),   S(38, 1),   S(9, 21),    S(21, 22),  S(43, 8),
      S(56, 5),   S(29, 1),   S(0, 0),    S(41, 25),   S(19, 18),  S(-3, 16),
      S(9, 10),   S(13, 28),  S(32, -5),  S(34, -21),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(13, 41),  S(22, 41),  S(-12, 65), S(-11, 51), S(26, 39),  S(45, 33),
      S(49, 33),  S(80, 24),  S(0, 38),   S(-9, 55),  S(10, 61),  S(41, 41),
      S(14, 41),  S(49, 26),  S(41, 24),  S(53, 17),  S(-2, 37),  S(33, 39),
      S(31, 38),  S(41, 33),  S(79, 16),  S(72, 4),   S(116, 2),  S(68, 4),
      S(-12, 44), S(11, 36),  S(11, 49),  S(17, 40),  S(30, 18),  S(29, 10),
      S(28, 17),  S(28, 10),  S(-37, 36), S(-33, 41), S(-24, 40), S(-10, 34),
      S(-4, 27),  S(-34, 25), S(-3, 19),  S(-19, 14), S(-40, 30), S(-30, 28),
      S(-17, 22), S(-15, 27), S(-8, 21),  S(-11, 7),  S(28, -15), S(-6, -8),
      S(-45, 20), S(-25, 22), S(1, 20),   S(2, 19),   S(7, 6),    S(15, -3),
      S(41, -17), S(-25, -1), S(-15, 21), S(-13, 20), S(6, 25),   S(15, 20),
      S(22, 7),   S(10, 7),   S(11, 2),   S(-3, 1),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-81, 66), S(-50, 45),  S(-43, 84),  S(-4, 68),  S(-6, 49),
      S(6, 48),   S(75, -25),  S(-5, 43),   S(-17, 33), S(-48, 53),
      S(-33, 90), S(-61, 133), S(-67, 140), S(-8, 54),  S(-8, 43),
      S(60, 44),  S(-3, 47),   S(-17, 61),  S(-7, 87),  S(-15, 110),
      S(7, 88),   S(47, 52),   S(41, 6),    S(19, 58),  S(-24, 68),
      S(-18, 76), S(-13, 72),  S(-35, 105), S(-4, 95),  S(-5, 99),
      S(0, 84),   S(-1, 66),   S(-9, 36),   S(-26, 72), S(-31, 85),
      S(-11, 93), S(-15, 106), S(-14, 94),  S(-5, 69),  S(12, 65),
      S(-10, 17), S(-3, 50),   S(-3, 60),   S(-12, 77), S(-4, 99),
      S(12, 61),  S(19, 37),   S(15, 40),   S(-2, 8),   S(-6, 21),
      S(12, 29),  S(24, 37),   S(18, 50),   S(36, -4),  S(34, -24),
      S(47, -57), S(-9, 10),   S(2, 1),     S(17, 7),   S(28, 40),
      S(22, 17),  S(2, 4),     S(13, -14),  S(6, -20),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-50, -113), S(24, -65),  S(41, -38),   S(-73, -3),  S(-75, -18),
      S(-4, -7),    S(40, -3),   S(63, -140),  S(-81, -22), S(-14, 23),
      S(-88, 45),   S(34, 22),   S(0, 36),     S(-9, 64),   S(99, 37),
      S(57, -6),    S(-149, 10), S(22, 32),    S(-50, 60),  S(-75, 66),
      S(-50, 76),   S(66, 68),   S(60, 55),    S(12, 12),   S(-73, -13),
      S(-53, 31),   S(-98, 64),  S(-218, 81),  S(-195, 74), S(-125, 69),
      S(-112, 50),  S(-156, 23), S(-133, -23), S(-84, 14),  S(-146, 52),
      S(-225, 71),  S(-229, 69), S(-118, 44),  S(-129, 25), S(-209, 16),
      S(-56, -35),  S(-11, -2),  S(-82, 27),   S(-143, 42), S(-123, 36),
      S(-91, 30),   S(-22, -3),  S(-85, -13),  S(66, -63),  S(25, -21),
      S(-1, 0),     S(-68, 5),   S(-67, 9),    S(-23, 8),   S(48, -21),
      S(45, -51),   S(26, -113), S(64, -71),   S(21, -31),  S(-98, -36),
      S(2, -59),    S(-51, -22), S(49, -63),   S(36, -117),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),   S(-4, 6),  S(-4, 16),  S(-7, 57),
      S(9, 102), S(4, 197), S(70, 217), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-4, -36),   S(-9, -25),  S(2, -40),
      S(21, -73),  S(-71, -109), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-22, -9), S(-28, -17), S(-24, -16),
      S(-5, -31), S(9, -36),  S(28, -7),   S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-19, -17), S(-5, -6),   S(-11, -3),
      S(-19, -11), S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-43, -40), S(-15, -1), S(0, 13),  S(13, 24), S(25, 42),
      S(40, 39),   S(52, 39),  S(59, 23), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-32, -72), S(-6, -38), S(5, -12), S(20, 3),  S(29, 19),
      S(35, 30),   S(38, 33),  S(41, 40), S(44, 39), S(53, 32),
      S(57, 29),   S(29, 40),  S(54, 15), S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-48, -75), S(-32, -49), S(-33, -29), S(-26, -18), S(-26, -8),
      S(-17, 5),   S(-6, 9),    S(2, 17),    S(17, 22),   S(20, 27),
      S(29, 34),   S(32, 42),   S(34, 40),   S(22, 45),   S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-157, -207), S(-60, -135), S(-45, -116), S(-28, -89), S(-19, -75),
      S(-10, -61),   S(-6, -22),   S(3, -10),    S(9, -1),    S(16, 8),
      S(22, 11),     S(26, 15),    S(30, 27),    S(30, 31),   S(37, 33),
      S(32, 51),     S(30, 60),    S(34, 60),    S(49, 60),   S(41, 68),
      S(75, 43),     S(85, 50),    S(69, 44),    S(101, 47),  S(68, 40),
      S(34, 66),     S(16, 75),    S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-2, 43), S(-3, 28), S(-17, 21), S(-22, 22), S(-2, -17),
};
constexpr Score trappedRookWeight = {
      S(-6, 14),
};
constexpr Score rookOnOpenFile[2] = {
      S(21, 18),
      S(28, 28),
};
constexpr Score openFileNextToKing[3] = {
      S(-1, -19),
      S(-18, -15),
      S(-53, -40),
};
constexpr Score pawnShieldStrength[4] = {
      S(-81, -12),
      S(-28, -27),
      S(16, -53),
      S(48, -94),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(11, 14),
      S(-50, -50),
      S(-5, -5),
      S(100, 100),
};
constexpr Score kingAttackersDistance[8] = {
      S(-20, -14), S(-26, -28), S(-18, -23), S(-6, -16),
      S(-8, -19),  S(-20, -9),  S(-23, -5),  S(0, 0),
};
constexpr Score xRayKingAttackersDistance[8] = {
      S(0, 0),     S(-33, -29), S(-22, -23), S(-20, -34),
      S(-14, -23), S(-21, -24), S(-32, -12), S(0, 0),
};
constexpr Score xRayKingAttackPieceWeight[5] = {
      S(-23, 22), S(-16, 6), S(-29, 8), S(0, 12), S(-17, 40),
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
    int hangingPieceWeight[5][NUM_COLOR] = {{0}};
};

struct EvalWeights {
    const Score pawnScore = S(107, 164);
    const Score knightScore = S(432, 528);
    const Score bishopScore = S(477, 556);
    const Score rookScore = S(634, 994);
    const Score queenScore = S(1282, 1788);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(8, 128),  S(26, 98),
          S(-14, 114),  S(30, 35),  S(-19, 49), S(-7, 50),  S(-135, 129),
          S(-176, 154), S(0, 104),  S(4, 107),  S(45, 44),  S(46, -13),
          S(44, -13),   S(109, 17), S(53, 77),  S(15, 82),  S(-21, 68),
          S(-14, 49),   S(5, 24),   S(12, -10), S(41, -4),  S(43, 10),
          S(17, 33),    S(17, 28),  S(-36, 32), S(-27, 28), S(-9, 9),
          S(7, -4),     S(10, -3),  S(9, 6),    S(0, 12),   S(-8, 4),
          S(-28, 23),   S(-19, 19), S(-6, 10),  S(1, 10),   S(20, 11),
          S(8, 11),     S(31, 2),   S(13, -5),  S(-26, 28), S(-20, 26),
          S(-9, 15),    S(0, 11),   S(12, 21),  S(16, 21),  S(28, 8),
          S(-22, 8),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-172, -82), S(-142, -25), S(-67, 3),    S(-8, -18),  S(14, -3),
          S(-75, -20),  S(-92, -18),  S(-97, -110), S(-8, -6),   S(21, 2),
          S(52, -7),    S(72, 1),     S(65, -14),   S(112, -24), S(26, -6),
          S(75, -39),   S(0, 3),      S(43, 3),     S(67, 27),   S(72, 32),
          S(96, 21),    S(115, -1),   S(54, -12),   S(39, -14),  S(15, 14),
          S(13, 25),    S(45, 52),    S(86, 50),    S(41, 55),   S(79, 46),
          S(4, 28),     S(56, 6),     S(1, 24),     S(15, 17),   S(23, 52),
          S(36, 49),    S(49, 56),    S(41, 37),    S(57, 13),   S(25, 15),
          S(-27, -8),   S(-15, 5),    S(3, 16),     S(11, 41),   S(31, 38),
          S(15, 7),     S(17, -2),    S(11, -1),    S(-39, -8),  S(-21, 8),
          S(-14, 2),    S(5, 8),      S(16, 5),     S(15, -2),   S(3, -11),
          S(4, -1),     S(-89, -11),  S(-16, -17),  S(-35, -4),  S(-7, 2),
          S(2, -1),     S(0, -10),    S(-11, -8),   S(-37, -6),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-10, 7),  S(-74, 21), S(-56, 9),  S(-114, 27), S(-92, 14),
          S(-94, 7),  S(-39, 8),  S(-50, -2), S(-15, -6),  S(26, 0),
          S(10, 6),   S(-20, 9),  S(18, -6),  S(2, -2),    S(1, 11),
          S(-17, -8), S(-3, 27),  S(32, 9),   S(30, 18),   S(37, 8),
          S(18, 14),  S(55, 21),  S(15, 10),  S(2, 23),    S(-12, 16),
          S(15, 25),  S(19, 24),  S(32, 40),  S(30, 35),   S(28, 28),
          S(8, 22),   S(-26, 16), S(0, 8),    S(-10, 31),  S(2, 35),
          S(40, 38),  S(33, 35),  S(16, 30),  S(-4, 25),   S(31, -3),
          S(0, 21),   S(26, 20),  S(20, 27),  S(22, 32),   S(34, 38),
          S(29, 25),  S(29, 14),  S(34, 3),   S(19, 21),   S(23, 5),
          S(38, 1),   S(9, 21),   S(21, 22),  S(43, 8),    S(56, 5),
          S(29, 1),   S(0, 0),    S(41, 25),  S(19, 18),   S(-3, 16),
          S(9, 10),   S(13, 28),  S(32, -5),  S(34, -21),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(13, 41),  S(22, 41),  S(-12, 65), S(-11, 51), S(26, 39),
          S(45, 33),  S(49, 33),  S(80, 24),  S(0, 38),   S(-9, 55),
          S(10, 61),  S(41, 41),  S(14, 41),  S(49, 26),  S(41, 24),
          S(53, 17),  S(-2, 37),  S(33, 39),  S(31, 38),  S(41, 33),
          S(79, 16),  S(72, 4),   S(116, 2),  S(68, 4),   S(-12, 44),
          S(11, 36),  S(11, 49),  S(17, 40),  S(30, 18),  S(29, 10),
          S(28, 17),  S(28, 10),  S(-37, 36), S(-33, 41), S(-24, 40),
          S(-10, 34), S(-4, 27),  S(-34, 25), S(-3, 19),  S(-19, 14),
          S(-40, 30), S(-30, 28), S(-17, 22), S(-15, 27), S(-8, 21),
          S(-11, 7),  S(28, -15), S(-6, -8),  S(-45, 20), S(-25, 22),
          S(1, 20),   S(2, 19),   S(7, 6),    S(15, -3),  S(41, -17),
          S(-25, -1), S(-15, 21), S(-13, 20), S(6, 25),   S(15, 20),
          S(22, 7),   S(10, 7),   S(11, 2),   S(-3, 1),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-81, 66), S(-50, 45),  S(-43, 84),  S(-4, 68),  S(-6, 49),
          S(6, 48),   S(75, -25),  S(-5, 43),   S(-17, 33), S(-48, 53),
          S(-33, 90), S(-61, 133), S(-67, 140), S(-8, 54),  S(-8, 43),
          S(60, 44),  S(-3, 47),   S(-17, 61),  S(-7, 87),  S(-15, 110),
          S(7, 88),   S(47, 52),   S(41, 6),    S(19, 58),  S(-24, 68),
          S(-18, 76), S(-13, 72),  S(-35, 105), S(-4, 95),  S(-5, 99),
          S(0, 84),   S(-1, 66),   S(-9, 36),   S(-26, 72), S(-31, 85),
          S(-11, 93), S(-15, 106), S(-14, 94),  S(-5, 69),  S(12, 65),
          S(-10, 17), S(-3, 50),   S(-3, 60),   S(-12, 77), S(-4, 99),
          S(12, 61),  S(19, 37),   S(15, 40),   S(-2, 8),   S(-6, 21),
          S(12, 29),  S(24, 37),   S(18, 50),   S(36, -4),  S(34, -24),
          S(47, -57), S(-9, 10),   S(2, 1),     S(17, 7),   S(28, 40),
          S(22, 17),  S(2, 4),     S(13, -14),  S(6, -20),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-50, -113), S(24, -65),  S(41, -38),   S(-73, -3),  S(-75, -18),
          S(-4, -7),    S(40, -3),   S(63, -140),  S(-81, -22), S(-14, 23),
          S(-88, 45),   S(34, 22),   S(0, 36),     S(-9, 64),   S(99, 37),
          S(57, -6),    S(-149, 10), S(22, 32),    S(-50, 60),  S(-75, 66),
          S(-50, 76),   S(66, 68),   S(60, 55),    S(12, 12),   S(-73, -13),
          S(-53, 31),   S(-98, 64),  S(-218, 81),  S(-195, 74), S(-125, 69),
          S(-112, 50),  S(-156, 23), S(-133, -23), S(-84, 14),  S(-146, 52),
          S(-225, 71),  S(-229, 69), S(-118, 44),  S(-129, 25), S(-209, 16),
          S(-56, -35),  S(-11, -2),  S(-82, 27),   S(-143, 42), S(-123, 36),
          S(-91, 30),   S(-22, -3),  S(-85, -13),  S(66, -63),  S(25, -21),
          S(-1, 0),     S(-68, 5),   S(-67, 9),    S(-23, 8),   S(48, -21),
          S(45, -51),   S(26, -113), S(64, -71),   S(21, -31),  S(-98, -36),
          S(2, -59),    S(-51, -22), S(49, -63),   S(36, -117),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),   S(-4, 6),  S(-4, 16),  S(-7, 57),
          S(9, 102), S(4, 197), S(70, 217), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-4, -36),   S(-9, -25),  S(2, -40),
          S(21, -73),  S(-71, -109), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-22, -9), S(-28, -17), S(-24, -16),
          S(-5, -31), S(9, -36),  S(28, -7),   S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-19, -17), S(-5, -6),   S(-11, -3),
          S(-19, -11), S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-43, -40), S(-15, -1), S(0, 13),  S(13, 24), S(25, 42),
          S(40, 39),   S(52, 39),  S(59, 23), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-32, -72), S(-6, -38), S(5, -12), S(20, 3),  S(29, 19),
          S(35, 30),   S(38, 33),  S(41, 40), S(44, 39), S(53, 32),
          S(57, 29),   S(29, 40),  S(54, 15), S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-48, -75), S(-32, -49), S(-33, -29), S(-26, -18), S(-26, -8),
          S(-17, 5),   S(-6, 9),    S(2, 17),    S(17, 22),   S(20, 27),
          S(29, 34),   S(32, 42),   S(34, 40),   S(22, 45),   S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-157, -207), S(-60, -135), S(-45, -116), S(-28, -89), S(-19, -75),
          S(-10, -61),   S(-6, -22),   S(3, -10),    S(9, -1),    S(16, 8),
          S(22, 11),     S(26, 15),    S(30, 27),    S(30, 31),   S(37, 33),
          S(32, 51),     S(30, 60),    S(34, 60),    S(49, 60),   S(41, 68),
          S(75, 43),     S(85, 50),    S(69, 44),    S(101, 47),  S(68, 40),
          S(34, 66),     S(16, 75),    S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-2, 43), S(-3, 28), S(-17, 21), S(-22, 22), S(-2, -17),
    };
    const Score trappedRookWeight = {
          S(-6, 14),
    };
    const Score rookOnOpenFile[2] = {
          S(21, 18),
          S(28, 28),
    };
    const Score openFileNextToKing[3] = {
          S(-1, -19),
          S(-18, -15),
          S(-53, -40),
    };
    const Score pawnShieldStrength[4] = {
          S(-81, -12),
          S(-28, -27),
          S(16, -53),
          S(48, -94),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(11, 14),
          S(-50, -50),
          S(-5, -5),
          S(100, 100),
    };
    const Score kingAttackersDistance[8] = {
          S(-20, -14), S(-26, -28), S(-18, -23), S(-6, -16),
          S(-8, -19),  S(-20, -9),  S(-23, -5),  S(0, 0),
    };
    const Score xRayKingAttackersDistance[8] = {
          S(0, 0),     S(-33, -29), S(-22, -23), S(-20, -34),
          S(-14, -23), S(-21, -24), S(-32, -12), S(0, 0),
    };
    const Score xRayKingAttackPieceWeight[5] = {
          S(-23, 22), S(-16, 6), S(-29, 8), S(0, 12), S(-17, 40),
    };
    // const Score hangingPieceWeight[5] = {
    //       S(-10, -10), S(-25, -25), S(-27, -27), S(-50, -50), S(-100, -100),
    // };
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
        mgScore += MgScore(xRayKingAttackPieceWeight[xRayPc]);
        egScore += EgScore(xRayKingAttackPieceWeight[xRayPc]);

        if (T) {
            trace.xRayKingAttackersDistance[distanceToKing][C]++;
            trace.xRayKingAttackPieceWeight[xRayPc][C]++;
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

            if (T == TRACE) {
                trace.kingAttackers[piece - 1][C]++;
                trace.kingAttackersDistance[popcount(
                      rectArray[kingSquare][attacker])][C]++;
            }

            mgScore += MgScore(kingAttackersWeight[piece - 1]);
            egScore += EgScore(kingAttackersWeight[piece - 1]);
            mgScore += MgScore(kingAttackersDistance[popcount(
                  rectArray[kingSquare][attacker])]);
            egScore += EgScore(kingAttackersDistance[popcount(
                  rectArray[kingSquare][attacker])]);

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
