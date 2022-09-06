#ifndef BITBOARDS_H_
#define BITBOARDS_H_
#include "defs.h"
#include "random.h"
#include "util.h"
#include <array>
#include <cassert>
#include <immintrin.h>

namespace Yayo {

extern Bitboard bishopAttacks[B_ATK_TBL_SIZE];
extern Bitboard rookAttacks[R_ATK_TBL_SIZE];
extern Bitboard LINE[SQUARE_CT][SQUARE_CT];

extern Bitboard northPassedPawns[SQUARE_CT];
extern Bitboard southPassedPawns[SQUARE_CT];

struct Magic {
    Bitboard mask;
    Bitboard magic;
    Bitboard *attacks;
    unsigned int shift;

    constexpr uint64_t index(Bitboard occ) const { return __builtin_ia32_pext_di(occ, mask); };
    constexpr Bitboard c_attacks(Bitboard occ) const { return attacks[index(occ)]; }
};

extern Magic bishopMagics[SQUARE_CT];
extern Magic rookMagics[SQUARE_CT];

extern std::uint64_t zobristBlackToMove;
extern std::uint64_t zobristEpFile[8];
extern std::uint64_t zobristCastleRights[16];
extern std::uint64_t zobristPieceSq[16][64];

constexpr Bitboard FILE_BB(File f) { return A_FILEBB << f; };

constexpr Bitboard RANK_BB(Rank r) { return RANK_1BB >> (r * 8); };

constexpr Bitboard SQUARE_BB(Square n) { return 1ULL << n; };

constexpr Bitboard operator&(Bitboard b, Rank r) { return b & RANK_BB(r); }

template <Direction D> constexpr Bitboard shift(Bitboard b) {
    return D == NORTH                   ? b >> 8
           : D == SOUTH                 ? b << 8
           : D == NORTH + NORTH         ? b >> 16
           : D == SOUTH + SOUTH + SOUTH ? b << 32
           : D == NORTH + NORTH + NORTH ? b >> 32
           : D == SOUTH + SOUTH         ? b << 16
           : D == EAST                  ? (b << 1) & ~A_FILEBB
           : D == WEST                  ? (b >> 1) & ~H_FILEBB
           : D == NORTH + EAST          ? (b >> 7) & ~A_FILEBB
           : D == NORTH + WEST          ? (b >> 9) & ~H_FILEBB
           : D == SOUTH + EAST          ? (b << 9) & ~A_FILEBB
           : D == SOUTH + WEST          ? (b << 7) & ~H_FILEBB
           : D == NORTH + NORTH_EAST    ? (b >> 15) & ~A_FILEBB
           : D == NORTH + NORTH_WEST    ? (b >> 17) & ~H_FILEBB
           : D == SOUTH + SOUTH_EAST    ? (b << 15) & ~A_FILEBB
           : D == SOUTH + SOUTH_WEST    ? (b << 17) & ~H_FILEBB
                                        : 0;
}

template <Direction D> constexpr Bitboard shift(Square s) { return shift<D>(SQUARE_BB(s)); }

template <Direction D> constexpr Bitboard fill(Square s) {
    Bitboard sq = SQUARE_BB(s);
    sq |= shift<D>(sq);
    sq |= shift<D + D>(sq);
    sq |= shift<D + D + D>(sq);
    return sq;
}

template <Direction D> constexpr Bitboard fill(Bitboard sq) {
    sq |= shift<D>(sq);
    sq |= shift<D + D>(sq);
    sq |= shift<D + D + D>(sq);
    return sq;
}

template <Color C> constexpr Bitboard passedPawnMask(Bitboard sq) {
    constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
    sq                    = shift<D>(sq);
    return fill<D>(shift<WEST>(sq)) | fill<D>(sq) | fill<D>(shift<EAST>(sq));
}

template <Color C> constexpr Bitboard passedPawnMask(Square s) {
    constexpr Direction D = (C == WHITE) ? NORTH : SOUTH;
    const Bitboard sq     = shift<D>(SQUARE_BB(s));

    if (C == WHITE)
        return fill<NORTH>(shift<WEST>(sq)) | fill<NORTH>(sq) | fill<NORTH>(shift<EAST>(sq));

    return fill<SOUTH>(shift<WEST>(sq)) | fill<SOUTH>(sq) | fill<SOUTH>(shift<EAST>(sq));
}

constexpr Bitboard maskKingAttacks(Bitboard b) {
    b |= shift<EAST>(b) | shift<WEST>(b);
    b |= shift<SOUTH>(b) | shift<NORTH>(b);
    return b;
}

template <Color C> Bitboard pawnSglAttacks(Bitboard b) = delete;
template <> constexpr Bitboard pawnSglAttacks<WHITE>(Bitboard b) { return shift<NORTH_EAST>(b) & shift<NORTH_WEST>(b); }
template <> constexpr Bitboard pawnSglAttacks<BLACK>(Bitboard b) { return shift<SOUTH_EAST>(b) & shift<SOUTH_WEST>(b); }

template <Color C> Bitboard pawnDblAttacks(Bitboard b) = delete;
template <> constexpr Bitboard pawnDblAttacks<WHITE>(Bitboard b) {
    Bitboard attacks = shift<NORTH>(b);
    attacks          = shift<EAST>(attacks) | shift<WEST>(attacks);
    return attacks;
}
template <> constexpr Bitboard pawnDblAttacks<BLACK>(Bitboard b) {
    Bitboard attacks = shift<SOUTH>(b);
    attacks          = shift<EAST>(attacks) | shift<WEST>(attacks);
    return attacks;
}

constexpr Bitboard knightAllAttacks(Bitboard b) {
    Bitboard bb  = (shift<NORTH + NORTH>(b) | shift<SOUTH + SOUTH>(b));
    Bitboard bb1 = shift<EAST>(shift<EAST>(b)) | shift<WEST>(shift<WEST>(b));
    return shift<EAST>(bb) | shift<WEST>(bb) | shift<NORTH>(bb1) | shift<SOUTH>(bb1);
}

constexpr Bitboard in_between(Square from, Square to) {
    int frInt = int(from);
    int toInt = int(to);

    constexpr Bitboard m    = -1ULL;
    constexpr Bitboard a2a7 = 0x0001010101010100ULL;
    constexpr Bitboard b2g7 = 0x0040201008040200ULL;
    constexpr Bitboard h1b7 = 0x0002040810204080ULL;

    const Bitboard fd = (frInt & 7) - (toInt & 7);
    const Bitboard rd = (frInt >> 3) - (toInt >> 3);
    const Bitboard b  = (m << from) ^ (m << to);

    Bitboard line;
    line = ((fd & 7) - 1) & a2a7;
    line += 2 * (((rd & 7) - 1) >> 58);    /* b8g8 if same rank     */
    line += (((rd - fd) & 15) - 1) & b2g7; /* b2g7 if same diagonal */
    line += (((rd + fd) & 15) - 1) & h1b7; /* h1b7 if same antidiag */
    line *= b & -b;

    return line & b;
}

constexpr std::array<std::array<Bitboard, 64>, 64> rectArray = {
    []() constexpr {std::array<std::array<Bitboard, 64>, 64> arr;
for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
        arr[i][j] = in_between(Square(i), Square(j));
    }
}
return arr;
} // namespace Yayo
()
}
;

