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
#include "weights.hpp"
#include <thread>

namespace Yayo {
namespace {
constexpr int TEMPO = 5;
} // namespace

constexpr short gamePhaseValues[] = {0, 1, 1, 2, 4, 0};

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
    int pawnShield[4][NUM_COLOR] = {{0}};
    int kingAttackerCount[16][NUM_COLOR] = {{0}};
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
              (MgScore(pawnScore) * mgPhase + EgScore(pawnScore) * egPhase) /
              24;
        const auto knightVal = (MgScore(knightScore) * mgPhase +
                                EgScore(knightScore) * egPhase) /
                               24;
        const auto bishopVal = (MgScore(bishopScore) * mgPhase +
                                EgScore(bishopScore) * egPhase) /
                               24;
        const auto rookVal =
              (MgScore(rookScore) * mgPhase + EgScore(rookScore) * egPhase) /
              24;
        const auto queenVal =
              (MgScore(queenScore) * mgPhase + EgScore(queenScore) * egPhase) /
              24;

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

        const Score wPcSq = pieceSquare<WHITE>();
        const Score bPcSq = pieceSquare<BLACK>();
        const int mgPcSq = MgScore(wPcSq) - MgScore(bPcSq);
        const int egPcSq = EgScore(wPcSq) - EgScore(bPcSq);

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
        const int mgIsolatedPawn =
              MgScore(wIsolatedPawn) - MgScore(bIsolatedPawn);
        const int egIsolatedPawn =
              EgScore(wIsolatedPawn) - EgScore(bIsolatedPawn);

        const Score wBackwardPawn = backwardPawnScore<WHITE>();
        const Score bBackwardPawn = backwardPawnScore<BLACK>();
        const int mgBackwardPawn =
              MgScore(wBackwardPawn) - MgScore(bBackwardPawn);
        const int egBackwardPawn =
              EgScore(wBackwardPawn) - EgScore(bBackwardPawn);

        const Score wMobility = mobilityScore<WHITE>();
        const Score bMobility = mobilityScore<BLACK>();
        const int mgMobility = MgScore(wMobility) - MgScore(bMobility);
        const int egMobility = EgScore(wMobility) - EgScore(bMobility);

        const Score wPawnShield = pawnShieldScore<WHITE>();
        const Score bPawnShield = pawnShieldScore<BLACK>();
        const int mgPawnShield = MgScore(wPawnShield) - MgScore(bPawnShield);
        const int egPawnShield = EgScore(wPawnShield) - EgScore(bPawnShield);

        const Score wKingAttackers = kingAttackersScore<WHITE>();
        const Score bKingAttackers = kingAttackersScore<BLACK>();
        const int mgKingAttackers =
              MgScore(wKingAttackers) - MgScore(bKingAttackers);
        const int egKingAttackers =
              EgScore(wKingAttackers) - EgScore(bKingAttackers);

        const auto color = (board.turn == WHITE) ? 1 : -1;
        const auto materialScore = wMaterial - bMaterial;
        const int pcSqEval = (mgPcSq * mgPhase + egPcSq * egPhase) / 24;
        const int passedPawnEval =
              (mgPassedPawn * mgPhase + egPassedPawn * egPhase) / 24;
        const int doubledPawnEval =
              (mgDoubledPawn * mgPhase + egDoubledPawn * egPhase) / 24;
        const int isolatedPawnEval =
              (mgIsolatedPawn * mgPhase + egIsolatedPawn * egPhase) / 24;
        const int backwardPawnEval =
              (mgBackwardPawn * mgPhase + egBackwardPawn * egPhase) / 24;
        const int mobilityEval =
              (mgMobility * mgPhase + egMobility * egPhase) / 24;
        const int pawnShieldEval =
              (mgPawnShield * mgPhase + egPawnShield * egPhase) / 24;
        const int kingAttackersEval =
              (mgKingAttackers * mgPhase + egKingAttackers * egPhase) / 24;

        auto eval = TEMPO;
        eval += materialScore + pcSqEval + passedPawnEval + doubledPawnEval +
                isolatedPawnEval + backwardPawnEval + mobilityEval +
                pawnShieldEval + kingAttackersEval;

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
    template <Color C> constexpr Bitboard doubledPawns();
    template <Color C> constexpr Bitboard backwardPawns();

