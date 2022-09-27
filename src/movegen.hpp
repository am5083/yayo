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

#ifndef MOVEGEN_H_
#define MOVEGEN_H_
#include "board.hpp"
#include "move.hpp"
#include "util.hpp"
#include <cassert>
#include <thread>

namespace Yayo {

// MoveType
// M_CAPTURE
// M_QUIET
// CHECK_EVASION
// CAP_QUIET
// LEGAL

constexpr int victimScores[7] = {0, 100, 200, 300, 400, 500, 600};
static int mvvLvaTable[7][7];

void initMvvLva();
void printMvvLvaTable();

template <Color Turn, MoveType Type>
constexpr moveList *generatePawnMoves(const Board &board, moveList *mList, const Bitboard targets);

template <Color C, PieceT P, MoveType T>
constexpr moveList *generateMoves(const Board &board, moveList *mList, Bitboard targets);

template <Color C, MoveType T> constexpr moveList *generateLegal(const Board &board, moveList *mList);

constexpr moveList *generateCaptures(const Board &board, moveList *mList);
constexpr moveList *generate(const Board &board, moveList *mList);

void makeNullMove(Board &board);
void unmakeNullMove(Board &board);

void make(Board &board, unsigned short move);
void unmake(Board &board, unsigned short move);

template <Color Turn, MoveType Type>
constexpr moveList *generatePawnMoves(const Board &board, moveList *mList, const Bitboard targets) {
    constexpr Color enemy       = ~Turn;
    constexpr Bitboard Rank7    = (Turn == WHITE) ? RANK_7BB : RANK_2BB;
    constexpr Bitboard Rank3    = (Turn == WHITE) ? RANK_3BB : RANK_6BB;
    constexpr Direction Push    = (Turn == WHITE) ? NORTH : SOUTH;
    constexpr Direction eAttack = Push + EAST;
    constexpr Direction wAttack = Push + WEST;

    Bitboard capture = board.pieces(enemy);
    Bitboard push    = ~board.pieces();

    if (board.checkPcs) {
        push    = targets & ~board.pieces();
        capture = targets & board.checkPcs;
    }

    //(Type == CHECK_EVASION) ? board.bCheckPcs() : (Type == M_QUIET) ? 0 : board.pieces(enemy);
    Bitboard singlePushPawns = board.pieces(PAWN, Turn) & ~Rank7;
    Bitboard promoPawns      = board.pieces(PAWN, Turn) & Rank7;

    if (Type != M_CAPTURE) {
        Bitboard singlePush = shift<Push>(singlePushPawns) & ~board.pieces();
        Bitboard doublePush = shift<Push>(singlePush & Rank3) & push;
        singlePush &= push;

        if (Type == CHECK_EVASION) {
            singlePush &= targets;
            doublePush &= targets;
        }

        while (singlePush) {
            const Square s = Square(__builtin_ctzll(singlePush));
            singlePush &= singlePush - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, QUIET), false, false);
        }

