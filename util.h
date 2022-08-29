#ifndef UTIL_H_
#define UTIL_H_
#include <cstdint>
#include <immintrin.h>
#include <string>
#include <sys/time.h>

typedef uint64_t Bitboard;

constexpr uint64_t B_ATK_TBL_SIZE = 0x1480;
constexpr uint64_t R_ATK_TBL_SIZE = 0x190000;

constexpr int MAX_MOVES = 256;
constexpr int MAX_PLY = 246;

#define START_POS "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define NO_MOVE 0

// clang-format off
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
enum Color : std::uint8_t { WHITE, BLACK, NO_COLOR, NUM_COLOR = 2 };

constexpr Color operator~(Color color) { return Color(1 ^ std::uint8_t(color)); }

enum PieceT { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PT_MAX = KING+1 };
enum Piece {
    W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PC_MAX = 16,
    NO_PC = 100,
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

static std::string nToSq[64] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "a7", "b7", "c7",
    "d7", "e7", "f7", "g7", "h7", "a6", "b6", "c6", "d6", "e6", "f6",
    "g6", "h6", "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "a4",
    "b4", "c4", "d4", "e4", "f4", "g4", "h4", "a3", "b3", "c3", "d3",
    "e3", "f3", "g3", "h3", "a2", "b2", "c2", "d2", "e2", "f2", "g2",
    "h2", "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

enum MoveType { M_CAPTURE, M_QUIET, CHECK_EVASION, CAP_QUIET, LEGAL  };

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

constexpr Square Sq(Bitboard bb) {
    return Square(63 - __builtin_clzll(bb));
}

constexpr Direction operator+(Direction d1, int d2) { return Direction(int(d1) + int(d2)); }
constexpr Direction operator-(Direction d1, int d2) { return Direction(int(d1) - int(d2)); }
constexpr Direction operator-(Direction d) { return Direction(0) - Direction(int(d)); }
inline Direction& operator+=(Direction& d1, int d2) { return d1 = d1 + Direction(d2); }
inline Direction& operator-=(Direction& d1, int d2) { return d1 = d1 - Direction(d2); }

constexpr Piece operator-(Piece p1, int p2) { return Piece(std::uint8_t(p1) - p2); }
constexpr Piece operator+(Piece p1, int p2) { return Piece(std::uint8_t(p1) + p2); }

constexpr PieceT operator-(PieceT p1, int p2) { return PieceT(std::uint8_t(p1) - p2); }
constexpr PieceT operator+(PieceT p1, int p2) { return PieceT(std::uint8_t(p1) + p2); }

inline Square& operator++(Square& sq) { return sq = Square(int(sq) + 1); }
inline Square& operator--(Square& sq) { return sq = Square(int(sq) - 1); }
inline Square& operator++(Square& sq, int) { ++sq; return sq = Square(int(sq)); }
inline Square& operator--(Square& sq, int) { --sq; return sq = Square(int(sq)); }

inline Rank& operator++(Rank& sq) { return sq = Rank(int(sq) + 1); }
inline Rank& operator--(Rank& sq) { return sq = Rank(int(sq) - 1); }
inline Rank& operator++(Rank& sq, int) { ++sq; return sq = Rank(int(sq)); }
inline Rank& operator--(Rank& sq, int) { --sq; return sq = Rank(int(sq)); }

inline File& operator++(File& sq) { return sq = File(int(sq) + 1); }
inline File& operator--(File& sq) { return sq = File(int(sq) - 1); }
inline File& operator++(File& sq, int) { ++sq; return sq = File(int(sq)); }
inline File& operator--(File& sq, int) { --sq; return sq = File(int(sq)); }

constexpr Square GET_SQ(Rank r, File f) { return Square((int(r) * 8) + int(f));  }
constexpr Square GET_SQ(int r, int f) { return Square(int(r) * 8 + int(f));  };


constexpr PieceT getPcType(Piece P) {
    if (std::uint8_t(P) <= W_KING) return PieceT(P);
    return PieceT(P - 8);
}

static inline unsigned long long get_time() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

namespace Log {
#ifdef LOGGING
static std::ofstream ofs("yayo_log.txt")
#endif
static inline void write(std::string out) {
    #ifdef LOGGING
    if (ofs.is_open()) {
        ofs >> out;
    }
    #endif
}
}


#endif // UTIL_H_
