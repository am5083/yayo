#ifndef BOARD_H_
#define BOARD_H_
#include "bitboard.hpp"
#include "move.hpp"
#include "util.hpp"
#include <cassert>
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

    Board() {
        key = 0;
        checkPcs = 0;
        lastCapt = NO_PC;
        turn = WHITE;
        ply = 0;
        gamePly = 0;
        halfMoves = 0;
        fullMoves = 0;
        enPass = SQUARE_64;
        castleRights = 0;
        color[WHITE] = 0;
        color[BLACK] = 0;

        for (int i = 0; i < 13; i++)
            pieceBB[i] = 0;
        for (int i = 0; i < 7; i++)
            cPieceBB[i] = 0;
        for (int i = 0; i < 64; i++)
            board[i] = NO_PC;
    }

    Board(const Board &other) {
        for (int i = 0; hist[i].key; i++) {
            hist[i].castleStatus = other.hist[i].castleStatus;
            hist[i].checkPcs = other.hist[i].checkPcs;
            hist[i].enPass = other.hist[i].enPass;
            hist[i].fullMoves = other.hist[i].fullMoves;
            hist[i].halfMoves = other.hist[i].halfMoves;
            hist[i].key = other.hist[i].key;
            hist[i].lastCapt = other.hist[i].lastCapt;
        }

        for (int i = 0; i < 64; i++) {
            board[i] = other.board[i];
        }

        color[0] = other.color[0];
        color[1] = other.color[1];

        for (int i = 0; i < PC_MAX; i++) {
            pieceBB[i] = other.pieceBB[i];
        }

        for (int i = 0; i < 7; i++) {
            cPieceBB[i] = other.cPieceBB[i];
        }

        key = other.key;
        checkPcs = other.checkPcs;
        lastCapt = other.lastCapt;
        turn = other.turn;
        ply = other.ply;
        gamePly = other.gamePly;
        halfMoves = other.halfMoves;
        fullMoves = other.fullMoves;
        enPass = other.enPass;
        castleRights = other.castleRights;
    }

    constexpr bool operator==(const Board &b1) const {
        bool equal = true;
        for (int i = 0; hist[i].key; i++) {
            equal = equal && (hist[i].castleStatus == b1.hist[i].castleStatus);
            equal = equal && (hist[i].checkPcs == b1.hist[i].checkPcs);
            equal = equal && (hist[i].enPass == b1.hist[i].enPass);
            equal = equal && (hist[i].fullMoves == b1.hist[i].fullMoves);
            equal = equal && (hist[i].halfMoves == b1.hist[i].halfMoves);
            equal = equal && (hist[i].key == b1.hist[i].key);
            equal = equal && (hist[i].lastCapt == b1.hist[i].lastCapt);

            if (!equal) {
                std::cout << "ERROR! HIST ARRAY"
                          << "\n";
                return false;
            }
        }

        for (int i = 0; i < 64; i++) {
            equal = equal && (board[i] == b1.board[i]);

            if (!equal) {
                print();
                b1.print();
                std::cout << "INDEX: " << i << std::endl;
                std::cout << "BOARD1: " << board[i] << std::endl;
                std::cout << "BOARD2: " << b1.board[i] << std::endl;
                std::cout << "ERROR! BOARD ARRAY"
                          << "\n";
                return false;
            }
        }

        equal = equal && (color[0] == b1.color[0]);
        equal = equal && (color[1] == b1.color[1]);

        if (!equal) {
            std::cout << "ERROR! COLOR ARRAY"
                      << "\n";
            return false;
        }

        for (int i = 0; i < PC_MAX; i++) {
            equal = equal && (pieceBB[i] == b1.pieceBB[i]);
        }

        if (!equal) {
            std::cout << "ERROR! PIECEBB ARRAY"
                      << "\n";
            return false;
        }
        for (int i = 0; i < 7; i++) {
            equal = equal && (cPieceBB[i] == b1.cPieceBB[i]);

            if (!equal) {
                std::cout << "ERROR! CPIECEBB ARRAY"
                          << "; INDEX: " << i << "\n";
                Bitboards::print_bitboard(cPieceBB[1]);
                Bitboards::print_bitboard(b1.cPieceBB[1]);
                print();
                b1.print();
                return false;
            }
        }

        return equal;

        equal = equal && (key == b1.key);
        equal = equal && (checkPcs == b1.checkPcs);
        equal = equal && (lastCapt == b1.lastCapt);
        equal = equal && (turn == b1.turn);
        equal = equal && (ply == b1.ply);
        equal = equal && (gamePly == b1.gamePly);
        equal = equal && (halfMoves == b1.halfMoves);
        equal = equal && (fullMoves == b1.fullMoves);
        equal = equal && (enPass == b1.enPass);
        equal = equal && (castleRights == b1.castleRights);

        if (!equal) {
            std::cout << "ERROR! VARIABLES"
                      << "\n";
            return false;
        }
        return equal;
    }

    constexpr int numRepetition() const ;
    constexpr bool isRepetition() const;

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
    constexpr int see(Square toSq, Piece toPc, Square from, Piece fromPc);
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

        assert(sq < 64 && sq >= 0);

        return (pawnAttacks[~C][sq] & pieces(PAWN, C)) | (knightAttacks[sq] & knights) | (kingAttacks[sq] & kings) |
               (getBishopAttacks(sq, occ) & queenBishops) | (getRookAttacks(sq, occ) & queenRooks);
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

constexpr int Board::numRepetition() const {
    int num_rep = 0;
    for (int i = 0; i < gamePly; i++) {
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

constexpr int Board::see(Square toSq, Piece toPc, Square from, Piece fromPc) {
    int gain[32];
    int ply = 0;

    Bitboard occ       = pieces();
    Bitboard xRayPcs   = pieces(PAWN) | pieces(BISHOP) | pieces(ROOK) | pieces(QUEEN);
    Bitboard fromMap   = SQUARE_BB(from);
    Bitboard atkDefMap = (turn == WHITE) ? attacksToKing<BLACK>(toSq, pieces()) : attacksToKing<WHITE>(toSq, pieces());

    assert(toPc <= B_KING);
    assert(getPcType(toPc) >= 0);
    assert(getPcType(toPc) < 8);

    int pcVal[] = {0, PAWN_VAL, KNIGHT_VAL, BISHOP_VAL, ROOK_VAL, QUEEN_VAL, KING_VAL, 0};
    gain[ply]   = pcVal[getPcType(toPc)];

    do {
        ply++;

        gain[ply] = pcVal[getPcType(fromPc)] - gain[ply - 1];
        if (std::max(-gain[ply - 1], ply) < 0)
            break;

        atkDefMap ^= fromMap;
        occ ^= fromMap;

        if (fromMap & xRayPcs) {
            xRayPcs ^= fromMap;
            atkDefMap |= xRayAtks(toSq, occ);
        }

        fromMap = getLVA(turn, atkDefMap, &fromPc);
    } while (fromMap && ply < 32);

    while (--ply && (ply - 1) >= 0) {
        gain[ply - 1] = -std::max(-gain[ply - 1], gain[ply]);
    }

    return gain[0];
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


} // namespace Yayo
#endif // BOARD_H_