constexpr std::array<std::array<Bitboard, 64>, 2> pawnAttacks = {
    []() constexpr {std::array<std::array<Bitboard, 64>, 2> arr;
for (int i = 0; i < 64; i++) {
    arr[WHITE][i] = pawnDblAttacks<WHITE>(SQUARE_BB(Square(i)));
    arr[BLACK][i] = pawnDblAttacks<BLACK>(SQUARE_BB(Square(i)));
}
return arr;
}
()
}
;

constexpr std::array<Bitboard, 64> knightAttacks = {[]() constexpr {std::array<Bitboard, SQUARE_CT> arr;
for (int i = 0; i < 64; i++)
    arr[i] = knightAllAttacks(SQUARE_BB(Square(i)));
return arr;
}
()
}
;

constexpr std::array<Bitboard, 64> kingAttacks = {[]() constexpr {std::array<Bitboard, SQUARE_CT> arr;
for (int i = 0; i < 64; i++)
    arr[i] = maskKingAttacks(SQUARE_BB(Square(i)));
return arr;
}
()
}
;

constexpr Bitboard getRookAttacks(Square s, Bitboard b) {
    assert(rookMagics[s].index(b) < R_ATK_TBL_SIZE);
    assert(rookMagics[s].index(b) >= 0);
    return rookMagics[s].c_attacks(b);
}
constexpr Bitboard getBishopAttacks(Square s, Bitboard b) {
    assert(bishopMagics[s].index(b) < B_ATK_TBL_SIZE);
    assert(bishopMagics[s].index(b) >= 0);
    return bishopMagics[s].c_attacks(b);
}
constexpr Bitboard getPawnAttacks(Color c, Square s) { return pawnAttacks[c][s]; }

template <Color C> constexpr Bitboard getPassMask(int s) {
    return (C == WHITE) ? northPassedPawns[s] : southPassedPawns[s];
}

template <PieceT P> constexpr Bitboard getAttacks(Square s) { return 0; }
template <> constexpr Bitboard getAttacks<KNIGHT>(Square s) { return knightAttacks[s]; }
template <> constexpr Bitboard getAttacks<KING>(Square s) { return kingAttacks[s]; }

template <PieceT P> constexpr Bitboard getAttacks(Square s, Bitboard b) { return 0; };
template <> constexpr Bitboard getAttacks<BISHOP>(Square s, Bitboard b) { return bishopMagics[s].c_attacks(b); }
template <> constexpr Bitboard getAttacks<ROOK>(Square s, Bitboard b) { return rookMagics[s].c_attacks(b); }
template <> constexpr Bitboard getAttacks<QUEEN>(Square s, Bitboard b) {
    return getAttacks<ROOK>(s, b) | getAttacks<BISHOP>(s, b);
}
template <> constexpr Bitboard getAttacks<KNIGHT>(Square s, Bitboard) { return knightAttacks[s]; }
template <> constexpr Bitboard getAttacks<KING>(Square s, Bitboard) { return kingAttacks[s]; }

namespace Bitboards {
void init_arrays();
void print_bitboard(Bitboard bitboard);
} // namespace Bitboards
} // namespace Yayo

#endif // BITBOARDS_H_