        while (doublePush) {
            const Square s = Square(__builtin_ctzll(doublePush));
            doublePush &= doublePush - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push) + int(Push)), s, DOUBLE_PAWN), false, false);
        }
    }

    if (promoPawns) {
        Bitboard ePromoCapture = shift<eAttack>(promoPawns) & capture;
        Bitboard wPromoCapture = shift<wAttack>(promoPawns) & capture;
        Bitboard pushPromo     = shift<Push>(promoPawns) & push;

        if (Type == CHECK_EVASION) {
            pushPromo &= targets;
        }

        while (ePromoCapture) {
            const Square s = Square(__builtin_ctzll(ePromoCapture));
            ePromoCapture &= ePromoCapture - 1;

            Square fromSq = Square(int(s) + int(wAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_ROOK), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_BISHOP), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_KNIGHT), true, true, score);
        }

        while (wPromoCapture) {
            const Square s = Square(__builtin_ctzll(wPromoCapture));
            wPromoCapture &= wPromoCapture - 1;

            Square fromSq = Square(int(s) + int(eAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
        }

        while (pushPromo) {
            const Square s = Square(__builtin_ctzll(pushPromo));
            pushPromo &= pushPromo - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_QUEEN), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_ROOK), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_BISHOP), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_KNIGHT), true, false);
        }
    }

    if (Type == M_CAPTURE || Type == CHECK_EVASION || Type == CAP_QUIET) {

        Bitboard eastAttacks =
            (eAttack == NORTH + EAST ? shift<NORTH + EAST>(singlePushPawns) : shift<SOUTH + EAST>(singlePushPawns)) &
            capture;
        Bitboard westAttacks =
            (wAttack == NORTH + WEST ? shift<NORTH + WEST>(singlePushPawns) : shift<SOUTH + WEST>(singlePushPawns)) &
            capture;

        while (eastAttacks) {
            const Square s = Square(__builtin_ctzll(eastAttacks));
            eastAttacks &= eastAttacks - 1;

            Square fromSq = Square(int(s) + int(wAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CAPTURE), false, true, score);
        }

        while (westAttacks) {
            const Square s = Square(__builtin_ctzll(westAttacks));
            westAttacks &= westAttacks - 1;

            Square fromSq = Square(int(s) + int(eAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(Square(int(s) + int(eAttack)), s, CAPTURE), false, true, score);
        }

        if (board.epSq() != SQUARE_64) {
            if (Type == CHECK_EVASION && (targets & (board.epSq() + Square(Push)))) {
                return mList;
            }

            eastAttacks = singlePushPawns & getPawnAttacks(enemy, board.epSq());
            while (eastAttacks) {
                const Square s      = Square(__builtin_ctzll(eastAttacks));
                const Bitboard pawn = SQUARE_BB(s);
                eastAttacks &= eastAttacks - 1;
                if (board.isSqAttacked(Sq(board.pieces(KING, Turn)), board.pieces() ^ pawn, ~Turn))
                    continue;
                mList->addMove(encodeMove(s, board.epSq(), EP_CAPTURE), false, true, 105);
            }
        }
    }
    return mList;
}

