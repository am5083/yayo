#ifndef UTIL_H_
#define UTIL_H_
#include <cstdint>

namespace Yayo {
    typedef uint64_t Bitboard;

    constexpr uint64_t B_ATK_TBL_SIZE = 0x1480;
    constexpr uint64_t R_ATK_TBL_SIZE = 0x190000;

    constexpr int MAX_MOVES = 256;
    constexpr int MAX_PLY   = 246;

    enum Square : int {
        A8 = 0, B8, C8, D8, E8, F8, G8, H8,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A1, B1, C1, D1, E1, F1, G1, H1,

        SQUARE_64 = 64,
        SQUARE_0  = 0,
        SQUARE_CT = 64,
    };

    // decreasing because A8 = 0, H1 = 64
    enum Rank  : int { RANK_8, RANK_7, RANK_6, RANK_5, RANK_4, RANK_3, RANK_2, RANK_1, RANK_MAX };
    enum File  : int { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_MAX };
    enum Color : int { WHITE, BLACK, NO_COLOR, NUM_COLOR = 2 };

    enum PieceT { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, };
    enum Piece {
        NO_PC,
        W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
        B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
        PC_MAX = 16
    };

    enum Direction : int {
        NORTH = 8,
        EAST  = 1,
        SOUTH = -NORTH,
        WEST  = -EAST,

        NORTH_EAST = NORTH + EAST,
        NORTH_WEST = NORTH + WEST,
        SOUTH_EAST = SOUTH + EAST,
        SOUTH_WEST = SOUTH + WEST,
    };

    constexpr Bitboard GET   (Bitboard bb, int n)    { return bb &   (1ULL << n); }
    constexpr Bitboard SET   (Bitboard bb, int n)    { return bb |   (1ULL << n); }
    constexpr Bitboard TOGGLE(Bitboard bb, int n)    { return bb ^=  (1ULL << n); }
    constexpr Bitboard POP   (Bitboard bb, int n)    { return bb &= ~(1ULL << n); }
    constexpr Bitboard OFF   (Bitboard bb, int n)    { return bb &=  (0 << n);    }

    constexpr File FILE_OF(Square sq) {
        return File(sq % 8);
    }

    constexpr Rank RANK_OF(Square sq) {
        return Rank(7 - (sq >> 3));
    }

    constexpr Direction operator+(Direction d1, int d2) { return Direction(int(d1) + int(d2)); }
    constexpr Direction operator-(Direction d1, int d2) { return Direction(int(d1) - int(d2)); }
    constexpr Direction operator-(Direction d) { return -Direction(int(d)); }
    inline Direction& operator+=(Direction& d1, int d2) { return d1 = d1 + Direction(d2); }
    inline Direction& operator-=(Direction& d1, int d2) { return d1 = d1 - Direction(d2); }

    inline Square& operator++(Square& sq) { return sq = Square(int(sq) + 1); }
    inline Square& operator--(Square& sq) { return sq = Square(int(sq) + 1); }
    inline Square& operator++(Square& sq, int) { ++sq; return sq = Square(int(sq)); }
    inline Square& operator--(Square& sq, int) { ++sq; return sq = Square(int(sq)); }
}

#endif // UTIL_H_
