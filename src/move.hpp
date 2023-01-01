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

#ifndef MOVE_H_
#define MOVE_H_
#include "util.hpp"
#include <iostream>

/*
  Move information is encoded as bits in a 16-bit word
         to sq
           v
  000000 000000 0000
    ^            ^
  from sq      flags

  Move encoding:

     binary encoding   size       value       hex val
  0000 0000 0011 1111 (6 bit)  from square     0x3f
  0000 1111 1100 0000 (6 bit)  to square       0xfc0
  0011 0000 0000 0000 (2 bit)  flags           0x3000
  1100 0000 0000 0000 (2 bit)  promotions      0xC000
 */

namespace Yayo {

enum MoveFlag {
    QUIET,
    DOUBLE_PAWN,
    K_CASTLE,
    Q_CASTLE,
    CAPTURE,
    EP_CAPTURE,
    P_KNIGHT,  // knight promotion
    P_BISHOP,  // bishop promotion
    P_ROOK,    // rook promotion
    P_QUEEN,   // queen promotion
    CP_KNIGHT, // knight capture promotion
    CP_BISHOP, // bishop capture promotion
    CP_ROOK,   // rook capture promotion
    CP_QUEEN,  // queen capture promotion
};

struct Move {
    unsigned move;
    int score;
};

constexpr unsigned short encodeMove(Square from, Square to, MoveFlag flags) {
    return int(from) | (int(to) << 6) | (flags << 12);
};

constexpr Square getFrom(Move move) { return Square(move.move & 63); };
constexpr Square getTo(Move move) { return Square((move.move >> 6) & 63); };
constexpr MoveFlag getCapture(Move move) {
    return MoveFlag((move.move >> 12) & 0x0f);
};

constexpr Square getFrom(unsigned short move) { return Square(move & 63); };
constexpr Square getTo(unsigned short move) {
    return Square((move >> 6) & 63);
};
constexpr MoveFlag getCapture(unsigned short move) {
    return MoveFlag((move >> 12) & 0x0f);
};

static inline std::string move_str(unsigned short move) {
    int to = getTo(move), from = getFrom(move), flags = getCapture(move);
    std::string m;
    m += nToSq[from];
    m += nToSq[to];
    m += flags;

    return m;
}

void print_move(unsigned short move);

struct moveList {
    Move moves[256];
    unsigned short nMoves = 0, nTactical = 0, nQuiet = 0;

    void print() const;

    void print(int flag) const;

    void addCaptures(Square from, Bitboard attacks);

    void addQuiets(Square from, Bitboard pushes);

    void addMove(int move, bool promo, bool cap);

    void addMove(int move, bool promo, bool cap, int score);

    void swapBest(int index);

    moveList &operator+=(moveList m2) {
        int j = 0;

        const int tnMoves = this->nMoves, onMoves = m2.nMoves;
        for (int i = tnMoves; i < (tnMoves + onMoves); i++) {
            this->moves[i] = m2.moves[j];
            this->nMoves++;
            j++;
        }
        return *this;
    }

    void print() {
        for (int i = 0; i < nMoves; i++) {
            print_move(moves[i].move);
            std::cout << ": " << moves[i].score << "\n";
        }
    }
};

};     // namespace Yayo
#endif // MOVE_H_
