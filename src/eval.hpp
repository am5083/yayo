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

constexpr Score pawnScore = S(7, -26);
constexpr Score knightScore = S(122, 483);
constexpr Score bishopScore = S(136, 562);
constexpr Score rookScore = S(127, 562);
constexpr Score queenScore = S(862, 1375);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),     S(39, 83),  S(-6, 160),
      S(58, 26),    S(-40, -75), S(-96, 51),  S(-7, 59),  S(-144, 182),
      S(-297, 159), S(-20, 17),  S(-11, 36),  S(53, -30), S(-41, -105),
      S(13, -81),   S(41, -82),  S(-96, 83),  S(-58, 31), S(-31, 28),
      S(-17, 42),   S(-25, 24),  S(-31, -25), S(-10, 13), S(-4, 23),
      S(21, 36),    S(-34, 42),  S(-23, 4),   S(-8, 7),   S(31, 33),
      S(-12, -10),  S(12, 14),   S(31, 21),   S(16, 1),   S(-20, 39),
      S(-12, -1),   S(-19, -31), S(7, -19),   S(29, 2),   S(20, -11),
      S(26, 14),    S(30, -9),   S(-36, 3),   S(-2, -5),  S(-29, -4),
      S(-15, -6),   S(4, 98),    S(25, 30),   S(2, 2),    S(33, -26),
      S(-29, -7),   S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-131, -65), S(-132, 93),  S(-156, -59), S(97, 2),    S(23, 5),
      S(-134, 44),  S(-161, 49),  S(8, -205),   S(63, -197), S(5, -38),
      S(37, 26),    S(-49, 14),   S(-27, 2),    S(73, -10),  S(-64, -13),
      S(108, 97),   S(-103, 2),   S(64, 82),    S(-12, -43), S(-2, 63),
      S(-37, 24),   S(-114, 112), S(-59, -44),  S(12, 83),   S(-30, -39),
      S(0, -36),    S(-3, 27),    S(-16, 51),   S(5, 21),    S(24, 15),
      S(25, 49),    S(31, 49),    S(-31, 163),  S(-12, 22),  S(-18, -30),
      S(-10, 54),   S(-16, 25),   S(-1, 59),    S(25, 64),   S(29, -107),
      S(-14, 27),   S(-12, -1),   S(-16, -19),  S(29, -31),  S(6, 35),
      S(-17, 21),   S(-11, 50),   S(7, 16),     S(-71, -55), S(-174, 109),
      S(20, -57),   S(-23, 43),   S(-7, 24),    S(24, -38),  S(-43, -162),
      S(-50, 78),   S(-79, -34),  S(34, -21),   S(114, 14),  S(-40, -82),
      S(46, -64),   S(18, -77),   S(24, 26),    S(68, 2),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-277, 32),  S(-194, 1),   S(-241, -3), S(-247, -5),  S(-234, 0),
      S(-331, 33),  S(-38, -68),  S(-172, 84), S(108, -49),  S(-61, 29),
      S(-103, 89),  S(-150, -74), S(-49, 66),  S(-133, -39), S(-158, 43),
      S(-29, -100), S(-48, 26),   S(-11, 14),  S(-87, 41),   S(-36, 52),
      S(-81, 45),   S(-145, 82),  S(-101, 32), S(22, -6),    S(-29, 10),
      S(-16, 29),   S(24, -14),   S(-38, 39),  S(-23, 10),   S(-45, -32),
      S(1, 102),    S(-78, 11),   S(-54, 4),   S(-40, 83),   S(-18, -25),
      S(-12, 83),   S(0, 22),     S(2, 40),    S(-34, 8),    S(48, -38),
      S(33, -38),   S(-6, 11),    S(-13, 32),  S(6, 42),     S(-11, 35),
      S(1, 91),     S(9, 12),     S(44, -21),  S(56, 31),    S(28, -23),
      S(7, -3),     S(32, 21),    S(10, 34),   S(95, 21),    S(17, -13),
      S(-7, 202),   S(48, -27),   S(207, 9),   S(20, -8),    S(-53, -21),
      S(-82, 39),   S(41, 2),     S(7, 6),     S(58, -59),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(-161, 53), S(-40, 60),  S(-71, 37), S(-78, 32),  S(-32, 25),
      S(-52, 70),  S(10, 62),   S(111, 27), S(-2, -2),   S(58, -23),
      S(-22, 20),  S(45, -4),   S(-29, 33), S(35, -15),  S(29, -44),
      S(-16, -11), S(-64, 41),  S(46, 16),  S(-40, 15),  S(-54, 38),
      S(12, -32),  S(4, 22),    S(84, -20), S(181, -77), S(18, -38),
      S(-3, -12),  S(-52, -8),  S(5, -63),  S(-71, -14), S(19, -17),
      S(9, -24),   S(58, -58),  S(-17, 8),  S(-82, 22),  S(-148, 72),
      S(-69, 14),  S(-15, -24), S(-79, 9),  S(-27, -46), S(-77, 34),
      S(-117, 2),  S(-12, -24), S(4, 16),   S(18, -4),   S(3, -10),
      S(-8, 14),   S(-7, 0),    S(-78, -2), S(-89, 25),  S(-22, -56),
      S(26, -52),  S(-12, 17),  S(-73, 38), S(-11, 28),  S(55, -6),
      S(-18, -53), S(-23, -59), S(3, 11),   S(-10, -36), S(1, -29),
      S(-13, -11), S(-18, -4),  S(14, -48), S(5, -11),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(77, 184),  S(-84, -4),   S(-47, -24),  S(-109, 126), S(71, 55),
      S(-43, -41), S(128, -162), S(-71, -12),  S(-10, 93),   S(-25, 39),
      S(-62, 123), S(-126, 160), S(-158, 205), S(45, 25),    S(81, 53),
      S(59, 112),  S(3, -4),     S(-21, 22),   S(9, 21),     S(-76, 104),
      S(28, 24),   S(5, 100),    S(37, -77),   S(51, 44),    S(-44, 25),
      S(-29, 58),  S(-16, 35),   S(-67, 133),  S(-46, 78),   S(-63, 150),
      S(-9, 92),   S(-1, -35),   S(5, 35),     S(-65, 76),   S(-46, 185),
      S(-24, 156), S(-21, 76),   S(40, 43),    S(-20, 68),   S(19, 22),
      S(-5, -27),  S(-17, 12),   S(-29, 84),   S(-14, 90),   S(7, 47),
      S(-8, 123),  S(27, 2),     S(-26, 78),   S(17, 6),     S(-65, 62),
      S(12, -32),  S(46, -48),   S(15, 36),    S(27, -3),    S(19, -32),
      S(-65, 62),  S(65, 30),    S(39, -22),   S(29, 33),    S(29, 22),
      S(71, -164), S(100, 27),   S(15, -19),   S(-57, -66),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-125, -110), S(105, -83),  S(61, -96),  S(-137, -116), S(-176, -65),
      S(-146, -141), S(-30, 0),    S(129, -75), S(-150, -143), S(-89, 73),
      S(-163, 57),   S(4, 3),      S(93, -57),  S(-46, -24),   S(-25, -17),
      S(82, -64),    S(-56, -16),  S(93, 103),  S(167, 35),    S(28, 7),
      S(-130, 28),   S(-16, 3),    S(37, 5),    S(35, 12),     S(-125, 25),
      S(50, 68),     S(-229, 48),  S(-318, 26), S(-280, 91),   S(-319, 51),
      S(-9, -77),    S(-15, 16),   S(-138, 21), S(-119, 24),   S(-97, 60),
      S(-265, 70),   S(-218, 55),  S(-190, 31), S(-201, 26),   S(-226, 5),
      S(29, -60),    S(-30, 31),   S(-215, 62), S(-146, 57),   S(-149, 63),
      S(-137, 13),   S(-96, -5),   S(-87, 15),  S(3, 8),       S(-71, 13),
      S(-1, 44),     S(-41, 41),   S(-59, 39),  S(-35, -3),    S(45, -56),
      S(32, -17),    S(143, -140), S(36, -27),  S(32, -14),    S(-106, 6),
      S(46, -23),    S(37, -1),    S(5, -65),   S(55, 0),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),    S(-5, -43),  S(-27, -19), S(-12, 4),
      S(-49, 51), S(-34, 130), S(-7, 153),  S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10),  S(-7, -36),    S(-3, -28),  S(1, -92),
      S(-53, -231), S(-108, -212), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(16, -31),  S(-1, -27),  S(10, -22),
      S(-5, -52), S(-1, -110), S(65, -174), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(1, -7),    S(-3, -16),  S(7, -39),
      S(-12, 26),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-32, 12), S(-8, 27),  S(-18, 19), S(-39, -6), S(-46, 10),
      S(-38, 20), S(-44, 26), S(-7, -6),  S(40, 30),
};
constexpr Score BishopMobilityScore[14] = {
      S(-20, -39), S(-21, 13), S(-5, 30),  S(-47, 39), S(-36, 47),
      S(-37, 23),  S(-26, 33), S(-25, 16), S(0, 9),    S(-21, 7),
      S(118, -8),  S(138, 22), S(-82, 2),  S(96, 98),
};
constexpr Score RookMobilityScore[15] = {
      S(-35, -98), S(-25, -32), S(-43, -16), S(-41, 3),   S(-39, -41),
      S(-38, -18), S(-41, -45), S(-46, -49), S(-46, -70), S(-46, -55),
      S(-42, -66), S(-29, -43), S(23, -42),  S(-1, -83),  S(70, 175),
};
constexpr Score QueenMobilityScore[28] = {
      S(-136, -271), S(1, -117), S(-7, -73), S(8, -71),   S(3, 31),   S(3, 109),
      S(17, 28),     S(4, 20),   S(14, -17), S(11, 8),    S(11, 29),  S(13, 11),
      S(-12, 55),    S(14, -10), S(12, -13), S(18, -18),  S(13, -15), S(31, 1),
      S(1, 14),      S(-2, 10),  S(39, -3),  S(1, 55),    S(-1, 68),  S(2, -21),
      S(65, 46),     S(36, 52),  S(14, 37),  S(119, 221),
};
constexpr Score kingAttackersWeight[5] = {
      S(-10, 40), S(43, 5), S(30, 62), S(10, 6), S(3, 59),
};
constexpr Score trappedRookWeight = {
      S(0, 35),
};
constexpr Score rookOnOpenFile[2] = {
      S(-80, 168),
      S(19, -130),
};
constexpr Score openFileNextToKing[3] = {
      S(-31, -37),
      S(10, -28),
      S(-43, -1),
};
constexpr Score pawnShieldStrength[4] = {
      S(-18, -28),
      S(-28, -34),
      S(-5, -56),
      S(1, -49),
};
constexpr Score pushedPawnShieldStrength[4] = {
      S(-38, 1),
      S(-50, -50),
      S(-5, -5),
      S(100, 100),
};
constexpr Score kingAttackersDistance[8] = {
      S(48, -6), S(32, -3), S(1, 10),   S(23, 12),
      S(2, 12),  S(-3, 0),  S(-6, -28), S(0, 0),
};
constexpr Score xRayKingAttackersDistance[8] = {
      S(16, 155), S(51, 104), S(19, 52),  S(16, 33),
      S(25, -15), S(-2, 10),  S(-14, 47), S(0, 0),
};
constexpr Score xRayKingAttackPieceWeight[5] = {
      S(-23, 22), S(-16, 6), S(-9, 37), S(38, 20), S(38, 351),
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
    const Score pawnScore = S(7, -26);
    const Score knightScore = S(122, 483);
    const Score bishopScore = S(136, 562);
    const Score rookScore = S(127, 562);
    const Score queenScore = S(862, 1375);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),     S(39, 83),  S(-6, 160),
          S(58, 26),    S(-40, -75), S(-96, 51),  S(-7, 59),  S(-144, 182),
          S(-297, 159), S(-20, 17),  S(-11, 36),  S(53, -30), S(-41, -105),
          S(13, -81),   S(41, -82),  S(-96, 83),  S(-58, 31), S(-31, 28),
          S(-17, 42),   S(-25, 24),  S(-31, -25), S(-10, 13), S(-4, 23),
          S(21, 36),    S(-34, 42),  S(-23, 4),   S(-8, 7),   S(31, 33),
          S(-12, -10),  S(12, 14),   S(31, 21),   S(16, 1),   S(-20, 39),
          S(-12, -1),   S(-19, -31), S(7, -19),   S(29, 2),   S(20, -11),
          S(26, 14),    S(30, -9),   S(-36, 3),   S(-2, -5),  S(-29, -4),
          S(-15, -6),   S(4, 98),    S(25, 30),   S(2, 2),    S(33, -26),
          S(-29, -7),   S(0, 0),     S(0, 0),     S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),     S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-131, -65), S(-132, 93),  S(-156, -59), S(97, 2),    S(23, 5),
          S(-134, 44),  S(-161, 49),  S(8, -205),   S(63, -197), S(5, -38),
          S(37, 26),    S(-49, 14),   S(-27, 2),    S(73, -10),  S(-64, -13),
          S(108, 97),   S(-103, 2),   S(64, 82),    S(-12, -43), S(-2, 63),
          S(-37, 24),   S(-114, 112), S(-59, -44),  S(12, 83),   S(-30, -39),
          S(0, -36),    S(-3, 27),    S(-16, 51),   S(5, 21),    S(24, 15),
          S(25, 49),    S(31, 49),    S(-31, 163),  S(-12, 22),  S(-18, -30),
          S(-10, 54),   S(-16, 25),   S(-1, 59),    S(25, 64),   S(29, -107),
          S(-14, 27),   S(-12, -1),   S(-16, -19),  S(29, -31),  S(6, 35),
          S(-17, 21),   S(-11, 50),   S(7, 16),     S(-71, -55), S(-174, 109),
          S(20, -57),   S(-23, 43),   S(-7, 24),    S(24, -38),  S(-43, -162),
          S(-50, 78),   S(-79, -34),  S(34, -21),   S(114, 14),  S(-40, -82),
          S(46, -64),   S(18, -77),   S(24, 26),    S(68, 2),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-277, 32),  S(-194, 1),   S(-241, -3), S(-247, -5),  S(-234, 0),
          S(-331, 33),  S(-38, -68),  S(-172, 84), S(108, -49),  S(-61, 29),
          S(-103, 89),  S(-150, -74), S(-49, 66),  S(-133, -39), S(-158, 43),
          S(-29, -100), S(-48, 26),   S(-11, 14),  S(-87, 41),   S(-36, 52),
          S(-81, 45),   S(-145, 82),  S(-101, 32), S(22, -6),    S(-29, 10),
          S(-16, 29),   S(24, -14),   S(-38, 39),  S(-23, 10),   S(-45, -32),
          S(1, 102),    S(-78, 11),   S(-54, 4),   S(-40, 83),   S(-18, -25),
          S(-12, 83),   S(0, 22),     S(2, 40),    S(-34, 8),    S(48, -38),
          S(33, -38),   S(-6, 11),    S(-13, 32),  S(6, 42),     S(-11, 35),
          S(1, 91),     S(9, 12),     S(44, -21),  S(56, 31),    S(28, -23),
          S(7, -3),     S(32, 21),    S(10, 34),   S(95, 21),    S(17, -13),
          S(-7, 202),   S(48, -27),   S(207, 9),   S(20, -8),    S(-53, -21),
          S(-82, 39),   S(41, 2),     S(7, 6),     S(58, -59),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(-161, 53), S(-40, 60),  S(-71, 37), S(-78, 32),  S(-32, 25),
          S(-52, 70),  S(10, 62),   S(111, 27), S(-2, -2),   S(58, -23),
          S(-22, 20),  S(45, -4),   S(-29, 33), S(35, -15),  S(29, -44),
          S(-16, -11), S(-64, 41),  S(46, 16),  S(-40, 15),  S(-54, 38),
          S(12, -32),  S(4, 22),    S(84, -20), S(181, -77), S(18, -38),
          S(-3, -12),  S(-52, -8),  S(5, -63),  S(-71, -14), S(19, -17),
          S(9, -24),   S(58, -58),  S(-17, 8),  S(-82, 22),  S(-148, 72),
          S(-69, 14),  S(-15, -24), S(-79, 9),  S(-27, -46), S(-77, 34),
          S(-117, 2),  S(-12, -24), S(4, 16),   S(18, -4),   S(3, -10),
          S(-8, 14),   S(-7, 0),    S(-78, -2), S(-89, 25),  S(-22, -56),
          S(26, -52),  S(-12, 17),  S(-73, 38), S(-11, 28),  S(55, -6),
          S(-18, -53), S(-23, -59), S(3, 11),   S(-10, -36), S(1, -29),
          S(-13, -11), S(-18, -4),  S(14, -48), S(5, -11),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(77, 184),  S(-84, -4),   S(-47, -24),  S(-109, 126), S(71, 55),
          S(-43, -41), S(128, -162), S(-71, -12),  S(-10, 93),   S(-25, 39),
          S(-62, 123), S(-126, 160), S(-158, 205), S(45, 25),    S(81, 53),
          S(59, 112),  S(3, -4),     S(-21, 22),   S(9, 21),     S(-76, 104),
          S(28, 24),   S(5, 100),    S(37, -77),   S(51, 44),    S(-44, 25),
          S(-29, 58),  S(-16, 35),   S(-67, 133),  S(-46, 78),   S(-63, 150),
          S(-9, 92),   S(-1, -35),   S(5, 35),     S(-65, 76),   S(-46, 185),
          S(-24, 156), S(-21, 76),   S(40, 43),    S(-20, 68),   S(19, 22),
          S(-5, -27),  S(-17, 12),   S(-29, 84),   S(-14, 90),   S(7, 47),
          S(-8, 123),  S(27, 2),     S(-26, 78),   S(17, 6),     S(-65, 62),
          S(12, -32),  S(46, -48),   S(15, 36),    S(27, -3),    S(19, -32),
          S(-65, 62),  S(65, 30),    S(39, -22),   S(29, 33),    S(29, 22),
          S(71, -164), S(100, 27),   S(15, -19),   S(-57, -66),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-125, -110), S(105, -83),  S(61, -96),  S(-137, -116), S(-176, -65),
          S(-146, -141), S(-30, 0),    S(129, -75), S(-150, -143), S(-89, 73),
          S(-163, 57),   S(4, 3),      S(93, -57),  S(-46, -24),   S(-25, -17),
          S(82, -64),    S(-56, -16),  S(93, 103),  S(167, 35),    S(28, 7),
          S(-130, 28),   S(-16, 3),    S(37, 5),    S(35, 12),     S(-125, 25),
          S(50, 68),     S(-229, 48),  S(-318, 26), S(-280, 91),   S(-319, 51),
          S(-9, -77),    S(-15, 16),   S(-138, 21), S(-119, 24),   S(-97, 60),
          S(-265, 70),   S(-218, 55),  S(-190, 31), S(-201, 26),   S(-226, 5),
          S(29, -60),    S(-30, 31),   S(-215, 62), S(-146, 57),   S(-149, 63),
          S(-137, 13),   S(-96, -5),   S(-87, 15),  S(3, 8),       S(-71, 13),
          S(-1, 44),     S(-41, 41),   S(-59, 39),  S(-35, -3),    S(45, -56),
          S(32, -17),    S(143, -140), S(36, -27),  S(32, -14),    S(-106, 6),
          S(46, -23),    S(37, -1),    S(5, -65),   S(55, 0),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),    S(-5, -43),  S(-27, -19), S(-12, 4),
          S(-49, 51), S(-34, 130), S(-7, 153),  S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10),  S(-7, -36),    S(-3, -28),  S(1, -92),
          S(-53, -231), S(-108, -212), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(16, -31),  S(-1, -27),  S(10, -22),
          S(-5, -52), S(-1, -110), S(65, -174), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(1, -7),    S(-3, -16),  S(7, -39),
          S(-12, 26),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-32, 12), S(-8, 27),  S(-18, 19), S(-39, -6), S(-46, 10),
          S(-38, 20), S(-44, 26), S(-7, -6),  S(40, 30),
    };
    const Score BishopMobilityScore[14] = {
          S(-20, -39), S(-21, 13), S(-5, 30),  S(-47, 39), S(-36, 47),
          S(-37, 23),  S(-26, 33), S(-25, 16), S(0, 9),    S(-21, 7),
          S(118, -8),  S(138, 22), S(-82, 2),  S(96, 98),
    };
    const Score RookMobilityScore[15] = {
          S(-35, -98), S(-25, -32), S(-43, -16), S(-41, 3),   S(-39, -41),
          S(-38, -18), S(-41, -45), S(-46, -49), S(-46, -70), S(-46, -55),
          S(-42, -66), S(-29, -43), S(23, -42),  S(-1, -83),  S(70, 175),
    };
    const Score QueenMobilityScore[28] = {
          S(-136, -271), S(1, -117), S(-7, -73),  S(8, -71),  S(3, 31),
          S(3, 109),     S(17, 28),  S(4, 20),    S(14, -17), S(11, 8),
          S(11, 29),     S(13, 11),  S(-12, 55),  S(14, -10), S(12, -13),
          S(18, -18),    S(13, -15), S(31, 1),    S(1, 14),   S(-2, 10),
          S(39, -3),     S(1, 55),   S(-1, 68),   S(2, -21),  S(65, 46),
          S(36, 52),     S(14, 37),  S(119, 221),
    };
    const Score kingAttackersWeight[5] = {
          S(-10, 40), S(43, 5), S(30, 62), S(10, 6), S(3, 59),
    };
    const Score trappedRookWeight = {
          S(0, 35),
    };
    const Score rookOnOpenFile[2] = {
          S(-80, 168),
          S(19, -130),
    };
    const Score openFileNextToKing[3] = {
          S(-31, -37),
          S(10, -28),
          S(-43, -1),
    };
    const Score pawnShieldStrength[4] = {
          S(-18, -28),
          S(-28, -34),
          S(-5, -56),
          S(1, -49),
    };
    const Score pushedPawnShieldStrength[4] = {
          S(-38, 1),
          S(-50, -50),
          S(-5, -5),
          S(100, 100),
    };
    const Score kingAttackersDistance[8] = {
          S(48, -6), S(32, -3), S(1, 10),   S(23, 12),
          S(2, 12),  S(-3, 0),  S(-6, -28), S(0, 0),
    };
    const Score xRayKingAttackersDistance[8] = {
          S(16, 155), S(51, 104), S(19, 52),  S(16, 33),
          S(25, -15), S(-2, 10),  S(-14, 47), S(0, 0),
    };
    const Score xRayKingAttackPieceWeight[5] = {
          S(-23, 22), S(-16, 6), S(-9, 37), S(38, 20), S(38, 351),
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
        if (n > 1) {
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
    Bitboard kingArea = kingAttacks[kingSquare] | king;

    int mgScore = 0, egScore = 0;
    while (kingArea) {
        Square areaSq = Square(lsb_index(kingArea));
        Bitboard attackPcs = board.attacksToKing<~C>(areaSq, board.pieces());

        if (!attackPcs) {
            mgScore += MgScore(kingAttackersWeight[0]);
            egScore += EgScore(kingAttackersWeight[0]);

            if (T) {
                trace.kingAttackers[0][C]++;
            }
        }

        while (attackPcs) {
            Square attacker = Square(lsb_index(attackPcs));
            PieceT piece = getPcType(board.board[attacker]);

            const int distance = popcount(rectArray[kingSquare][attacker]);
            if (T == TRACE) {
                trace.kingAttackers[piece][C]++;
                trace.kingAttackersDistance[distance][C]++;
            }

            mgScore += MgScore(kingAttackersWeight[piece]);
            egScore += EgScore(kingAttackersWeight[piece]);
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