template <Color C, PieceT P, MoveType T>
constexpr moveList *generateMoves(const Board &board, moveList *mList, Bitboard targets) {
    Bitboard Turn = board.pieces(P, C);

    while (Turn) {
        const Square s = Square(__builtin_ctzll(Turn));
        Turn &= Turn - 1;
        Bitboard b = getAttacks<P>(s, board.pieces()) & targets & ~board.pieces(KING);
        while (b) {
            const Square t    = Square(__builtin_ctzll(b));
            const MoveFlag mf = (SQUARE_BB(t) & board.pieces(~C)) ? CAPTURE : QUIET;

            int score = 0;
            if (board.board[t] != NO_PC) {
                PieceT toPc = getPcType(board.board[t]);
                score       = mvvLvaTable[toPc][P] + 1000000;
            }
            mList->addMove(encodeMove(s, t, mf), false, bool(mf), score);
            b &= b - 1;
        }
    }
    return mList;
}
using namespace Yayo;
template <Color C, MoveType T> constexpr moveList *generateLegal(const Board &board, moveList *mList) {
    const Square kingSq = Square(__builtin_ctzll(board.pieces(KING, C)));

    Bitboard orth = getRookAttacks(kingSq, board.pieces(~C));
    Bitboard diag = getBishopAttacks(kingSq, board.pieces(~C));

    Bitboard pinners = (board.pieces(QUEEN, ~C) | board.pieces(ROOK, ~C)) & orth;
    pinners |= (board.pieces(QUEEN, ~C) | board.pieces(BISHOP, ~C)) & diag;
    Bitboard pinned = 0;
    while (pinners) {
        const Square sq = Square(__builtin_ctzll(pinners));
        Bitboard pp     = rectArray[sq][kingSq] & board.pieces(C);
        if (__builtin_popcountll(pp) == 1)
            pinned ^= pp;

        pinners &= pinners - 1;
    }

    moveList temp    = {0};
    Bitboard targets = 0;
    if (T != CHECK_EVASION || __builtin_popcountll(board.bCheckPcs()) < 1) {
        switch (T) {
        case CHECK_EVASION:
            targets = board.between(kingSq, Square(__builtin_ctzll(board.bCheckPcs())));
            break;
        case M_CAPTURE:
            targets = board.pieces(~C);
            break;
        case CAP_QUIET:
            targets = ~board.pieces(C);
            break;
        default:
            targets = 0;
            break;
        }

        if (__builtin_popcountll(board.bCheckPcs()) == 1) {
            targets = LINE[kingSq][__builtin_ctzll(board.checkPcs)] & ~board.pieces(C) & (orth | diag);
            targets |= board.checkPcs;
        } else if (__builtin_popcountll(board.bCheckPcs()) > 1) {
            targets = 0;
        }

        generatePawnMoves<C, T>(board, &temp, targets);
        generateMoves<C, KNIGHT, T>(board, &temp, targets);
        generateMoves<C, BISHOP, T>(board, &temp, targets);
        generateMoves<C, ROOK, T>(board, &temp, targets);
        generateMoves<C, QUEEN, T>(board, &temp, targets);
    }

    Bitboard b = kingAttacks[kingSq] & ~board.pieces(C);
    if (__builtin_popcountll(board.bCheckPcs()) == 1 && !(board.checkPcs & board.pieces(PAWN, ~C))) {
        Bitboard pinmask = LINE[__builtin_ctzll(board.checkPcs)][kingSq];
        b &= ~(pinmask & ~board.checkPcs);
    }

    if (T == M_CAPTURE) {
        b &= targets;
    }

    while (b) {
        const Square s = Square(__builtin_ctzll(b));
        b &= b - 1;
        if (board.isSqAttacked(s, board.pieces() ^ board.pieces(KING, C), ~C)) {
            continue;
        }

        const MoveFlag mf = (SQUARE_BB(s) & board.pieces(~C)) ? CAPTURE : QUIET;

        int score = 0;
        if (board.board[s] != NO_PC) {
            PieceT toPc = getPcType(board.board[s]);
            score       = mvvLvaTable[toPc][KING] + 1000000;
        }

        temp.addMove(encodeMove(kingSq, s, mf), false, bool(mf), score);
    }

    if ((T == M_QUIET || T == CAP_QUIET) && board.canCastle(C)) {
        CastleRights cr    = board.canCastle(C);
        const bool inCheck = board.bCheckPcs();
        if (cr & KING_SIDE && !inCheck) {
            Bitboard betweenMask = (3ULL << (kingSq + 1));
            if (!(betweenMask & board.pieces())) {
                Square to     = SQUARE_64;
                bool attacked = false;
                while (betweenMask && !attacked) {
                    to = Square(__builtin_ctzll(betweenMask));
                    betweenMask &= betweenMask - 1;
                    attacked = attacked || board.attacksToKing<~C>(to, board.pieces());
                }

                if (!attacked) {
                    temp.addMove(encodeMove(kingSq, Square(kingSq + 2), K_CASTLE), false, false);
                }
            }
        }

        if (cr & QUEEN_SIDE && !inCheck) {
            Bitboard betweenMask = (3ULL << (kingSq - 2));
            if (!((7ULL << (kingSq - 3)) & board.pieces())) {
                Square to     = SQUARE_64;
                bool attacked = false;
                while (betweenMask && !attacked) {
                    to = Square(__builtin_ctzll(betweenMask));
                    betweenMask &= betweenMask - 1;
                    attacked = attacked || board.attacksToKing<~C>(to, board.pieces());
                }

                if (!attacked) {
                    temp.addMove(encodeMove(kingSq, Square(kingSq - 2), Q_CASTLE), false, false);
                }
            }
        }
    }

    int j = 0;
    for (int i = 0; i < temp.nMoves; i++) {
        Square fromSq    = getFrom(temp.moves[i]);
        Bitboard fromMsk = SQUARE_BB(fromSq);
        if (pinned && (pinned & fromMsk)) {
            if (LINE[kingSq][fromSq] & SQUARE_BB(getTo(temp.moves[i]))) {
                mList->moves[j] = temp.moves[i];
                mList->nMoves++;
                j++;
            }
        } else {
            mList->moves[j] = temp.moves[i];
            mList->nMoves++;
            j++;
        }
    }

    return mList;
}

constexpr moveList *generateCaptures(const Board &board, moveList *mList) {
    (board.turn == WHITE) ? generateLegal<WHITE, M_CAPTURE>(board, mList)
                          : generateLegal<BLACK, M_CAPTURE>(board, mList);
    return mList;
}

constexpr moveList *generate(const Board &board, moveList *mList) {
    (board.turn == WHITE) ? generateLegal<WHITE, CAP_QUIET>(board, mList)
                          : generateLegal<BLACK, CAP_QUIET>(board, mList);
    return mList;
}

} // namespace Yayo

#endif // MOVEGEN_H_
