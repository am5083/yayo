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

#include "move.hpp"

void Yayo::moveList::print() const {
    for (int i = 0; i < nMoves; i++) {
        print_move(moves[i].move);
        std::cout << std::endl;
    }
}

void Yayo::moveList::print(int flag) const {
    for (int i = 0; i < nMoves; i++) {
        switch (flag) {
        case 1:
            print_move(moves[i].move);
            std::cout << ": " << moves[i].score;
            std::cout << std::endl;
            break;
        default:
            print_move(moves[i].move);
            std::cout << std::endl;
            break;
        }
    }
}

void Yayo::moveList::addCaptures(Square from, Bitboard attacks) {
    while (attacks) {
        int sq             = (__builtin_ffsll(attacks) - 1);
        Bitboard bb        = (1 << sq);
        moves[nMoves].move = encodeMove(from, Square(sq), CAPTURE);
        nMoves++;
        nTactical++;
        attacks ^= bb;
    }
}

void Yayo::moveList::addQuiets(Square from, Bitboard pushes) {
    while (pushes) {
        int sq             = (__builtin_ffsll(pushes) - 1);
        Bitboard bb        = (1 << sq);
        moves[nMoves].move = encodeMove(from, Square(sq), QUIET);
        nMoves++;
        nTactical++;
        pushes ^= bb;
    }
}

void Yayo::moveList::addMove(int move, bool promo, bool cap) {
    if (getCapture(move) >= CAPTURE)
        nTactical++;
    moves[nMoves].move = move;
    nMoves++;
}

void Yayo::moveList::addMove(int move, bool promo, bool cap, int score) {
    if (getCapture(move) >= CAPTURE)
        nTactical++;
    moves[nMoves].move  = move;
    moves[nMoves].score = score;
    nMoves++;
}

void Yayo::moveList::swapBest(int index) {
    if (nMoves < 2)
        return;

    int best_index = -1;
    Move best = moves[index], cur = moves[index];
    for (int i = index; i < nMoves; i++) {
        if (moves[i].score > best.score) {
            best_index = i;
            best       = moves[i];
        }
    }

    if (best_index == -1)
        return;

    moves[index]      = best;
    moves[best_index] = cur;
}

void Yayo::print_move(unsigned short move) {
    int to = getTo(move), from = getFrom(move), flags = getCapture(move);
    switch (flags) {
    case P_KNIGHT:
    case CP_KNIGHT:
        printf("%s%sn", nToSq[from].c_str(), nToSq[to].c_str());
        break;
    case P_BISHOP:
    case CP_BISHOP:
        printf("%s%sb", nToSq[from].c_str(), nToSq[to].c_str());
        break;
    case P_ROOK:
    case CP_ROOK:
        printf("%s%sr", nToSq[from].c_str(), nToSq[to].c_str());
        break;
    case P_QUEEN:
    case CP_QUEEN:
        printf("%s%sq", nToSq[from].c_str(), nToSq[to].c_str());
        break;
    default:
        printf("%s%s", nToSq[from].c_str(), nToSq[to].c_str());
        break;
    }
}
