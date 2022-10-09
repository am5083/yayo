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

constexpr Score pawnScore = S(96, 144);
constexpr Score knightScore = S(404, 493);
constexpr Score bishopScore = S(444, 519);
constexpr Score rookScore = S(585, 920);
constexpr Score queenScore = S(1215, 1667);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(18, 117), S(22, 93),
      S(-13, 108),  S(29, 37),  S(-29, 53), S(-13, 45), S(-141, 126),
      S(-167, 149), S(0, 99),   S(2, 103),  S(41, 43),  S(43, -10),
      S(49, -12),   S(102, 14), S(60, 72),  S(22, 74),  S(-21, 64),
      S(-13, 47),   S(4, 22),   S(10, -10), S(37, -4),  S(41, 9),
      S(17, 31),    S(19, 25),  S(-36, 32), S(-26, 28), S(-10, 9),
      S(4, -4),     S(7, -2),   S(9, 7),    S(2, 11),   S(-5, 4),
      S(-28, 23),   S(-19, 19), S(-8, 11),  S(-1, 12),  S(18, 11),
      S(8, 12),     S(31, 3),   S(13, -4),  S(-26, 26), S(-20, 24),
      S(-10, 13),   S(-1, 14),  S(8, 20),   S(16, 14),  S(28, 4),
      S(-18, 2),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-167, -78), S(-133, -33), S(-57, -3),   S(-2, -20),  S(21, -5),
      S(-80, -15),  S(-81, -20),  S(-92, -114), S(-15, -7),  S(18, 2),
      S(56, -12),   S(76, -4),    S(67, -13),   S(111, -19), S(34, -10),
      S(72, -34),   S(-7, 6),     S(44, -1),    S(69, 22),   S(77, 22),
      S(99, 19),    S(120, -2),   S(65, -15),   S(35, -12),  S(11, 10),
      S(11, 23),    S(40, 48),    S(76, 44),    S(37, 47),   S(74, 41),
      S(12, 26),    S(55, 0),     S(-1, 21),    S(12, 13),   S(20, 48),
      S(32, 41),    S(43, 52),    S(37, 34),    S(55, 11),   S(22, 13),
      S(-25, -11),  S(-15, 3),    S(2, 13),     S(10, 35),   S(30, 35),
      S(12, 5),     S(15, -4),    S(7, -3),     S(-43, -6),  S(-23, 8),
      S(-13, -1),   S(2, 5),      S(13, 2),     S(13, -5),   S(4, -19),
      S(1, -4),     S(-95, -17),  S(-17, -21),  S(-33, -7),  S(-11, 1),
      S(-1, -3),    S(-2, -13),   S(-14, -8),   S(-41, -13),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-2, 6),   S(-78, 19), S(-38, 6),  S(-106, 22), S(-71, 5), S(-87, 7),
      S(-31, 8),  S(-45, -2), S(-11, -7), S(27, -3),   S(10, 2),  S(-17, 9),
      S(24, -9),  S(7, -3),   S(17, 4),   S(-12, -10), S(-3, 22), S(32, 6),
      S(33, 12),  S(36, 6),   S(29, 9),   S(56, 17),   S(33, 8),  S(7, 17),
      S(-12, 11), S(15, 19),  S(17, 22),  S(41, 37),   S(29, 33), S(30, 26),
      S(14, 21),  S(-18, 14), S(-2, 6),   S(-11, 28),  S(3, 31),  S(38, 33),
      S(29, 33),  S(12, 27),  S(-1, 25),  S(28, -6),   S(-1, 18), S(22, 18),
      S(17, 23),  S(17, 27),  S(30, 31),  S(22, 22),   S(27, 14), S(29, 2),
      S(18, 15),  S(18, 2),   S(33, 0),   S(7, 17),    S(16, 16), S(38, 6),
      S(48, 4),   S(24, 5),   S(0, -4),   S(35, 24),   S(14, 16), S(-4, 14),
      S(6, 9),    S(9, 24),   S(32, -6),  S(29, -14),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(18, 38),  S(37, 33),  S(2, 58),   S(8, 42),   S(42, 32),  S(46, 33),
      S(61, 28),  S(87, 20),  S(1, 33),   S(-6, 48),  S(12, 53),  S(47, 33),
      S(17, 36),  S(52, 23),  S(38, 21),  S(45, 19),  S(0, 30),   S(33, 33),
      S(30, 31),  S(37, 31),  S(76, 14),  S(71, 4),   S(118, 3),  S(67, 5),
      S(-13, 39), S(17, 28),  S(9, 43),   S(21, 35),  S(30, 15),  S(27, 10),
      S(32, 17),  S(28, 7),   S(-34, 30), S(-30, 35), S(-18, 36), S(-9, 29),
      S(-2, 24),  S(-32, 25), S(0, 19),   S(-18, 15), S(-37, 24), S(-29, 27),
      S(-18, 19), S(-14, 23), S(-9, 20),  S(-12, 7),  S(28, -14), S(-6, -8),
      S(-43, 17), S(-22, 17), S(-2, 18),  S(-1, 17),  S(3, 6),    S(10, -4),
      S(35, -13), S(-29, -1), S(-15, 17), S(-13, 17), S(3, 20),   S(10, 17),
      S(17, 5),   S(5, 5),    S(7, 0),    S(-7, 0),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-73, 65),  S(-41, 36),  S(-37, 76), S(1, 66),   S(7, 44),   S(17, 44),
      S(85, -17),  S(3, 47),    S(-18, 34), S(-43, 45), S(-28, 82), S(-52, 124),
      S(-55, 134), S(7, 50),    S(-3, 45),  S(60, 46),  S(-4, 42),  S(-15, 54),
      S(-4, 78),   S(-15, 103), S(17, 79),  S(50, 49),  S(62, 6),   S(20, 62),
      S(-22, 58),  S(-14, 59),  S(-12, 63), S(-22, 96), S(3, 84),   S(-4, 97),
      S(12, 83),   S(6, 56),    S(-8, 26),  S(-23, 58), S(-25, 76), S(-11, 89),
      S(-16, 100), S(-15, 90),  S(2, 64),   S(10, 65),  S(-12, 16), S(-5, 43),
      S(-4, 51),   S(-12, 65),  S(-6, 91),  S(10, 46),  S(17, 37),  S(11, 37),
      S(-4, 4),    S(-8, 13),   S(8, 19),   S(20, 24),  S(14, 39),  S(29, -6),
      S(27, -17),  S(40, -60),  S(-9, 5),   S(-1, -1),  S(14, 0),   S(23, 25),
      S(17, 13),   S(-5, 11),   S(4, -20),  S(-1, -13),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-54, -108), S(30, -64),  S(35, -36),   S(-58, -10), S(-65, -15),
      S(-11, -7),   S(45, 7),    S(43, -130),  S(-64, -21), S(-18, 21),
      S(-90, 41),   S(22, 20),   S(1, 28),     S(-12, 59),  S(97, 38),
      S(62, -9),    S(-139, 10), S(2, 31),     S(-47, 55),  S(-63, 61),
      S(-45, 69),   S(51, 68),   S(60, 54),    S(20, 10),   S(-56, -11),
      S(-41, 30),   S(-100, 60), S(-204, 72),  S(-182, 68), S(-126, 64),
      S(-111, 51),  S(-133, 19), S(-142, -15), S(-84, 16),  S(-146, 51),
      S(-205, 66),  S(-223, 64), S(-127, 43),  S(-135, 26), S(-198, 17),
      S(-53, -31),  S(-30, 2),   S(-85, 26),   S(-143, 40), S(-118, 33),
      S(-99, 28),   S(-32, -2),  S(-86, -11),  S(61, -57),  S(19, -18),
      S(-5, 1),     S(-66, 6),   S(-67, 9),    S(-28, 6),   S(40, -20),
      S(41, -47),   S(35, -112), S(66, -72),   S(25, -35),  S(-89, -30),
      S(2, -50),    S(-43, -22), S(49, -61),   S(40, -109),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),  S(-4, 6),  S(-2, 14),  S(-6, 51),
      S(6, 93), S(0, 181), S(68, 203), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-3, -32),  S(-7, -24),  S(2, -36),
      S(18, -66),  S(-66, -92), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-18, -8), S(-24, -16), S(-21, -14),
      S(-4, -29), S(6, -36),  S(22, -11),  S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-18, -15), S(-4, -6),   S(-9, -3),
      S(-19, -9),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-37, -42), S(-15, -6), S(-3, 5),  S(9, 14),  S(19, 30),
      S(31, 27),   S(42, 27),  S(51, 13), S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-29, -74), S(-7, -41), S(2, -18), S(15, -4), S(22, 11),
      S(27, 21),   S(31, 24),  S(33, 31), S(36, 29), S(46, 25),
      S(49, 22),   S(16, 36),  S(45, 6),  S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-47, -69), S(-33, -47), S(-33, -34), S(-26, -23), S(-26, -15),
      S(-18, -3),  S(-9, 1),    S(-2, 8),    S(12, 12),   S(15, 16),
      S(22, 23),   S(25, 32),   S(23, 33),   S(4, 40),    S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-137, -178), S(-56, -111), S(-39, -100), S(-26, -73), S(-17, -71),
      S(-7, -74),    S(-3, -34),   S(5, -28),    S(11, -21),  S(16, -9),
      S(21, -7),     S(25, -2),    S(28, 8),     S(25, 14),   S(32, 17),
      S(26, 36),     S(24, 45),    S(25, 48),    S(35, 57),   S(22, 67),
      S(56, 39),     S(59, 54),    S(37, 45),    S(81, 47),   S(46, 36),
      S(43, 83),     S(31, 100),   S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-22, 21), S(-15, 6), S(-29, 7), S(-36, 8), S(-20, -21),
};
constexpr Score trappedRookWeight = {
      S(-5, 3),
};
constexpr Score rookOnOpenFile[2] = {
      S(19, 7),
      S(30, 17),
};
constexpr Score openFileNextToKing[3] = {
      S(-2, -19),
      S(-17, -15),
      S(-54, -38),
};
constexpr Score pawnShieldStrength[4] = {
      S(-75, -18),
      S(-26, -28),
      S(14, -45),
      S(40, -73),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(7, 20),
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
    const Score pawnScore = S(96, 144);
    const Score knightScore = S(404, 493);
    const Score bishopScore = S(444, 519);
    const Score rookScore = S(585, 920);
    const Score queenScore = S(1215, 1667);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(18, 117), S(22, 93),
          S(-13, 108),  S(29, 37),  S(-29, 53), S(-13, 45), S(-141, 126),
          S(-167, 149), S(0, 99),   S(2, 103),  S(41, 43),  S(43, -10),
          S(49, -12),   S(102, 14), S(60, 72),  S(22, 74),  S(-21, 64),
          S(-13, 47),   S(4, 22),   S(10, -10), S(37, -4),  S(41, 9),
          S(17, 31),    S(19, 25),  S(-36, 32), S(-26, 28), S(-10, 9),
          S(4, -4),     S(7, -2),   S(9, 7),    S(2, 11),   S(-5, 4),
          S(-28, 23),   S(-19, 19), S(-8, 11),  S(-1, 12),  S(18, 11),
          S(8, 12),     S(31, 3),   S(13, -4),  S(-26, 26), S(-20, 24),
          S(-10, 13),   S(-1, 14),  S(8, 20),   S(16, 14),  S(28, 4),
          S(-18, 2),    S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-167, -78), S(-133, -33), S(-57, -3),   S(-2, -20),  S(21, -5),
          S(-80, -15),  S(-81, -20),  S(-92, -114), S(-15, -7),  S(18, 2),
          S(56, -12),   S(76, -4),    S(67, -13),   S(111, -19), S(34, -10),
          S(72, -34),   S(-7, 6),     S(44, -1),    S(69, 22),   S(77, 22),
          S(99, 19),    S(120, -2),   S(65, -15),   S(35, -12),  S(11, 10),
          S(11, 23),    S(40, 48),    S(76, 44),    S(37, 47),   S(74, 41),
          S(12, 26),    S(55, 0),     S(-1, 21),    S(12, 13),   S(20, 48),
          S(32, 41),    S(43, 52),    S(37, 34),    S(55, 11),   S(22, 13),
          S(-25, -11),  S(-15, 3),    S(2, 13),     S(10, 35),   S(30, 35),
          S(12, 5),     S(15, -4),    S(7, -3),     S(-43, -6),  S(-23, 8),
          S(-13, -1),   S(2, 5),      S(13, 2),     S(13, -5),   S(4, -19),
          S(1, -4),     S(-95, -17),  S(-17, -21),  S(-33, -7),  S(-11, 1),
          S(-1, -3),    S(-2, -13),   S(-14, -8),   S(-41, -13),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-2, 6),   S(-78, 19), S(-38, 6),  S(-106, 22), S(-71, 5), S(-87, 7),
          S(-31, 8),  S(-45, -2), S(-11, -7), S(27, -3),   S(10, 2),  S(-17, 9),
          S(24, -9),  S(7, -3),   S(17, 4),   S(-12, -10), S(-3, 22), S(32, 6),
          S(33, 12),  S(36, 6),   S(29, 9),   S(56, 17),   S(33, 8),  S(7, 17),
          S(-12, 11), S(15, 19),  S(17, 22),  S(41, 37),   S(29, 33), S(30, 26),
          S(14, 21),  S(-18, 14), S(-2, 6),   S(-11, 28),  S(3, 31),  S(38, 33),
          S(29, 33),  S(12, 27),  S(-1, 25),  S(28, -6),   S(-1, 18), S(22, 18),
          S(17, 23),  S(17, 27),  S(30, 31),  S(22, 22),   S(27, 14), S(29, 2),
          S(18, 15),  S(18, 2),   S(33, 0),   S(7, 17),    S(16, 16), S(38, 6),
          S(48, 4),   S(24, 5),   S(0, -4),   S(35, 24),   S(14, 16), S(-4, 14),
          S(6, 9),    S(9, 24),   S(32, -6),  S(29, -14),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(18, 38),  S(37, 33),  S(2, 58),   S(8, 42),   S(42, 32),
          S(46, 33),  S(61, 28),  S(87, 20),  S(1, 33),   S(-6, 48),
          S(12, 53),  S(47, 33),  S(17, 36),  S(52, 23),  S(38, 21),
          S(45, 19),  S(0, 30),   S(33, 33),  S(30, 31),  S(37, 31),
          S(76, 14),  S(71, 4),   S(118, 3),  S(67, 5),   S(-13, 39),
          S(17, 28),  S(9, 43),   S(21, 35),  S(30, 15),  S(27, 10),
          S(32, 17),  S(28, 7),   S(-34, 30), S(-30, 35), S(-18, 36),
          S(-9, 29),  S(-2, 24),  S(-32, 25), S(0, 19),   S(-18, 15),
          S(-37, 24), S(-29, 27), S(-18, 19), S(-14, 23), S(-9, 20),
          S(-12, 7),  S(28, -14), S(-6, -8),  S(-43, 17), S(-22, 17),
          S(-2, 18),  S(-1, 17),  S(3, 6),    S(10, -4),  S(35, -13),
          S(-29, -1), S(-15, 17), S(-13, 17), S(3, 20),   S(10, 17),
          S(17, 5),   S(5, 5),    S(7, 0),    S(-7, 0),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-73, 65), S(-41, 36),  S(-37, 76),  S(1, 66),   S(7, 44),
          S(17, 44),  S(85, -17),  S(3, 47),    S(-18, 34), S(-43, 45),
          S(-28, 82), S(-52, 124), S(-55, 134), S(7, 50),   S(-3, 45),
          S(60, 46),  S(-4, 42),   S(-15, 54),  S(-4, 78),  S(-15, 103),
          S(17, 79),  S(50, 49),   S(62, 6),    S(20, 62),  S(-22, 58),
          S(-14, 59), S(-12, 63),  S(-22, 96),  S(3, 84),   S(-4, 97),
          S(12, 83),  S(6, 56),    S(-8, 26),   S(-23, 58), S(-25, 76),
          S(-11, 89), S(-16, 100), S(-15, 90),  S(2, 64),   S(10, 65),
          S(-12, 16), S(-5, 43),   S(-4, 51),   S(-12, 65), S(-6, 91),
          S(10, 46),  S(17, 37),   S(11, 37),   S(-4, 4),   S(-8, 13),
          S(8, 19),   S(20, 24),   S(14, 39),   S(29, -6),  S(27, -17),
          S(40, -60), S(-9, 5),    S(-1, -1),   S(14, 0),   S(23, 25),
          S(17, 13),  S(-5, 11),   S(4, -20),   S(-1, -13),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-54, -108), S(30, -64),  S(35, -36),   S(-58, -10), S(-65, -15),
          S(-11, -7),   S(45, 7),    S(43, -130),  S(-64, -21), S(-18, 21),
          S(-90, 41),   S(22, 20),   S(1, 28),     S(-12, 59),  S(97, 38),
          S(62, -9),    S(-139, 10), S(2, 31),     S(-47, 55),  S(-63, 61),
          S(-45, 69),   S(51, 68),   S(60, 54),    S(20, 10),   S(-56, -11),
          S(-41, 30),   S(-100, 60), S(-204, 72),  S(-182, 68), S(-126, 64),
          S(-111, 51),  S(-133, 19), S(-142, -15), S(-84, 16),  S(-146, 51),
          S(-205, 66),  S(-223, 64), S(-127, 43),  S(-135, 26), S(-198, 17),
          S(-53, -31),  S(-30, 2),   S(-85, 26),   S(-143, 40), S(-118, 33),
          S(-99, 28),   S(-32, -2),  S(-86, -11),  S(61, -57),  S(19, -18),
          S(-5, 1),     S(-66, 6),   S(-67, 9),    S(-28, 6),   S(40, -20),
          S(41, -47),   S(35, -112), S(66, -72),   S(25, -35),  S(-89, -30),
          S(2, -50),    S(-43, -22), S(49, -61),   S(40, -109),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),  S(-4, 6),  S(-2, 14),  S(-6, 51),
          S(6, 93), S(0, 181), S(68, 203), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-3, -32),  S(-7, -24),  S(2, -36),
          S(18, -66),  S(-66, -92), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-18, -8), S(-24, -16), S(-21, -14),
          S(-4, -29), S(6, -36),  S(22, -11),  S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-18, -15), S(-4, -6),   S(-9, -3),
          S(-19, -9),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-37, -42), S(-15, -6), S(-3, 5),  S(9, 14),  S(19, 30),
          S(31, 27),   S(42, 27),  S(51, 13), S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-29, -74), S(-7, -41), S(2, -18), S(15, -4), S(22, 11),
          S(27, 21),   S(31, 24),  S(33, 31), S(36, 29), S(46, 25),
          S(49, 22),   S(16, 36),  S(45, 6),  S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-47, -69), S(-33, -47), S(-33, -34), S(-26, -23), S(-26, -15),
          S(-18, -3),  S(-9, 1),    S(-2, 8),    S(12, 12),   S(15, 16),
          S(22, 23),   S(25, 32),   S(23, 33),   S(4, 40),    S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-137, -178), S(-56, -111), S(-39, -100), S(-26, -73), S(-17, -71),
          S(-7, -74),    S(-3, -34),   S(5, -28),    S(11, -21),  S(16, -9),
          S(21, -7),     S(25, -2),    S(28, 8),     S(25, 14),   S(32, 17),
          S(26, 36),     S(24, 45),    S(25, 48),    S(35, 57),   S(22, 67),
          S(56, 39),     S(59, 54),    S(37, 45),    S(81, 47),   S(46, 36),
          S(43, 83),     S(31, 100),   S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-22, 21), S(-15, 6), S(-29, 7), S(-36, 8), S(-20, -21),
    };
    const Score trappedRookWeight = {
          S(-5, 3),
    };
    const Score rookOnOpenFile[2] = {
          S(19, 7),
          S(30, 17),
    };
    const Score openFileNextToKing[3] = {
          S(-2, -19),
          S(-17, -15),
          S(-54, -38),
    };
    const Score pawnShieldStrength[4] = {
          S(-75, -18),
          S(-26, -28),
          S(14, -45),
          S(40, -73),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(7, 20),
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
