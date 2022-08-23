#include "bitboard.h"
#include "util.h"
#include <array>
#include <iostream>
#include <stdio.h>

namespace Yayo {
Bitboard bishopAttacks[B_ATK_TBL_SIZE];
Bitboard rookAttacks[R_ATK_TBL_SIZE];
Bitboard LINE[SQUARE_CT][SQUARE_CT];

Magic bishopMagics[SQUARE_CT];
Magic rookMagics[SQUARE_CT];

namespace {
constexpr Bitboard maskRookOccupancy(Square sq);
constexpr Bitboard maskBishopOccupancy(Square sq);
constexpr Bitboard genRookAttacks(Square square, Bitboard bb);
constexpr Bitboard genBishopAttacks(Square square, Bitboard bb);
} // namespace

void initMasks(const PieceT pc, Bitboard attack_table[], Magic mask[]) {
    int size = 0;
    for (int sq = 0; sq < 64; sq++) {
        Magic *magic = &mask[sq];
        magic->mask = (pc == ROOK) ? maskRookOccupancy(Square(sq)) : maskBishopOccupancy(Square(sq));
        magic->attacks = (sq == SQUARE_0) ? attack_table : (mask[sq - 1].attacks + size);

        Bitboard b = size = 0;
        do {
            size++;
            magic->attacks[magic->index(b)] =
                (pc == ROOK) ? genRookAttacks(Square(sq), b) : genBishopAttacks(Square(sq), b);
            b = (b - magic->mask) & magic->mask;
        } while (b);
    }
}

void Bitboards::init_arrays() {
    initMasks(ROOK, rookAttacks, rookMagics);
    initMasks(BISHOP, bishopAttacks, bishopMagics);

    for (int i = 0; i < 64; i++) {
        for (int to = 0; to < 64; to++) {
            Bitboard rookAtk = genRookAttacks(Square(i), 0);
            Bitboard bishAtk = genBishopAttacks(Square(i), 0);

            if (rookAtk & SQUARE_BB(Square(to))) {
                LINE[i][to] = (rookAtk & genRookAttacks(Square(to), SQUARE_BB(Square(i)))) | SQUARE_BB(Square(i)) |
                              SQUARE_BB(Square(to));
            }

            if (bishAtk & SQUARE_BB(Square(to))) {
                LINE[i][to] = (bishAtk & genBishopAttacks(Square(to), SQUARE_BB(Square(i)))) | SQUARE_BB(Square(i)) |
                              SQUARE_BB(Square(to));
            }
        }
    }
}

void Bitboards::print_bitboard(Bitboard bitboard) {
    for (int rank = 0; rank < 8; rank++) {
        // print all the file letters before the first rank
        if (rank == 0) {
            printf("  ");
            for (int i = 0; i < 8; i++)
                printf(" %2c ", 'A' + i);
            printf("\n");
        }

        printf("%2d ", 8 - rank);

        // print the status of the bit at each square
        for (int file = 0; file < 8; file++) {
            int s = rank * 8 + file;
            printf(" %-2c ", GET(bitboard, s) ? '1' : '.');
        }
        printf("%2d ", 8 - rank);
        printf("\n");
    }

    printf("  ");
    for (int i = 0; i < 8; i++)
        printf(" %2c ", 'A' + i);
    printf("\n\n");

    printf("\n");
}

namespace {

constexpr Bitboard maskRookOccupancy(Square sq) {
    Bitboard occ = 0;
    int file, rank;
    int sF = sq % 8, sR = sq / 8;

    for (rank = sR + 1; rank <= 6; rank++)
        occ |= SET(occ, (8 * rank + sF));
    for (rank = sR - 1; rank >= 1; rank--)
        occ |= SET(occ, (8 * rank + sF));
    for (file = sF + 1; file <= 6; file++)
        occ |= SET(occ, (8 * sR + file));
    for (file = sF - 1; file >= 1; file--)
        occ |= SET(occ, (8 * sR + file));

    return occ;
}

constexpr Bitboard maskBishopOccupancy(Square sq) {
    Bitboard occ = 0;
    int file, rank;
    int sF = sq % 8, sR = sq / 8;

    for (file = sF + 1, rank = sR + 1; file <= 6 && rank <= 6; file++, rank++)
        occ |= SET(occ, (8 * rank + file));
    for (file = sF - 1, rank = sR + 1; file >= 1 && rank <= 6; file--, rank++)
        occ |= SET(occ, (8 * rank + file));
    for (file = sF - 1, rank = sR - 1; file >= 1 && rank >= 1; file--, rank--)
        occ |= SET(occ, (8 * rank + file));
    for (file = sF + 1, rank = sR - 1; file <= 6 && rank >= 1; file++, rank--)
        occ |= SET(occ, (8 * rank + file));

    return occ;
}

constexpr Bitboard genRookAttacks(Square sq, Bitboard blocks) {
    Bitboard occ = 0;
    int file, rank;
    int sF = int(sq) % 8, sR = int(sq) / 8;

    for (rank = sR + 1; rank <= 7; rank++) {
        int s = 8 * rank + sF;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (rank = sR - 1; rank >= 0; rank--) {
        int s = 8 * rank + sF;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (file = sF + 1; file <= 7; file++) {
        int s = 8 * sR + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (file = sF - 1; file >= 0; file--) {
        int s = 8 * sR + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    return occ;
}

constexpr Bitboard genBishopAttacks(Square sq, Bitboard blocks) {
    Bitboard occ = 0;
    int file, rank;
    int sF = sq % 8, sR = sq / 8;

    for (file = sF + 1, rank = sR + 1; file <= 7 && rank <= 7; file++, rank++) {
        int s = 8 * rank + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (file = sF - 1, rank = sR + 1; file >= 0 && rank <= 7; file--, rank++) {
        int s = 8 * rank + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (file = sF - 1, rank = sR - 1; file >= 0 && rank >= 0; file--, rank--) {
        int s = 8 * rank + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    for (file = sF + 1, rank = sR - 1; file <= 7 && rank >= 0; file++, rank--) {
        int s = 8 * rank + file;
        occ |= SET(occ, s);
        if (SQUARE_BB(Square(s)) & blocks)
            break;
    }

    return occ;
}
} // namespace

} // namespace Yayo
