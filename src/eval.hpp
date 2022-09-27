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

constexpr Score pawnScore   = S(93, 122);
constexpr Score knightScore = S(376, 406);
constexpr Score bishopScore = S(390, 401);
constexpr Score rookScore   = S(526, 669);
constexpr Score queenScore  = S(1117, 1284);

// clang-format off
constexpr Score taperedPawnPcSq[SQUARE_CT] = {
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
S(  21,  122), S(  15,   94), S( -22,  105), S(   7,   49), S( -26,   53), S(  10,   39), S(-103,  102), S(-109,  136),
S( -11,  100), S(  -8,  103), S(  13,   53), S(   9,    6), S(  29,   -1), S(  81,   23), S(  54,   72), S(  16,   79),
S( -19,   58), S( -14,   40), S(   0,   18), S(   6,  -10), S(  29,   -6), S(  28,    4), S(  14,   23), S(   7,   25),
S( -36,   32), S( -26,   23), S( -11,    6), S(   2,   -7), S(   1,   -4), S(   5,    4), S(  -3,    7), S( -12,    6),
S( -26,   23), S( -17,   17), S(  -7,    9), S(  -3,    9), S(  17,   10), S(   9,   10), S(  27,    5), S(   0,    1),
S( -21,   25), S( -15,   24), S(  -4,   11), S(  -2,   15), S(   8,   17), S(  35,   10), S(  42,    2), S(  -3,    3),
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
S(-156,  -47), S(-104,  -36), S( -38,  -13), S( -25,  -13), S(  50,  -15), S( -78,  -19), S( -33,  -43), S( -92,  -96),
S( -32,   -7), S(  -4,    1), S(  50,  -21), S(  50,   -4), S(  51,  -13), S(  88,  -18), S(  17,  -18), S(  38,  -33),
S( -22,   10), S(  40,  -13), S(  56,   13), S(  78,   10), S(  99,    6), S( 122,  -12), S(  72,  -22), S(  48,  -32),
S(   1,    3), S(   9,   15), S(  32,   38), S(  68,   33), S(  45,   32), S(  70,   26), S(  28,    9), S(  58,   -3),
S(  -7,   14), S(   5,    9), S(  16,   37), S(  29,   28), S(  36,   38), S(  33,   22), S(  51,    4), S(  21,    6),
S( -23,  -17), S( -19,   -2), S(   1,    2), S(   9,   22), S(  27,   22), S(  12,   -3), S(  16,  -14), S(  -2,  -10),
S( -39,  -11), S( -25,    2), S( -15,   -7), S(   1,   -3), S(   9,   -3), S(   8,   -7), S(  -1,  -21), S(  -6,  -13),
S( -88,  -20), S( -19,  -22), S( -38,   -8), S( -10,    0), S(  -6,  -11), S(  -5,  -11), S( -15,   -8), S( -31,  -22),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
S( -13,    4), S( -29,   -1), S( -66,    8), S( -63,    3), S( -39,   -6), S( -51,    0), S(  -8,   -1), S( -19,  -14),
S( -10,  -10), S(   9,  -10), S( -15,   -3), S( -12,    1), S(  16,  -13), S(  26,  -14), S(  35,   -4), S( -11,  -11),
S( -11,   13), S(  22,    0), S(  29,    1), S(  37,   -1), S(  35,   -2), S(  58,    4), S(  53,   -2), S(  32,    2),
S( -17,   -3), S(   6,    6), S(  13,   14), S(  50,   17), S(  35,   18), S(  38,   10), S(   9,   13), S(  -2,    2),
S(  -9,   -4), S(  -6,   14), S(   6,   18), S(  34,   13), S(  31,   18), S(   4,   16), S(   0,    9), S(  26,  -17),
S(   1,    9), S(  21,    4), S(  14,   15), S(  15,   15), S(  23,   20), S(  17,   12), S(  19,    0), S(  24,   -7),
S(  15,    1), S(  13,   -4), S(  26,   -8), S(   0,    9), S(  10,    5), S(  28,   -1), S(  35,   -2), S(  19,   -2),
S(  -6,   -2), S(  30,   14), S(  10,   11), S(  -5,    6), S(  -1,    5), S(   2,   13), S(   8,   -1), S(   9,   -7),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
S(  26,   17), S(  46,   14), S(  16,   35), S(  26,   23), S(  57,   14), S(  28,   24), S(  42,   19), S(  64,   12),
S(   8,   19), S(   9,   29), S(  31,   33), S(  62,   13), S(  46,   12), S(  73,    8), S(  39,    8), S(  64,    5),
S(  -1,   21), S(  23,   24), S(  23,   18), S(  37,   17), S(  51,   10), S(  71,   -4), S( 100,   -1), S(  59,    4),
S( -17,   24), S(   8,   18), S(   3,   32), S(  16,   16), S(  21,    2), S(  28,    0), S(  31,    9), S(  21,    0),
S( -36,   15), S( -28,   18), S( -19,   23), S( -15,   17), S(  -8,   12), S( -24,   13), S(  12,    5), S(  -9,    9),
S( -36,   12), S( -27,   14), S( -20,    6), S( -17,   11), S( -10,    8), S(  -3,   -6), S(  31,  -19), S(  -8,  -14),
S( -47,    5), S( -27,    2), S( -10,    8), S(  -7,    5), S(  -1,   -7), S(   5,  -11), S(  27,  -17), S( -37,    1),
S( -21,    6), S( -17,    6), S(  -3,    6), S(   8,    3), S(  12,   -6), S(   1,   -3), S(   5,   -6), S( -21,   -6),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
S( -36,   30), S( -18,   19), S(   9,   45), S(  17,   56), S(  39,   43), S(  43,   34), S(  68,   12), S(  47,   30),
S( -15,   21), S( -48,   38), S( -17,   59), S( -19,   75), S( -14,   97), S(  37,   35), S(  21,   35), S(  76,   33),
S( -11,   22), S( -23,   28), S(  -6,   46), S(  -9,   77), S(  23,   73), S(  70,   53), S(  78,   19), S(  71,   50),
S( -25,   29), S( -25,   34), S( -20,   43), S( -19,   65), S(  -3,   76), S(  13,   73), S(  18,   76), S(  22,   46),
S( -15,    5), S( -27,   40), S( -27,   42), S( -20,   68), S( -15,   66), S( -11,   66), S(  -1,   50), S(  15,   57),
S( -14,    1), S(  -7,   12), S( -14,   27), S( -15,   34), S( -11,   53), S(   6,   27), S(  15,   17), S(  10,   29),
S( -13,    5), S( -13,  -12), S(   3,  -11), S(  15,  -10), S(   8,    3), S(  17,  -21), S(  15,  -19), S(  25,  -34),
S(  -6,  -15), S(  -3,  -18), S(   8,  -26), S(  17,  -11), S(   6,  -10), S(  -7,   -8), S(  -7,  -17), S( -13,   -7),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
S( -71,  -98), S(  10,  -60), S(  11,  -32), S( -23,  -11), S( -61,  -19), S( -29,    2), S(  16,    9), S(   7,  -71),
S(  -7,  -32), S( -11,   11), S( -57,   17), S(  -5,   20), S(  -1,   25), S(  -1,   39), S(  12,   37), S(   2,    6),
S( -54,   -7), S(  11,   11), S( -21,   38), S( -17,   51), S( -19,   62), S(  27,   65), S(  42,   49), S(  -9,   10),
S( -33,  -17), S( -29,   24), S( -47,   42), S( -93,   56), S( -82,   59), S( -63,   58), S( -54,   43), S( -65,    4),
S( -77,  -23), S( -35,    2), S( -91,   38), S(-110,   58), S(-131,   61), S( -92,   45), S( -91,   25), S(-112,    2),
S( -42,  -33), S( -36,   -5), S( -77,   17), S(-119,   39), S(-114,   40), S(-108,   31), S( -61,    7), S( -85,   -9),
S(  33,  -42), S( -11,  -16), S( -28,   -3), S( -69,    8), S( -74,   15), S( -53,    7), S(   7,  -12), S(  19,  -36),
S(  29,  -84), S(  68,  -66), S(  41,  -46), S( -64,  -26), S(   4,  -39), S( -39,  -29), S(  47,  -59), S(  41,  -97),
};
constexpr Score passedPawnRankBonus[8] = {
S(   0,    0), S(  -5,    3), S(  -5,   14), S(  -5,   44),
S(   5,   79), S(  30,  135), S( 107,  190), S(   0,    0),
};
constexpr Score doubledPawnRankBonus[8] = {
S( -10,  -10), S(  -6,  -27), S(  -8,  -18), S(  -1,  -31),
S(  -3,  -45), S( -52,  -45), S( -10,  -10), S( -10,  -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
S(  -6,   -6), S( -15,   -6), S( -21,  -15), S( -15,  -13),
S(  -5,  -25), S(   4,  -26), S( -62,  -39), S(  -6,   -6),
};
constexpr Score backwardPawnRankBonus[8] = {
S( -15,  -15), S( -17,  -12), S(  -5,   -6), S(  -9,   -3),
S( -16,   -7), S( -15,  -15), S( -15,  -15), S( -15,  -15),
};
constexpr Score KnightMobilityScore[9] = {
S( -35,  -24), S( -17,   13), S(  -5,   22), S(   5,   31),
S(  14,   45), S(  24,   43), S(  35,   41), S(  43,   27),
S(  40,   30),
};
constexpr Score BishopMobilityScore[14] = {
S(  -9,  -19), S(  14,    6), S(  23,   29), S(  36,   43),
S(  44,   57), S(  52,   63), S(  56,   68), S(  60,   74),
S(  63,   70), S(  68,   72), S(  76,   66), S(  59,   79),
S(  81,   57), S(  96,   98),
};
constexpr Score RookMobilityScore[15] = {
S( -23,   51), S( -14,   74), S(  -7,   92), S(  -2,  103),
S(   0,  107), S(   8,  123), S(  17,  124), S(  24,  132),
S(  37,  136), S(  42,  139), S(  50,  148), S(  52,  157),
S(  55,  155), S(  41,  160), S(  70,  175),
};
constexpr Score QueenMobilityScore[28] = {
S( -61,  -81), S( -17,  -33), S(   4,   -5), S(  12,   38),
S(  20,   64), S(  28,   65), S(  33,  100), S(  41,  109),
S(  49,  108), S(  53,  133), S(  61,  130), S(  65,  141),
S(  67,  150), S(  65,  159), S(  73,  167), S(  69,  183),
S(  71,  190), S(  74,  194), S(  84,  202), S(  84,  207),
S( 114,  192), S( 129,  199), S( 114,  194), S( 138,  198),
S( 122,  189), S( 117,  183), S( 119,  203), S( 119,  221),
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
const Score pawnScore   = S(  93,  122);
const Score knightScore = S( 376,  406);
const Score bishopScore = S( 390,  401);
const Score rookScore   = S( 526,  669);
const Score queenScore  = S(1117, 1284);

const Score taperedPawnPcSq[SQUARE_CT] = {
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
S(  21,  122), S(  15,   94), S( -22,  105), S(   7,   49), S( -26,   53), S(  10,   39), S(-103,  102), S(-109,  136),
S( -11,  100), S(  -8,  103), S(  13,   53), S(   9,    6), S(  29,   -1), S(  81,   23), S(  54,   72), S(  16,   79),
S( -19,   58), S( -14,   40), S(   0,   18), S(   6,  -10), S(  29,   -6), S(  28,    4), S(  14,   23), S(   7,   25),
S( -36,   32), S( -26,   23), S( -11,    6), S(   2,   -7), S(   1,   -4), S(   5,    4), S(  -3,    7), S( -12,    6),
S( -26,   23), S( -17,   17), S(  -7,    9), S(  -3,    9), S(  17,   10), S(   9,   10), S(  27,    5), S(   0,    1),
S( -21,   25), S( -15,   24), S(  -4,   11), S(  -2,   15), S(   8,   17), S(  35,   10), S(  42,    2), S(  -3,    3),
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
};
const Score taperedKnightPcSq[SQUARE_CT] = {
S(-156,  -47), S(-104,  -36), S( -38,  -13), S( -25,  -13), S(  50,  -15), S( -78,  -19), S( -33,  -43), S( -92,  -96),
S( -32,   -7), S(  -4,    1), S(  50,  -21), S(  50,   -4), S(  51,  -13), S(  88,  -18), S(  17,  -18), S(  38,  -33),
S( -22,   10), S(  40,  -13), S(  56,   13), S(  78,   10), S(  99,    6), S( 122,  -12), S(  72,  -22), S(  48,  -32),
S(   1,    3), S(   9,   15), S(  32,   38), S(  68,   33), S(  45,   32), S(  70,   26), S(  28,    9), S(  58,   -3),
S(  -7,   14), S(   5,    9), S(  16,   37), S(  29,   28), S(  36,   38), S(  33,   22), S(  51,    4), S(  21,    6),
S( -23,  -17), S( -19,   -2), S(   1,    2), S(   9,   22), S(  27,   22), S(  12,   -3), S(  16,  -14), S(  -2,  -10),
S( -39,  -11), S( -25,    2), S( -15,   -7), S(   1,   -3), S(   9,   -3), S(   8,   -7), S(  -1,  -21), S(  -6,  -13),
S( -88,  -20), S( -19,  -22), S( -38,   -8), S( -10,    0), S(  -6,  -11), S(  -5,  -11), S( -15,   -8), S( -31,  -22),
};
const Score taperedBishopPcSq[SQUARE_CT] = {
S( -13,    4), S( -29,   -1), S( -66,    8), S( -63,    3), S( -39,   -6), S( -51,    0), S(  -8,   -1), S( -19,  -14),
S( -10,  -10), S(   9,  -10), S( -15,   -3), S( -12,    1), S(  16,  -13), S(  26,  -14), S(  35,   -4), S( -11,  -11),
S( -11,   13), S(  22,    0), S(  29,    1), S(  37,   -1), S(  35,   -2), S(  58,    4), S(  53,   -2), S(  32,    2),
S( -17,   -3), S(   6,    6), S(  13,   14), S(  50,   17), S(  35,   18), S(  38,   10), S(   9,   13), S(  -2,    2),
S(  -9,   -4), S(  -6,   14), S(   6,   18), S(  34,   13), S(  31,   18), S(   4,   16), S(   0,    9), S(  26,  -17),
S(   1,    9), S(  21,    4), S(  14,   15), S(  15,   15), S(  23,   20), S(  17,   12), S(  19,    0), S(  24,   -7),
S(  15,    1), S(  13,   -4), S(  26,   -8), S(   0,    9), S(  10,    5), S(  28,   -1), S(  35,   -2), S(  19,   -2),
S(  -6,   -2), S(  30,   14), S(  10,   11), S(  -5,    6), S(  -1,    5), S(   2,   13), S(   8,   -1), S(   9,   -7),
};
const Score taperedRookPcSq[SQUARE_CT] = {
S(  26,   17), S(  46,   14), S(  16,   35), S(  26,   23), S(  57,   14), S(  28,   24), S(  42,   19), S(  64,   12),
S(   8,   19), S(   9,   29), S(  31,   33), S(  62,   13), S(  46,   12), S(  73,    8), S(  39,    8), S(  64,    5),
S(  -1,   21), S(  23,   24), S(  23,   18), S(  37,   17), S(  51,   10), S(  71,   -4), S( 100,   -1), S(  59,    4),
S( -17,   24), S(   8,   18), S(   3,   32), S(  16,   16), S(  21,    2), S(  28,    0), S(  31,    9), S(  21,    0),
S( -36,   15), S( -28,   18), S( -19,   23), S( -15,   17), S(  -8,   12), S( -24,   13), S(  12,    5), S(  -9,    9),
S( -36,   12), S( -27,   14), S( -20,    6), S( -17,   11), S( -10,    8), S(  -3,   -6), S(  31,  -19), S(  -8,  -14),
S( -47,    5), S( -27,    2), S( -10,    8), S(  -7,    5), S(  -1,   -7), S(   5,  -11), S(  27,  -17), S( -37,    1),
S( -21,    6), S( -17,    6), S(  -3,    6), S(   8,    3), S(  12,   -6), S(   1,   -3), S(   5,   -6), S( -21,   -6),
};
const Score taperedQueenPcSq[SQUARE_CT] = {
S( -36,   30), S( -18,   19), S(   9,   45), S(  17,   56), S(  39,   43), S(  43,   34), S(  68,   12), S(  47,   30),
S( -15,   21), S( -48,   38), S( -17,   59), S( -19,   75), S( -14,   97), S(  37,   35), S(  21,   35), S(  76,   33),
S( -11,   22), S( -23,   28), S(  -6,   46), S(  -9,   77), S(  23,   73), S(  70,   53), S(  78,   19), S(  71,   50),
S( -25,   29), S( -25,   34), S( -20,   43), S( -19,   65), S(  -3,   76), S(  13,   73), S(  18,   76), S(  22,   46),
S( -15,    5), S( -27,   40), S( -27,   42), S( -20,   68), S( -15,   66), S( -11,   66), S(  -1,   50), S(  15,   57),
S( -14,    1), S(  -7,   12), S( -14,   27), S( -15,   34), S( -11,   53), S(   6,   27), S(  15,   17), S(  10,   29),
S( -13,    5), S( -13,  -12), S(   3,  -11), S(  15,  -10), S(   8,    3), S(  17,  -21), S(  15,  -19), S(  25,  -34),
S(  -6,  -15), S(  -3,  -18), S(   8,  -26), S(  17,  -11), S(   6,  -10), S(  -7,   -8), S(  -7,  -17), S( -13,   -7),
};
const Score taperedKingPcSq[SQUARE_CT] = {
S( -71,  -98), S(  10,  -60), S(  11,  -32), S( -23,  -11), S( -61,  -19), S( -29,    2), S(  16,    9), S(   7,  -71),
S(  -7,  -32), S( -11,   11), S( -57,   17), S(  -5,   20), S(  -1,   25), S(  -1,   39), S(  12,   37), S(   2,    6),
S( -54,   -7), S(  11,   11), S( -21,   38), S( -17,   51), S( -19,   62), S(  27,   65), S(  42,   49), S(  -9,   10),
S( -33,  -17), S( -29,   24), S( -47,   42), S( -93,   56), S( -82,   59), S( -63,   58), S( -54,   43), S( -65,    4),
S( -77,  -23), S( -35,    2), S( -91,   38), S(-110,   58), S(-131,   61), S( -92,   45), S( -91,   25), S(-112,    2),
S( -42,  -33), S( -36,   -5), S( -77,   17), S(-119,   39), S(-114,   40), S(-108,   31), S( -61,    7), S( -85,   -9),
S(  33,  -42), S( -11,  -16), S( -28,   -3), S( -69,    8), S( -74,   15), S( -53,    7), S(   7,  -12), S(  19,  -36),
S(  29,  -84), S(  68,  -66), S(  41,  -46), S( -64,  -26), S(   4,  -39), S( -39,  -29), S(  47,  -59), S(  41,  -97),
};
const Score passedPawnRankBonus[8] = {
S(   0,    0), S(  -5,    3), S(  -5,   14), S(  -5,   44),
S(   5,   79), S(  30,  135), S( 107,  190), S(   0,    0),
};
const Score doubledPawnRankBonus[8] = {
S( -10,  -10), S(  -6,  -27), S(  -8,  -18), S(  -1,  -31),
S(  -3,  -45), S( -52,  -45), S( -10,  -10), S( -10,  -10),
};
const Score isolatedPawnRankBonus[8] = {
S(  -6,   -6), S( -15,   -6), S( -21,  -15), S( -15,  -13),
S(  -5,  -25), S(   4,  -26), S( -62,  -39), S(  -6,   -6),
};
const Score backwardPawnRankBonus[8] = {
S( -15,  -15), S( -17,  -12), S(  -5,   -6), S(  -9,   -3),
S( -16,   -7), S( -15,  -15), S( -15,  -15), S( -15,  -15),
};
const Score KnightMobilityScore[9] = {
S( -35,  -24), S( -17,   13), S(  -5,   22), S(   5,   31),
S(  14,   45), S(  24,   43), S(  35,   41), S(  43,   27),
S(  40,   30),
};
const Score BishopMobilityScore[14] = {
S(  -9,  -19), S(  14,    6), S(  23,   29), S(  36,   43),
S(  44,   57), S(  52,   63), S(  56,   68), S(  60,   74),
S(  63,   70), S(  68,   72), S(  76,   66), S(  59,   79),
S(  81,   57), S(  96,   98),
};
const Score RookMobilityScore[15] = {
S( -23,   51), S( -14,   74), S(  -7,   92), S(  -2,  103),
S(   0,  107), S(   8,  123), S(  17,  124), S(  24,  132),
S(  37,  136), S(  42,  139), S(  50,  148), S(  52,  157),
S(  55,  155), S(  41,  160), S(  70,  175),
};
const Score QueenMobilityScore[28] = {
S( -61,  -81), S( -17,  -33), S(   4,   -5), S(  12,   38),
S(  20,   64), S(  28,   65), S(  33,  100), S(  41,  109),
S(  49,  108), S(  53,  133), S(  61,  130), S(  65,  141),
S(  67,  150), S(  65,  159), S(  73,  167), S(  69,  183),
S(  71,  190), S(  74,  194), S(  84,  202), S(  84,  207),
S( 114,  192), S( 129,  199), S( 114,  194), S( 138,  198),
S( 122,  189), S( 117,  183), S( 119,  203), S( 119,  221),
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