    template <Color C> constexpr Score isolatedPawnPenalty();
    template <Color C> constexpr Score backwardPawnScore();
    template <Color C> constexpr Score passedPawnScore();
    template <Color C> constexpr Score doubledPawnPenalty();
    template <Color C> constexpr Score pieceSquare();
    template <Color C> constexpr Score mobilityScore();
    template <Color C> constexpr Score pawnShieldScore();
    template <Color C> constexpr Score kingAttackersScore();

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
constexpr Score Eval<T>::pawnShieldScore() {
    int mgScore = 0, egScore = 0;

    Bitboard castleSquares = SQUARE_BB(F1) | SQUARE_BB(C1);
    castleSquares =
          C == WHITE ? castleSquares : (SQUARE_BB(F8) | SQUARE_BB(C1));

    Bitboard king = board.pieces(KING, C);

    if (king & castleSquares) {
        Bitboard kAtk = kingAttacks[Sq(king)];
        int count = popcount(kAtk & board.pieces(PAWN));

        if (T) {
            trace.pawnShield[count][C]++;
        }

        mgScore += MgScore(pawnShieldScores[count]);
        egScore += EgScore(pawnShieldScores[count]);
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

        if (RANK_OF(psq) <= 0 || RANK_OF(psq) > 7) {
            continue;
        }

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
    const Bitboard friendlyQueens = queens;

    const Bitboard excludedSquares = enemyPawnAttacks | secondThirdRankPawns |
                                     blockedPawns | friendlyKing |
                                     friendlyQueens | board.pieces(C);
    // enemyPawnAttacks; // | secondThirdRankPawns |
    // blockedPawns | friendlyKing |
    // friendlyQueens;

    int mgScore = 0;
    int egScore = 0;

    while (knights) {
        Square knightSq = Square(lsb_index(knights));
        Bitboard knightMoves = knightAttacks[knightSq] & ~excludedSquares;

        int numMoves = popcount(knightMoves);
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
              getBishopAttacks(bishopSq, excludedSquares) & ~excludedSquares;

        int numMoves = popcount(bishopMoves);
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
              getRookAttacks(rookSq, excludedSquares) & ~excludedSquares;

        int numMoves = popcount(rookMoves);
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
        Bitboard rookMoves =
              getRookAttacks(queenSq, excludedSquares) & ~excludedSquares;
        Bitboard bishopMoves =
              getBishopAttacks(queenSq, excludedSquares) & ~excludedSquares;
        Bitboard queenMoves = rookMoves | bishopMoves;

        int numMoves = popcount(queenMoves);
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

template <Tracing T>
template <Color C>
constexpr Score Eval<T>::kingAttackersScore() {
    constexpr Direction Up = C == WHITE ? NORTH : SOUTH;
    constexpr Direction Down = C == WHITE ? SOUTH : NORTH;
    Bitboard king = board.pieces(KING, C);
    Bitboard kingSafetyRegion = kingAttacks[Sq(king)];
    kingSafetyRegion |= shift<Up>(kingSafetyRegion);
    // kingSafetyRegion |= shift<Up>(kingSafetyRegion);

    int numAttacks = 0;

    Bitboard knights, kings, queenRooks, queenBishops;
    knights = board.pieces(KNIGHT, ~C);
    kings = board.pieces(KING, ~C);
    queenRooks = queenBishops = board.pieces(QUEEN, ~C);
    queenRooks |= board.pieces(ROOK, ~C);
    queenBishops |= board.pieces(BISHOP, ~C);

    Bitboard pawnAttacks = shift<Down>(board.pieces(PAWN, ~C));
    pawnAttacks = shift<EAST>(pawnAttacks) | shift<WEST>(pawnAttacks);

    Bitboard knightAtks = 0;
    while (knights) {
        Square knightSq = Square(lsb_index(knights));
        knightAtks |= knightAttacks[knightSq];
        knights &= knights - 1;
    }

    Bitboard kingAtks = kingAttacks[Sq(board.pieces(KING, ~C))];

    Bitboard orthAttacks = 0;
    while (queenRooks) {
        Square orth = Square(lsb_index(queenRooks));
        orthAttacks |= getRookAttacks(orth, board.pieces());
        queenRooks &= queenRooks - 1;
    }

    Bitboard diagAttacks = 0;
    while (queenBishops) {
        Square diag = Square(lsb_index(queenBishops));
        diagAttacks |= getBishopAttacks(diag, board.pieces());
        queenBishops &= queenBishops - 1;
    }

    Bitboard slidingAttacks = diagAttacks | orthAttacks;

    // Bitboards::print_bitboard(pawnAttacks & kingSafetyRegion);

    // int c = (C == WHITE) ? 0 : 1;
    numAttacks += popcount(pawnAttacks & kingSafetyRegion);
    // std::cout << c << ": "
    //           << "numAttacks: " << numAttacks << std::endl;

    numAttacks += popcount(knightAtks & kingSafetyRegion);
    // std::cout << c << ": "
    //           << "numAttacks: " << numAttacks << std::endl;

    numAttacks += popcount(kingAtks & kingSafetyRegion);
    // std::cout << c << ": "
    //           << "numAttacks: " << numAttacks << std::endl;

    numAttacks += popcount(slidingAttacks & kingSafetyRegion);
    // std::cout << c << ": "
    //           << "numAttacks: " << numAttacks << std::endl;

    if (numAttacks >= 15) {
        numAttacks = 15;
    }

    int mgScore = 0, egScore = 0;

    if (T) {
        trace.kingAttackerCount[numAttacks][C]++;
    }

    // std::cout << "numAttacks: " << numAttacks << std::endl;

    mgScore += MgScore(KingAttackerCountScore[numAttacks]);
    egScore += EgScore(KingAttackerCountScore[numAttacks]);

    return S(mgScore, egScore);
}

} // namespace Yayo
#endif // SEARCH_H_
