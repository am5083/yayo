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

#ifndef BOARD_H_
#define BOARD_H_
#include "bitboard.hpp"
#include "move.hpp"
#include "util.hpp"
#include <cstdint>
#include <string>

namespace Yayo {

// clang-format off
struct Hist {
    uint64_t key      = 0;

    Bitboard checkPcs = 0;
    Piece lastCapt    = NO_PC;
    Square enPass     = SQUARE_64;

    int castleStatus  = 0;
    int halfMoves     = 0;
    int fullMoves     = 0;
};

struct Info {
    unsigned long long nodes   = 0;

    long double startTime      = -1.0;
    long double stopTime       = -1.0;
    long double timeControl    = -1.0;
    long double maxTimeControl = -1.0;

    int depth                  = -1;
    int selDepth               = -1;
    int movestogo              = -1;

    bool timeGiven             = false;
    bool uciQuit               = false;
    bool uciStop = false;
};

enum CastleRights : int {
    WHITE_KING  = 1 << 3,
    WHITE_QUEEN = 1 << 2,
    BLACK_KING  = 1 << 1,
    BLACK_QUEEN = 1,
    KING_SIDE   = (1 << 3) | (1 << 1),
    QUEEN_SIDE  = 1 | (1 << 2),
    NO_CASTLE_W = -1,
    NO_CASTLE_B = -1,
    NO_CASTLE   = 0,
};

static const int castleMod[64] = {
    14, 15, 15, 15, 12, 15, 15, 13,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    11, 15, 15, 15, 3,  15, 15, 7,
};

class Board {
  public:
    Hist hist[1000];
    Piece board[64];
    Bitboard color[2];
    Bitboard pieceBB[PC_MAX];
    Bitboard cPieceBB[7];

    mutable Bitboard checkPcs;
    uint64_t key;

    Piece lastCapt;
    Color turn;
    int castleRights;
    Square enPass;
    int ply, gamePly;
    int halfMoves, fullMoves;

    Board();
    Board(const Board &other);

    constexpr bool operator==(const Board &b1) const;

    constexpr int numRepetition() const;
    constexpr bool isRepetition() const;
    constexpr bool isTMR() const;

    void print() const;
    void setFen(const std::string fen);
    std::string fen() const;

    constexpr Bitboard pieces() const { return this->color[WHITE] | this->color[BLACK]; }
    constexpr Bitboard pieces(PieceT p, Color c) const { return this->color[c] & this->cPieceBB[p]; };
    constexpr Bitboard pieces(Color c) const { return color[c]; }
    constexpr Bitboard pieces(PieceT p) const { return cPieceBB[p]; }

    constexpr Color side() const { return turn; }

    constexpr CastleRights canCastle(Color c) const;
    constexpr Square epSq() const { return enPass; };
    constexpr Bitboard bCheckPcs() const { return checkPcs; };
    constexpr Bitboard between(const Square s1, const Square s2) const { return rectArray[s1][s2]; };
    constexpr Bitboard xRayAtks(Square sq, Bitboard occ);
    constexpr Bitboard getLVA(Color side, Bitboard atkDefMap, Piece *p);
    int see(Square toSq, Piece toPc, Square from, Piece fromPc);
    constexpr bool castleBlocked(CastleRights cr, Square sq) const;
    constexpr bool isSqAttacked(Square sq, Bitboard occ, Color byColor) const;
    constexpr bool isDraw();
    std::uint64_t hash() const;

    template <Color C> constexpr Bitboard attacksToKing(Square sq, Bitboard occ) const {
        Bitboard knights, kings, queenRooks, queenBishops;
        knights = pieces(KNIGHT, C);
        kings = pieces(KING, C);
        queenRooks = queenBishops = pieces(QUEEN, C);
        queenRooks |= pieces(ROOK, C);
        queenBishops |= pieces(BISHOP, C);

        return (pawnAttacks[~C][sq] & pieces(PAWN, C)) | (knightAttacks[sq] & knights) | (kingAttacks[sq] & kings) |
               (getBishopAttacks(sq, occ) & queenBishops) | (getRookAttacks(sq, occ) & queenRooks);
    }

