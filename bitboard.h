#ifndef BITBOARDS_H_
#define BITBOARDS_H_
#include "defs.h"
#include "util.h"
#include <array>
#include <immintrin.h>

namespace Yayo {

constexpr Bitboard A_FILEBB = 0x101010101010101;
constexpr Bitboard B_FILEBB = 0x202020202020202;
constexpr Bitboard C_FILEBB = 0x404040404040404;
constexpr Bitboard D_FILEBB = 0x808080808080808;
constexpr Bitboard E_FILEBB = 0x1010101010101010;
constexpr Bitboard F_FILEBB = 0x2020202020202020;
constexpr Bitboard G_FILEBB = 0x4040404040404040;
constexpr Bitboard H_FILEBB = 0x8080808080808080;

constexpr Bitboard RANK_1BB = 0xff00000000000000;
constexpr Bitboard RANK_2BB = 0x00ff000000000000;
constexpr Bitboard RANK_3BB = 0x0000ff0000000000;
constexpr Bitboard RANK_4BB = 0x000000ff00000000;
constexpr Bitboard RANK_5BB = 0x00000000ff000000;
constexpr Bitboard RANK_6BB = 0x0000000000ff0000;
constexpr Bitboard RANK_7BB = 0x000000000000ff00;
constexpr Bitboard RANK_8BB = 0x00000000000000ff;

extern Bitboard bishopAttacks[B_ATK_TBL_SIZE];
extern Bitboard rookAttacks[R_ATK_TBL_SIZE];

extern Bitboard rectArray[SQUARE_CT];
extern Bitboard pawnAttacks[2][SQUARE_CT];
extern Bitboard knightAttacks[SQUARE_CT];
extern Bitboard kingAttacks[SQUARE_CT];

struct Magic {
    Bitboard mask;
    Bitboard magic;
    Bitboard *attacks;
    unsigned int shift;

    uint64_t index(Bitboard occ) { return _pext_u64(occ, this->mask); };
    Bitboard c_attacks(Bitboard occ) { return attacks[index(occ)]; }
};

extern Magic bishopMagics[SQUARE_CT];
extern Magic rookMagics[SQUARE_CT];

constexpr Bitboard FILE_BB(File f) { return A_FILEBB << f; };

constexpr Bitboard RANK_BB(Rank r) { return RANK_1BB >> (r * 8); };

constexpr Bitboard SQUARE_BB(Square n) { return 1ULL << n; };

template <Direction D> constexpr Bitboard shift(Bitboard b) {
    return D == NORTH                ? b >> 8
           : D == SOUTH              ? b << 8
           : D == NORTH + NORTH      ? b >> 16
           : D == SOUTH + SOUTH      ? b << 16
           : D == EAST               ? (b << 1) & ~A_FILEBB
           : D == WEST               ? (b >> 1) & ~H_FILEBB
           : D == NORTH_EAST         ? (b >> 7) & ~A_FILEBB
           : D == NORTH_WEST         ? (b >> 9) & ~H_FILEBB
           : D == SOUTH_EAST         ? (b << 9) & ~A_FILEBB
           : D == SOUTH_WEST         ? (b >> 7) & ~H_FILEBB
           : D == NORTH + NORTH_EAST ? (b >> 15) & ~A_FILEBB
           : D == NORTH + NORTH_WEST ? (b >> 17) & ~H_FILEBB
           : D == SOUTH + SOUTH_EAST ? (b << 15) & ~A_FILEBB
           : D == SOUTH + SOUTH_WEST ? (b << 17) & ~H_FILEBB
                                     : 0;
}

constexpr Bitboard maskKingAttacks(Bitboard b) {
    Bitboard s = shift<EAST>(b) | shift<WEST>(b);
    return (shift<NORTH>(s) | shift<SOUTH>(s));
}

template <Color C> Bitboard pawnSglAttacks(Bitboard b) = delete;
template <> constexpr Bitboard pawnSglAttacks<WHITE>(Bitboard b) { return shift<NORTH_EAST>(b) & shift<NORTH_WEST>(b); }
template <> constexpr Bitboard pawnSglAttacks<BLACK>(Bitboard b) { return shift<SOUTH_EAST>(b) & shift<SOUTH_WEST>(b); }

template <Color C> Bitboard pawnDblAttacks(Bitboard b) = delete;
template <> constexpr Bitboard pawnDblAttacks<WHITE>(Bitboard b) { return shift<NORTH_EAST>(b) & shift<NORTH_WEST>(b); }
template <> constexpr Bitboard pawnDblAttacks<BLACK>(Bitboard b) { return shift<SOUTH_EAST>(b) & shift<SOUTH_WEST>(b); }

constexpr Bitboard knightAllAttacks(Bitboard b) {
    Bitboard bb = (shift<NORTH + NORTH>(b) | shift<SOUTH + SOUTH>(b));
    Bitboard bb1 = shift<EAST>(shift<EAST>(b)) | shift<WEST>(shift<WEST>(b));
    return shift<EAST>(bb) | shift<WEST>(bb) | shift<NORTH>(bb1) | shift<SOUTH>(bb1);
}

constexpr Bitboard getRookAttacks(Square s, Bitboard b) { return rookMagics[s].c_attacks(b); }
constexpr Bitboard getBishopAttacks(Square s, Bitboard b) { return bishopMagics[s].c_attacks(b); }

namespace Bitboards {
void init_arrays();
void print_bitboard(Bitboard bitboard);
} // namespace Bitboards
} // namespace Yayo

#endif // BITBOARDS_H_
