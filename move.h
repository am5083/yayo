#ifndef MOVE_H_
#define MOVE_H_
#include "util.h"
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
    unsigned short move;
    long score;
};

constexpr unsigned short encodeMove(Square from, Square to, MoveFlag flags) {
    return int(from) | (int(to) << 6) | (flags << 12);
};
constexpr Square getFrom(Move move) { return Square(move.move & 63); };
constexpr Square getTo(Move move) { return Square((move.move >> 6) & 63); };
constexpr MoveFlag getCapture(Move move) { return MoveFlag((move.move >> 12) & 0x0f); };

constexpr Square getFrom(unsigned short move) { return Square(move & 63); };
constexpr Square getTo(unsigned short move) { return Square((move >> 6) & 63); };
constexpr MoveFlag getCapture(unsigned short move) { return MoveFlag((move >> 12) & 0x0f); };

struct moveList {
    Move moves[256];
    unsigned short nMoves, nTactical, nQuiet;

    inline void addCaptures(Square from, Bitboard attacks) {
        while (attacks) {
            int sq = (__builtin_ffsll(attacks) - 1);
            Bitboard bb = (1 << sq);
            moves[nMoves].move = encodeMove(from, Square(sq), CAPTURE);
            nMoves++;
            nTactical++;
            attacks ^= bb;
        }
    }

    inline void addQuiets(Square from, Bitboard pushes) {
        while (pushes) {
            int sq = (__builtin_ffsll(pushes) - 1);
            Bitboard bb = (1 << sq);
            moves[nMoves].move = encodeMove(from, Square(sq), QUIET);
            nMoves++;
            nTactical++;
            pushes ^= bb;
        }
    }

    inline void addMove(int move, bool promo, bool cap) {
        if (getCapture(move) >= CAPTURE) nTactical++;
        moves[nMoves].move = move;
        nMoves++;
    }
};

static inline void print_move(unsigned short move) {
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





};     // namespace Yayo
#endif // MOVE_H_
