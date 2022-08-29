#ifndef BOARD_H_
#define BOARD_H_
#include "bitboard.h"
#include "move.h"
#include "util.h"
#include "zobrist.h"
#include <cassert>
#include <cstdint>
#include <string>

namespace Yayo {

struct Hist {
    uint64_t key;
    Bitboard checkPcs;
    Piece lastCapt;
    int castleStatus;
    Square enPass;
    int halfMoves, fullMoves;

    Hist() {
        key = 0;
        checkPcs = 0;
        lastCapt = NO_PC;
        castleStatus = 0;
        enPass = SQUARE_64;
        halfMoves = 0;
        fullMoves = 0;
    }
};

struct Info {
    Info() {
        startTime = -1;
        stopTime = -1;
        timeControl = -1;
        maxTimeControl = -1;
        depth = -1;
        selDepth = -1;
        timeGiven = false;
        movestogo = -1;
        nodes = 0;
        uciQuit = false;
        uciStop = false;
    }
    long double startTime, stopTime;
    long double timeControl, maxTimeControl;
    int depth;
    int selDepth;
    bool timeGiven;
    int movestogo;
    unsigned long long nodes;

    bool uciQuit, uciStop;
};

enum CastleRights : int {
    WHITE_KING = 1 << 3,
    WHITE_QUEEN = 1 << 2,
    BLACK_KING = 1 << 1,
    BLACK_QUEEN = 1,
    KING_SIDE = (1 << 3) | (1 << 1),
    QUEEN_SIDE = 1 | (1 << 2),
    NO_CASTLE_W = -1,
    NO_CASTLE_B = -1,
    NO_CASTLE = 0,
};

static const int castleMod[64] = {
    14, 15, 15, 15, 12, 15, 15, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 11, 15, 15, 15, 3,  15, 15, 7,
};

class Board {
  public:
    Hist hist[1000];
    Piece board[64];
    Bitboard color[2];
    Bitboard pieceBB[PC_MAX];
    Bitboard cPieceBB[7];

    uint64_t key;
    Bitboard checkPcs;

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

    Board(Board &other) {
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

    inline bool operator==(Board &b1) {
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

    void print();
    void setFen(const std::string fen);
    std::string fen();

    Bitboard pieces() { return this->color[WHITE] | this->color[BLACK]; }
    Bitboard pieces(PieceT p, Color c) { return this->color[c] & this->cPieceBB[p]; };
    Bitboard pieces(Color c) { return color[c]; }
    Bitboard pieces(PieceT p) { return cPieceBB[p]; }

    Color side() { return turn; }

    inline Bitboard bCheckPcs() { return checkPcs; };

    inline Square epSq() { return enPass; };

    inline Bitboard between(Square s1, Square s2) { return rectArray[s1][s2]; };

    CastleRights canCastle(Color c) {
        int kingSide = castleRights & KING_SIDE;
        int queenSide = castleRights & QUEEN_SIDE;

        return CastleRights((c == WHITE) ? ((kingSide & 8) | (queenSide & 4)) : ((kingSide & 2) | (queenSide & 1)));
    };

    bool castleBlocked(CastleRights cr, Square sq) {
        Bitboard queenOccupancy = (7 << (int(sq) - 3));
        Bitboard kingOccupancy = (3 << (int(sq) + 1));

        if (checkPcs)
            return true;
        if (cr & QUEEN_SIDE)
            return !(queenOccupancy & pieces());
        if (cr & KING_SIDE)
            return !(kingOccupancy & pieces());
        return false;
    };

    template <Color C> Bitboard attacksToKing(Square sq, Bitboard occ) {
        Bitboard knights, kings, queenRooks, queenBishops;
        knights = pieces(KNIGHT, C);
        kings = pieces(KING, C);
        queenRooks = queenBishops = pieces(QUEEN, C);
        queenRooks |= pieces(ROOK, C);
        queenBishops |= pieces(BISHOP, C);

        assert(sq < 64 && sq >= 0);

        return (pawnAttacks[~C][sq] & pieces(PAWN, C)) | (knightAttacks[sq] & knights) | (kingAttacks[sq] & kings) |
               (getBishopAttacks(sq, occ) & queenBishops) | (getRookAttacks(sq, occ) & queenRooks);
    };

    bool isSqAttacked(Square sq, Bitboard occ, Color byColor) {
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
        rookQueens = pieces(QUEEN, byColor);
        bishopQueens = rookQueens | pieces(BISHOP, byColor);
        if (getBishopAttacks(sq, occ) & bishopQueens)
            return true;
        rookQueens |= pieces(ROOK, byColor);
        if (getRookAttacks(sq, occ) & rookQueens)
            return true;

        return false;
    }

    constexpr bool isDraw() {
        const int num = __builtin_popcountll(color[0] | color[1]);
        const bool kvk = num == 2;
        const bool kvbk = (num == 3) && (cPieceBB[BISHOP]);
        const bool kvnk = (num == 3) && (cPieceBB[KNIGHT]);
        const bool kvnnk = (num == 4) && (__builtin_popcountll(cPieceBB[KNIGHT]) == 2);

        return kvk || kvbk || kvnk || kvnnk;
    }
};
} // namespace Yayo
#endif // BOARD_H_
