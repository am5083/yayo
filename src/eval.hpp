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

constexpr Score pawnScore   = S(94, 124);
constexpr Score knightScore = S(379, 413);
constexpr Score bishopScore = S(394, 409);
constexpr Score rookScore   = S(529, 684);
constexpr Score queenScore  = S(1127, 1316);

// clang-format off
constexpr Score taperedPawnPcSq[SQUARE_CT] = {
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
S(  19,  120), S(  11,   94), S( -22,  106), S(   7,   48), S( -28,   49), S(   4,   38), S(-110,  104), S(-115,  137),
S( -10,  100), S(  -9,  104), S(  15,   52), S(  14,    3), S(  32,   -3), S(  84,   22), S(  56,   71), S(  19,   78),
S( -20,   59), S( -16,   41), S(   0,   18), S(   6,  -10), S(  29,   -6), S(  28,    4), S(  14,   23), S(   8,   25),
S( -36,   33), S( -27,   24), S( -11,    6), S(   2,   -7), S(   1,   -4), S(   5,    4), S(  -3,    7), S( -11,    6),
S( -26,   23), S( -18,   18), S(  -7,   10), S(  -3,    9), S(  17,   10), S(  10,   10), S(  27,    5), S(   1,    1),
S( -22,   26), S( -16,   25), S(  -4,   11), S(  -2,   15), S(   9,   17), S(  36,   10), S(  42,    2), S(  -2,    2),
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
S(-158,  -48), S(-108,  -36), S( -39,  -13), S( -22,  -12), S(  47,  -13), S( -76,  -19), S( -37,  -40), S( -92,  -97),
S( -28,   -7), S(  -1,    1), S(  48,  -20), S(  52,   -4), S(  55,  -14), S(  91,  -18), S(  17,  -17), S(  44,  -32),
S( -20,   10), S(  38,  -12), S(  57,   13), S(  79,   11), S( 100,    7), S( 122,  -11), S(  72,  -22), S(  47,  -31),
S(   2,    5), S(   9,   16), S(  33,   38), S(  68,   34), S(  45,   33), S(  69,   28), S(  28,   10), S(  60,   -3),
S(  -7,   16), S(   5,   10), S(  16,   38), S(  29,   29), S(  36,   39), S(  33,   23), S(  52,    4), S(  22,    7),
S( -23,  -17), S( -19,   -1), S(   1,    3), S(   9,   23), S(  28,   23), S(  13,   -3), S(  16,  -13), S(  -1,   -9),
S( -40,   -8), S( -23,    3), S( -14,   -6), S(   2,   -2), S(   9,   -3), S(   8,   -6), S(   0,  -21), S(  -6,  -12),
S( -87,  -20), S( -18,  -22), S( -37,   -7), S(  -9,    0), S(  -5,  -10), S(  -4,  -11), S( -14,   -7), S( -32,  -18),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
S( -12,    4), S( -35,    1), S( -65,    9), S( -68,    4), S( -41,   -5), S( -53,    1), S( -11,    0), S( -22,  -14),
S(  -9,  -10), S(   8,  -10), S( -14,   -4), S( -13,    2), S(  14,  -13), S(  22,  -13), S(  35,   -4), S(  -9,  -11),
S( -10,   14), S(  22,    1), S(  28,    3), S(  37,   -1), S(  35,   -2), S(  59,    5), S(  54,   -2), S(  33,    2),
S( -17,   -2), S(   6,    6), S(  13,   15), S(  50,   19), S(  35,   20), S(  38,   10), S(   9,   13), S(  -2,    2),
S(  -9,   -3), S(  -6,   15), S(   6,   19), S(  35,   14), S(  31,   20), S(   5,   17), S(  -1,   11), S(  27,  -18),
S(   1,   10), S(  21,    5), S(  14,   16), S(  15,   16), S(  24,   20), S(  18,   13), S(  19,    0), S(  25,   -7),
S(  15,    2), S(  13,   -4), S(  26,   -7), S(   0,    9), S(  10,    5), S(  29,    0), S(  35,   -1), S(  20,    0),
S(  -5,   -2), S(  31,   14), S(  11,   11), S(  -4,    6), S(  -1,    5), S(   3,   13), S(  13,   -2), S(  11,   -7),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
S(  25,   19), S(  47,   15), S(  13,   38), S(  23,   25), S(  56,   16), S(  31,   25), S(  44,   20), S(  67,   13),
S(   6,   21), S(   7,   31), S(  28,   35), S(  62,   15), S(  42,   15), S(  73,    9), S(  41,    9), S(  65,    6),
S(  -1,   21), S(  23,   25), S(  22,   19), S(  37,   18), S(  53,   10), S(  74,   -5), S( 106,   -2), S(  63,    3),
S( -17,   25), S(   9,   18), S(   2,   33), S(  14,   18), S(  21,    2), S(  28,    0), S(  34,    8), S(  25,   -1),
S( -36,   15), S( -29,   20), S( -19,   24), S( -16,   19), S( -10,   14), S( -25,   14), S(  12,    5), S(  -8,    9),
S( -36,   13), S( -28,   15), S( -20,    7), S( -18,   12), S( -11,   10), S(  -3,   -5), S(  33,  -20), S(  -7,  -14),
S( -48,    5), S( -27,    3), S( -10,    9), S(  -7,    6), S(   0,   -6), S(   5,  -11), S(  29,  -18), S( -35,    0),
S( -21,    6), S( -17,    6), S(  -3,    7), S(   7,    4), S(  12,   -6), S(   1,   -3), S(   6,   -6), S( -20,   -7),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
S( -39,   34), S( -19,   18), S(   6,   48), S(  17,   58), S(  36,   46), S(  42,   36), S(  71,   10), S(  47,   30),
S( -15,   22), S( -49,   39), S( -19,   62), S( -23,   80), S( -15,  102), S(  35,   37), S(  19,   35), S(  76,   35),
S( -11,   24), S( -25,   30), S(  -8,   51), S( -11,   81), S(  22,   76), S(  71,   53), S(  81,   18), S(  71,   53),
S( -25,   31), S( -25,   34), S( -20,   44), S( -20,   67), S(  -4,   78), S(  12,   76), S(  18,   76), S(  22,   46),
S( -15,    7), S( -27,   41), S( -27,   45), S( -21,   71), S( -16,   70), S( -12,   70), S(  -1,   52), S(  15,   59),
S( -13,    2), S(  -8,   15), S( -13,   28), S( -15,   37), S( -11,   56), S(   6,   28), S(  16,   18), S(  11,   31),
S( -12,    6), S( -14,  -11), S(   3,  -10), S(  15,   -9), S(   8,    4), S(  18,  -21), S(  16,  -19), S(  28,  -37),
S(  -6,  -13), S(  -2,  -19), S(   9,  -27), S(  18,  -11), S(   8,  -12), S(  -6,   -7), S(  -4,  -18), S(  -9,   -5),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
S( -71, -102), S(   9,  -62), S(  12,  -34), S( -24,  -11), S( -62,  -20), S( -28,    0), S(  17,    9), S(   6,  -80),
S( -13,  -33), S( -13,   10), S( -64,   18), S(  -4,   20), S(   0,   26), S(  -1,   40), S(  19,   37), S(   6,    4),
S( -62,   -7), S(   9,   11), S( -26,   39), S( -20,   52), S( -20,   64), S(  29,   66), S(  46,   49), S(  -8,   10),
S( -36,  -18), S( -32,   25), S( -55,   43), S(-106,   58), S( -93,   61), S( -71,   60), S( -63,   45), S( -71,    4),
S( -82,  -23), S( -41,    3), S(-103,   41), S(-122,   60), S(-146,   64), S(-101,   47), S(-101,   27), S(-123,    4),
S( -46,  -33), S( -39,   -5), S( -84,   18), S(-128,   41), S(-123,   42), S(-116,   33), S( -65,    7), S( -91,   -9),
S(  37,  -45), S( -12,  -16), S( -29,   -3), S( -70,    8), S( -75,   15), S( -54,    7), S(   8,  -13), S(  20,  -38),
S(  33,  -89), S(  71,  -69), S(  43,  -48), S( -65,  -27), S(   5,  -41), S( -38,  -31), S(  49,  -61), S(  43, -100),
};
constexpr Score passedPawnRankBonus[8] = {
S(   0,    0), S(  -5,    3), S(  -5,   14), S(  -4,   45),
S(   5,   80), S(  24,  140), S(  97,  189), S(   0,    0),
};
constexpr Score doubledPawnRankBonus[8] = {
S( -10,  -10), S(  -6,  -28), S(  -8,  -18), S(   0,  -32),
S(   1,  -47), S( -56,  -47), S( -10,  -10), S( -10,  -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
S(  -6,   -6), S( -16,   -6), S( -21,  -15), S( -16,  -13),
S(  -5,  -25), S(   5,  -27), S( -48,  -29), S(  -6,   -6),
};
constexpr Score backwardPawnRankBonus[8] = {
S( -15,  -15), S( -17,  -12), S(  -5,   -6), S(  -9,   -3),
S( -16,   -7), S( -15,  -15), S( -15,  -15), S( -15,  -15),
};
constexpr Score KnightMobilityScore[9] = {
S( -35,  -22), S( -17,   14), S(  -5,   24), S(   5,   32),
S(  14,   47), S(  25,   46), S(  36,   43), S(  44,   29),
S(  40,   30),
};
constexpr Score BishopMobilityScore[14] = {
S(  -9,  -18), S(  14,    8), S(  23,   31), S(  37,   45),
S(  45,   58), S(  52,   65), S(  56,   69), S(  61,   76),
S(  64,   72), S(  69,   73), S(  78,   68), S(  57,   81),
S(  81,   57), S(  96,   98),
};
constexpr Score RookMobilityScore[15] = {
S( -24,   56), S( -15,   76), S(  -7,   94), S(  -2,  105),
S(   0,  109), S(   8,  125), S(  17,  126), S(  24,  133),
S(  38,  138), S(  43,  141), S(  51,  149), S(  54,  158),
S(  57,  156), S(  43,  161), S(  70,  175),
};
constexpr Score QueenMobilityScore[28] = {
S( -66,  -87), S( -18,  -33), S(   4,   -6), S(  12,   39),
S(  20,   67), S(  28,   67), S(  33,  104), S(  41,  112),
S(  49,  111), S(  54,  136), S(  61,  133), S(  65,  144),
S(  67,  153), S(  65,  163), S(  73,  170), S(  69,  187),
S(  71,  194), S(  74,  198), S(  85,  206), S(  84,  211),
S( 117,  193), S( 132,  201), S( 116,  196), S( 144,  201),
S( 124,  190), S( 118,  183), S( 121,  205), S( 119,  221),
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
const Score pawnScore   = S(  94,  124);
const Score knightScore = S( 379,  413);
const Score bishopScore = S( 394,  409);
const Score rookScore   = S( 529,  684);
const Score queenScore  = S(1127, 1316);

const Score taperedPawnPcSq[SQUARE_CT] = {
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
S(  19,  120), S(  11,   94), S( -22,  106), S(   7,   48), S( -28,   49), S(   4,   38), S(-110,  104), S(-115,  137),
S( -10,  100), S(  -9,  104), S(  15,   52), S(  14,    3), S(  32,   -3), S(  84,   22), S(  56,   71), S(  19,   78),
S( -20,   59), S( -16,   41), S(   0,   18), S(   6,  -10), S(  29,   -6), S(  28,    4), S(  14,   23), S(   8,   25),
S( -36,   33), S( -27,   24), S( -11,    6), S(   2,   -7), S(   1,   -4), S(   5,    4), S(  -3,    7), S( -11,    6),
S( -26,   23), S( -18,   18), S(  -7,   10), S(  -3,    9), S(  17,   10), S(  10,   10), S(  27,    5), S(   1,    1),
S( -22,   26), S( -16,   25), S(  -4,   11), S(  -2,   15), S(   9,   17), S(  36,   10), S(  42,    2), S(  -2,    2),
S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0), S(   0,    0),
};
const Score taperedKnightPcSq[SQUARE_CT] = {
S(-158,  -48), S(-108,  -36), S( -39,  -13), S( -22,  -12), S(  47,  -13), S( -76,  -19), S( -37,  -40), S( -92,  -97),
S( -28,   -7), S(  -1,    1), S(  48,  -20), S(  52,   -4), S(  55,  -14), S(  91,  -18), S(  17,  -17), S(  44,  -32),
S( -20,   10), S(  38,  -12), S(  57,   13), S(  79,   11), S( 100,    7), S( 122,  -11), S(  72,  -22), S(  47,  -31),
S(   2,    5), S(   9,   16), S(  33,   38), S(  68,   34), S(  45,   33), S(  69,   28), S(  28,   10), S(  60,   -3),
S(  -7,   16), S(   5,   10), S(  16,   38), S(  29,   29), S(  36,   39), S(  33,   23), S(  52,    4), S(  22,    7),
S( -23,  -17), S( -19,   -1), S(   1,    3), S(   9,   23), S(  28,   23), S(  13,   -3), S(  16,  -13), S(  -1,   -9),
S( -40,   -8), S( -23,    3), S( -14,   -6), S(   2,   -2), S(   9,   -3), S(   8,   -6), S(   0,  -21), S(  -6,  -12),
S( -87,  -20), S( -18,  -22), S( -37,   -7), S(  -9,    0), S(  -5,  -10), S(  -4,  -11), S( -14,   -7), S( -32,  -18),
};
const Score taperedBishopPcSq[SQUARE_CT] = {
S( -12,    4), S( -35,    1), S( -65,    9), S( -68,    4), S( -41,   -5), S( -53,    1), S( -11,    0), S( -22,  -14),
S(  -9,  -10), S(   8,  -10), S( -14,   -4), S( -13,    2), S(  14,  -13), S(  22,  -13), S(  35,   -4), S(  -9,  -11),
S( -10,   14), S(  22,    1), S(  28,    3), S(  37,   -1), S(  35,   -2), S(  59,    5), S(  54,   -2), S(  33,    2),
S( -17,   -2), S(   6,    6), S(  13,   15), S(  50,   19), S(  35,   20), S(  38,   10), S(   9,   13), S(  -2,    2),
S(  -9,   -3), S(  -6,   15), S(   6,   19), S(  35,   14), S(  31,   20), S(   5,   17), S(  -1,   11), S(  27,  -18),
S(   1,   10), S(  21,    5), S(  14,   16), S(  15,   16), S(  24,   20), S(  18,   13), S(  19,    0), S(  25,   -7),
S(  15,    2), S(  13,   -4), S(  26,   -7), S(   0,    9), S(  10,    5), S(  29,    0), S(  35,   -1), S(  20,    0),
S(  -5,   -2), S(  31,   14), S(  11,   11), S(  -4,    6), S(  -1,    5), S(   3,   13), S(  13,   -2), S(  11,   -7),
};
const Score taperedRookPcSq[SQUARE_CT] = {
S(  25,   19), S(  47,   15), S(  13,   38), S(  23,   25), S(  56,   16), S(  31,   25), S(  44,   20), S(  67,   13),
S(   6,   21), S(   7,   31), S(  28,   35), S(  62,   15), S(  42,   15), S(  73,    9), S(  41,    9), S(  65,    6),
S(  -1,   21), S(  23,   25), S(  22,   19), S(  37,   18), S(  53,   10), S(  74,   -5), S( 106,   -2), S(  63,    3),
S( -17,   25), S(   9,   18), S(   2,   33), S(  14,   18), S(  21,    2), S(  28,    0), S(  34,    8), S(  25,   -1),
S( -36,   15), S( -29,   20), S( -19,   24), S( -16,   19), S( -10,   14), S( -25,   14), S(  12,    5), S(  -8,    9),
S( -36,   13), S( -28,   15), S( -20,    7), S( -18,   12), S( -11,   10), S(  -3,   -5), S(  33,  -20), S(  -7,  -14),
S( -48,    5), S( -27,    3), S( -10,    9), S(  -7,    6), S(   0,   -6), S(   5,  -11), S(  29,  -18), S( -35,    0),
S( -21,    6), S( -17,    6), S(  -3,    7), S(   7,    4), S(  12,   -6), S(   1,   -3), S(   6,   -6), S( -20,   -7),
};
const Score taperedQueenPcSq[SQUARE_CT] = {
S( -39,   34), S( -19,   18), S(   6,   48), S(  17,   58), S(  36,   46), S(  42,   36), S(  71,   10), S(  47,   30),
S( -15,   22), S( -49,   39), S( -19,   62), S( -23,   80), S( -15,  102), S(  35,   37), S(  19,   35), S(  76,   35),
S( -11,   24), S( -25,   30), S(  -8,   51), S( -11,   81), S(  22,   76), S(  71,   53), S(  81,   18), S(  71,   53),
S( -25,   31), S( -25,   34), S( -20,   44), S( -20,   67), S(  -4,   78), S(  12,   76), S(  18,   76), S(  22,   46),
S( -15,    7), S( -27,   41), S( -27,   45), S( -21,   71), S( -16,   70), S( -12,   70), S(  -1,   52), S(  15,   59),
S( -13,    2), S(  -8,   15), S( -13,   28), S( -15,   37), S( -11,   56), S(   6,   28), S(  16,   18), S(  11,   31),
S( -12,    6), S( -14,  -11), S(   3,  -10), S(  15,   -9), S(   8,    4), S(  18,  -21), S(  16,  -19), S(  28,  -37),
S(  -6,  -13), S(  -2,  -19), S(   9,  -27), S(  18,  -11), S(   8,  -12), S(  -6,   -7), S(  -4,  -18), S(  -9,   -5),
};
const Score taperedKingPcSq[SQUARE_CT] = {
S( -71, -102), S(   9,  -62), S(  12,  -34), S( -24,  -11), S( -62,  -20), S( -28,    0), S(  17,    9), S(   6,  -80),
S( -13,  -33), S( -13,   10), S( -64,   18), S(  -4,   20), S(   0,   26), S(  -1,   40), S(  19,   37), S(   6,    4),
S( -62,   -7), S(   9,   11), S( -26,   39), S( -20,   52), S( -20,   64), S(  29,   66), S(  46,   49), S(  -8,   10),
S( -36,  -18), S( -32,   25), S( -55,   43), S(-106,   58), S( -93,   61), S( -71,   60), S( -63,   45), S( -71,    4),
S( -82,  -23), S( -41,    3), S(-103,   41), S(-122,   60), S(-146,   64), S(-101,   47), S(-101,   27), S(-123,    4),
S( -46,  -33), S( -39,   -5), S( -84,   18), S(-128,   41), S(-123,   42), S(-116,   33), S( -65,    7), S( -91,   -9),
S(  37,  -45), S( -12,  -16), S( -29,   -3), S( -70,    8), S( -75,   15), S( -54,    7), S(   8,  -13), S(  20,  -38),
S(  33,  -89), S(  71,  -69), S(  43,  -48), S( -65,  -27), S(   5,  -41), S( -38,  -31), S(  49,  -61), S(  43, -100),
};
const Score passedPawnRankBonus[8] = {
S(   0,    0), S(  -5,    3), S(  -5,   14), S(  -4,   45),
S(   5,   80), S(  24,  140), S(  97,  189), S(   0,    0),
};
const Score doubledPawnRankBonus[8] = {
S( -10,  -10), S(  -6,  -28), S(  -8,  -18), S(   0,  -32),
S(   1,  -47), S( -56,  -47), S( -10,  -10), S( -10,  -10),
};
const Score isolatedPawnRankBonus[8] = {
S(  -6,   -6), S( -16,   -6), S( -21,  -15), S( -16,  -13),
S(  -5,  -25), S(   5,  -27), S( -48,  -29), S(  -6,   -6),
};
const Score backwardPawnRankBonus[8] = {
S( -15,  -15), S( -17,  -12), S(  -5,   -6), S(  -9,   -3),
S( -16,   -7), S( -15,  -15), S( -15,  -15), S( -15,  -15),
};
const Score KnightMobilityScore[9] = {
S( -35,  -22), S( -17,   14), S(  -5,   24), S(   5,   32),
S(  14,   47), S(  25,   46), S(  36,   43), S(  44,   29),
S(  40,   30),
};
const Score BishopMobilityScore[14] = {
S(  -9,  -18), S(  14,    8), S(  23,   31), S(  37,   45),
S(  45,   58), S(  52,   65), S(  56,   69), S(  61,   76),
S(  64,   72), S(  69,   73), S(  78,   68), S(  57,   81),
S(  81,   57), S(  96,   98),
};
const Score RookMobilityScore[15] = {
S( -24,   56), S( -15,   76), S(  -7,   94), S(  -2,  105),
S(   0,  109), S(   8,  125), S(  17,  126), S(  24,  133),
S(  38,  138), S(  43,  141), S(  51,  149), S(  54,  158),
S(  57,  156), S(  43,  161), S(  70,  175),
};
const Score QueenMobilityScore[28] = {
S( -66,  -87), S( -18,  -33), S(   4,   -6), S(  12,   39),
S(  20,   67), S(  28,   67), S(  33,  104), S(  41,  112),
S(  49,  111), S(  54,  136), S(  61,  133), S(  65,  144),
S(  67,  153), S(  65,  163), S(  73,  170), S(  69,  187),
S(  71,  194), S(  74,  198), S(  85,  206), S(  84,  211),
S( 117,  193), S( 132,  201), S( 116,  196), S( 144,  201),
S( 124,  190), S( 118,  183), S( 121,  205), S( 119,  221),
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