    constexpr Bitboard attacksToSq(Square sq, Bitboard occ) const {
        Bitboard knights, kings, queenRooks, queenBishops;
        knights = pieces(KNIGHT);
        kings = pieces(KING);
        queenRooks = queenBishops = pieces(QUEEN);
        queenRooks |= pieces(ROOK);
        queenBishops |= pieces(BISHOP);
        Bitboard pawns = pawnAttacks[WHITE][sq] | pawnAttacks[BLACK][sq];

        return (pawns & pieces(PAWN)) | (knightAttacks[sq] & knights) | (kingAttacks[sq] & kings) | (getBishopAttacks(sq, occ) & queenBishops)
            | (getRookAttacks(sq, occ) & queenRooks);
    }
};

constexpr bool Board::isDraw() {
    const int num    = popcount(color[0] | color[1]);
    const bool kvk   = num == 2;
    const bool kvbk  = (num == 3) && (cPieceBB[BISHOP]);
    const bool kvnk  = (num == 3) && (cPieceBB[KNIGHT]);
    const bool kvnnk = (num == 4) && (popcount(cPieceBB[KNIGHT]) == 2);

    return kvk || kvbk || kvnk || kvnnk;
}

constexpr bool Board::isSqAttacked(Square sq, Bitboard occ, Color byColor) const {
    Bitboard pawns, knights, kings, bishopQueens, rookQueens;
    pawns = pieces(PAWN, byColor);
    if (pawnAttacks[~byColor][sq] & pawns)
        return true;
    knights = pieces(KNIGHT, byColor);
    if (knightAttacks[sq] & knights)
        return true;
    kings = pieces(KING, byColor);
    if (kingAttacks[sq] & kings)
        return true;
    rookQueens   = pieces(QUEEN, byColor);
    bishopQueens = rookQueens | pieces(BISHOP, byColor);
    if (getBishopAttacks(sq, occ) & bishopQueens)
        return true;
    rookQueens |= pieces(ROOK, byColor);
    if (getRookAttacks(sq, occ) & rookQueens)
        return true;

    return false;
}

constexpr CastleRights Board::canCastle(Color c) const {
    const int kingSide  = castleRights & KING_SIDE;
    const int queenSide = castleRights & QUEEN_SIDE;

    return CastleRights((c == WHITE) ? ((kingSide & 8) | (queenSide & 4)) : ((kingSide & 2) | (queenSide & 1)));
};

constexpr bool Board::isRepetition() const {
    for (int i = gamePly - halfMoves; i < gamePly; ++i) {
        if (i < 0) continue;
        if (this->key == hist[i].key)
            return true;
    }

    return false;
}

constexpr bool Board::isTMR() const {
    int n = 1;
        for (int i = gamePly - 2; i >= 0; i -= 2) {
        if (i < gamePly - halfMoves) break;
        if (key == hist[i].key) {
            n++;
            if (i > gamePly - ply) return true;
            if (n == 3) return true;
        }
    }
    return false;
}

constexpr int Board::numRepetition() const {
    int num_rep = 0;
    for (int i = gamePly - halfMoves; i < gamePly; i++) {
        if (this->key == hist[i].key)
            num_rep++;
    }
    return num_rep;
}

constexpr Bitboard Board::xRayAtks(Square sq, Bitboard occ) {
    Bitboard queenRooks = 0, queenBishops = 0;

    queenBishops = pieces(QUEEN);
    queenRooks   = queenBishops;
    queenBishops |= pieces(BISHOP);
    queenRooks |= pieces(ROOK);

    queenRooks &= occ;
    queenBishops &= occ;

    const Bitboard q_b = getBishopAttacks(sq, occ) & queenBishops;
    const Bitboard q_r = getRookAttacks(sq, occ) & queenRooks;

    return q_b | q_r;
}

constexpr Bitboard Board::getLVA(Color side, Bitboard atkDefMap, Piece *p) {
    for (PieceT pt = PAWN; pt <= KING; pt = pt + 1) {
        *p              = getCPiece(side, pt);
        Bitboard atkMap = atkDefMap & pieces(pt, side);
        if (atkMap)
            return atkMap & -atkMap;
    }

    return 0;
}

constexpr bool Board::castleBlocked(CastleRights cr, Square sq) const {
    const Bitboard queenOccupancy = (7 << (int(sq) - 3));
    const Bitboard kingOccupancy  = (3 << (int(sq) + 1));

    if (checkPcs)
        return true;
    if (cr & QUEEN_SIDE)
        return !(queenOccupancy & pieces());
    if (cr & KING_SIDE)
        return !(kingOccupancy & pieces());
    return false;
};

bool isNoisy(Board &board, unsigned short move);
bool isQuiet(Board &board, unsigned short move);

} // namespace Yayo
#endif // BOARD_H_
