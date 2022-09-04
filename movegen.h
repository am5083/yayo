#ifndef MOVEGEN_H_
#define MOVEGEN_H_
#include "bitboard.h"
#include "board.h"
#include "move.h"
#include "tt.h"
#include "util.h"
#include <cassert>
#include <thread>

namespace Yayo {

// MoveType
// M_CAPTURE
// M_QUIET
// CHECK_EVASION
// CAP_QUIET
// LEGAL

constexpr int victimScores[7] = {0, 100, 200, 300, 400, 500, 600};
static int mvvLvaTable[7][7];

void initMvvLva() {
    for (int Atk = 0; Atk < 7; Atk++) {
        for (int Vic = 0; Vic < 7; Vic++) {
            mvvLvaTable[Vic][Atk] = victimScores[Vic] + 6 - (victimScores[Atk] / 100);
        }
    }
}

void printMvvLvaTable() {
    for (int Atk = 0; Atk < 7; Atk++) {
        for (int Vic = 0; Vic < 7; Vic++) {
            std::cout << mvvLvaTable[Vic][Atk] << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template <Color Turn, MoveType Type>
constexpr moveList *generatePawnMoves(const Board &board, moveList *mList, const Bitboard targets) {
    constexpr Color enemy       = ~Turn;
    constexpr Bitboard Rank7    = (Turn == WHITE) ? RANK_7BB : RANK_2BB;
    constexpr Bitboard Rank3    = (Turn == WHITE) ? RANK_3BB : RANK_6BB;
    constexpr Direction Push    = (Turn == WHITE) ? NORTH : SOUTH;
    constexpr Direction eAttack = Push + EAST;
    constexpr Direction wAttack = Push + WEST;

    Bitboard capture = board.pieces(enemy);
    Bitboard push    = ~board.pieces();

    if (board.checkPcs) {
        push    = targets & ~board.pieces();
        capture = targets & board.checkPcs;
    }

    //(Type == CHECK_EVASION) ? board.bCheckPcs() : (Type == M_QUIET) ? 0 : board.pieces(enemy);
    Bitboard singlePushPawns = board.pieces(PAWN, Turn) & ~Rank7;
    Bitboard promoPawns      = board.pieces(PAWN, Turn) & Rank7;

    if (Type != M_CAPTURE) {
        Bitboard singlePush = shift<Push>(singlePushPawns) & ~board.pieces();
        Bitboard doublePush = shift<Push>(singlePush & Rank3) & push;
        singlePush &= push;

        if (Type == CHECK_EVASION) {
            singlePush &= targets;
            doublePush &= targets;
        }

        while (singlePush) {
            const Square s = Square(__builtin_ctzll(singlePush));
            singlePush &= singlePush - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, QUIET), false, false);
        }

        while (doublePush) {
            const Square s = Square(__builtin_ctzll(doublePush));
            doublePush &= doublePush - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push) + int(Push)), s, DOUBLE_PAWN), false, false);
        }
    }

    if (promoPawns) {
        Bitboard ePromoCapture = shift<eAttack>(promoPawns) & capture;
        Bitboard wPromoCapture = shift<wAttack>(promoPawns) & capture;
        Bitboard pushPromo     = shift<Push>(promoPawns) & push;

        if (Type == CHECK_EVASION) {
            pushPromo &= targets;
        }

        while (ePromoCapture) {
            const Square s = Square(__builtin_ctzll(ePromoCapture));
            ePromoCapture &= ePromoCapture - 1;

            Square fromSq = Square(int(s) + int(wAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_ROOK), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_BISHOP), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_KNIGHT), true, true, score);
        }

        while (wPromoCapture) {
            const Square s = Square(__builtin_ctzll(wPromoCapture));
            wPromoCapture &= wPromoCapture - 1;

            Square fromSq = Square(int(s) + int(eAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
            mList->addMove(encodeMove(fromSq, s, CP_QUEEN), true, true, score);
        }

        while (pushPromo) {
            const Square s = Square(__builtin_ctzll(pushPromo));
            pushPromo &= pushPromo - 1;
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_QUEEN), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_ROOK), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_BISHOP), true, false);
            mList->addMove(encodeMove(Square(int(s) + int(Push)), s, P_KNIGHT), true, false);
        }
    }

    if (Type == M_CAPTURE || Type == CHECK_EVASION || Type == CAP_QUIET) {

        Bitboard eastAttacks =
            (eAttack == NORTH + EAST ? shift<NORTH + EAST>(singlePushPawns) : shift<SOUTH + EAST>(singlePushPawns)) &
            capture;
        Bitboard westAttacks =
            (wAttack == NORTH + WEST ? shift<NORTH + WEST>(singlePushPawns) : shift<SOUTH + WEST>(singlePushPawns)) &
            capture;

        while (eastAttacks) {
            const Square s = Square(__builtin_ctzll(eastAttacks));
            eastAttacks &= eastAttacks - 1;

            Square fromSq = Square(int(s) + int(wAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(fromSq, s, CAPTURE), false, true, score);
        }

        while (westAttacks) {
            const Square s = Square(__builtin_ctzll(westAttacks));
            westAttacks &= westAttacks - 1;

            Square fromSq = Square(int(s) + int(eAttack));
            PieceT fromPc = getPcType(board.board[fromSq]), toPc = getPcType(board.board[s]);

            int score = mvvLvaTable[toPc][fromPc] + 1000000;
            mList->addMove(encodeMove(Square(int(s) + int(eAttack)), s, CAPTURE), false, true, score);
        }

        if (board.epSq() != SQUARE_64) {
            if (Type == CHECK_EVASION && (targets & (board.epSq() + Square(Push)))) {
                return mList;
            }

            eastAttacks = singlePushPawns & getPawnAttacks(enemy, board.epSq());
            while (eastAttacks) {
                const Square s      = Square(__builtin_ctzll(eastAttacks));
                const Bitboard pawn = SQUARE_BB(s);
                eastAttacks &= eastAttacks - 1;
                if (board.isSqAttacked(Sq(board.pieces(KING, Turn)), board.pieces() ^ pawn, ~Turn))
                    continue;
                mList->addMove(encodeMove(s, board.epSq(), EP_CAPTURE), false, true, 105);
            }
        }
    }
    return mList;
}

template <Color C, PieceT P, MoveType T>
constexpr moveList *generateMoves(const Board &board, moveList *mList, Bitboard targets) {
    Bitboard Turn = board.pieces(P, C);

    while (Turn) {
        const Square s = Square(__builtin_ctzll(Turn));
        Turn &= Turn - 1;
        Bitboard b = getAttacks<P>(s, board.pieces()) & targets & ~board.pieces(KING);
        while (b) {
            const Square t    = Square(__builtin_ctzll(b));
            const MoveFlag mf = (SQUARE_BB(t) & board.pieces(~C)) ? CAPTURE : QUIET;

            int score = 0;
            if (board.board[t] != NO_PC) {
                PieceT toPc = getPcType(board.board[t]);
                score       = mvvLvaTable[toPc][P] + 1000000;
            }
            mList->addMove(encodeMove(s, t, mf), false, bool(mf), score);
            b &= b - 1;
        }
    }
    return mList;
}
using namespace Yayo;
template <Color C, MoveType T> constexpr moveList *generateLegal(const Board &board, moveList *mList) {
    const Square kingSq = Square(__builtin_ctzll(board.pieces(KING, C)));

    Bitboard orth = getRookAttacks(kingSq, board.pieces(~C));
    Bitboard diag = getBishopAttacks(kingSq, board.pieces(~C));

    Bitboard pinners = (board.pieces(QUEEN, ~C) | board.pieces(ROOK, ~C)) & orth;
    pinners |= (board.pieces(QUEEN, ~C) | board.pieces(BISHOP, ~C)) & diag;
    Bitboard pinned = 0;
    while (pinners) {
        const Square sq = Square(__builtin_ctzll(pinners));
        Bitboard pp     = rectArray[sq][kingSq] & board.pieces(C);
        if (__builtin_popcountll(pp) == 1)
            pinned ^= pp;

        pinners &= pinners - 1;
    }

    moveList temp    = {0};
    Bitboard targets = 0;
    if (T != CHECK_EVASION || __builtin_popcountll(board.bCheckPcs()) < 1) {
        switch (T) {
        case CHECK_EVASION:
            targets = board.between(kingSq, Square(__builtin_ctzll(board.bCheckPcs())));
            break;
        case M_CAPTURE:
            targets = board.pieces(~C);
            break;
        case CAP_QUIET:
            targets = ~board.pieces(C);
            break;
        default:
            targets = 0;
            break;
        }

        if (__builtin_popcountll(board.bCheckPcs()) == 1) {
            targets = LINE[kingSq][__builtin_ctzll(board.checkPcs)] & ~board.pieces(C) & (orth | diag);
            targets |= board.checkPcs;
        } else if (__builtin_popcountll(board.bCheckPcs()) > 1) {
            targets = 0;
        }

        generatePawnMoves<C, T>(board, &temp, targets);
        generateMoves<C, KNIGHT, T>(board, &temp, targets);
        generateMoves<C, BISHOP, T>(board, &temp, targets);
        generateMoves<C, ROOK, T>(board, &temp, targets);
        generateMoves<C, QUEEN, T>(board, &temp, targets);
    }

    Bitboard b = kingAttacks[kingSq] & ~board.pieces(C);
    if (__builtin_popcountll(board.bCheckPcs()) == 1 && !(board.checkPcs & board.pieces(PAWN, ~C))) {
        Bitboard pinmask = LINE[__builtin_ctzll(board.checkPcs)][kingSq];
        b &= ~(pinmask & ~board.checkPcs);
    }

    if (T == M_CAPTURE) {
        b &= targets;
    }

    while (b) {
        const Square s = Square(__builtin_ctzll(b));
        b &= b - 1;
        if (board.isSqAttacked(s, board.pieces() ^ board.pieces(KING, C), ~C)) {
            continue;
        }

        const MoveFlag mf = (SQUARE_BB(s) & board.pieces(~C)) ? CAPTURE : QUIET;

        int score = 0;
        if (board.board[s] != NO_PC) {
            PieceT toPc = getPcType(board.board[s]);
            score       = mvvLvaTable[toPc][KING] + 1000000;
        }

        temp.addMove(encodeMove(kingSq, s, mf), false, bool(mf), score);
    }

    if ((T == M_QUIET || T == CAP_QUIET) && board.canCastle(C)) {
        CastleRights cr    = board.canCastle(C);
        const bool inCheck = board.bCheckPcs();
        if (cr & KING_SIDE && !inCheck) {
            Bitboard betweenMask = (3ULL << (kingSq + 1));
            if (!(betweenMask & board.pieces())) {
                Square to     = SQUARE_64;
                bool attacked = false;
                while (betweenMask && !attacked) {
                    to = Square(__builtin_ctzll(betweenMask));
                    betweenMask &= betweenMask - 1;
                    attacked = attacked || board.attacksToKing<~C>(to, board.pieces());
                }

                if (!attacked) {
                    temp.addMove(encodeMove(kingSq, Square(kingSq + 2), K_CASTLE), false, false);
                }
            }
        }

        if (cr & QUEEN_SIDE && !inCheck) {
            Bitboard betweenMask = (3ULL << (kingSq - 2));
            if (!((7ULL << (kingSq - 3)) & board.pieces())) {
                Square to     = SQUARE_64;
                bool attacked = false;
                while (betweenMask && !attacked) {
                    to = Square(__builtin_ctzll(betweenMask));
                    betweenMask &= betweenMask - 1;
                    attacked = attacked || board.attacksToKing<~C>(to, board.pieces());
                }

                if (!attacked) {
                    temp.addMove(encodeMove(kingSq, Square(kingSq - 2), Q_CASTLE), false, false);
                }
            }
        }
    }

    int j = 0;
    for (int i = 0; i < temp.nMoves; i++) {
        Square fromSq    = getFrom(temp.moves[i]);
        Bitboard fromMsk = SQUARE_BB(fromSq);
        if (pinned && (pinned & fromMsk)) {
            if (LINE[kingSq][fromSq] & SQUARE_BB(getTo(temp.moves[i]))) {
                mList->moves[j] = temp.moves[i];
                mList->nMoves++;
                j++;
            }
        } else {
            mList->moves[j] = temp.moves[i];
            mList->nMoves++;
            j++;
        }
    }

    return mList;
}

constexpr moveList *generateCaptures(const Board &board, moveList *mList) {
    (board.turn == WHITE) ? generateLegal<WHITE, M_CAPTURE>(board, mList)
                          : generateLegal<BLACK, M_CAPTURE>(board, mList);
    return mList;
}

constexpr moveList *generate(const Board &board, moveList *mList) {
    (board.turn == WHITE) ? generateLegal<WHITE, CAP_QUIET>(board, mList)
                          : generateLegal<BLACK, CAP_QUIET>(board, mList);
    return mList;
}

void makeNullMove(Board &board) {
    int ply = board.gamePly;

    (board.hist)[ply].checkPcs     = board.checkPcs;
    (board.hist)[ply].lastCapt     = board.lastCapt;
    (board.hist)[ply].castleStatus = board.castleRights;
    (board.hist)[ply].enPass       = board.enPass;
    (board.hist)[ply].halfMoves    = board.halfMoves;
    (board.hist)[ply].fullMoves    = board.fullMoves;
    (board.hist)[ply].key          = board.key;

    board.checkPcs = 0;
    board.lastCapt = NO_PC;
    board.enPass   = SQUARE_64;

    board.key ^= zobristBlackToMove;
    board.key ^= (board.enPass < 64) ? zobristEpFile[board.enPass % 8] : 0;

    board.ply++;
    board.gamePly++;
    board.halfMoves++;
    board.fullMoves++;

    board.turn = ~board.turn;
}

void unmakeNullMove(Board &board) {
    board.turn = ~board.turn;
    board.ply--;
    board.gamePly--;

    int ply            = board.gamePly;
    board.checkPcs     = (board.hist)[ply].checkPcs;
    board.lastCapt     = (board.hist)[ply].lastCapt;
    board.castleRights = (board.hist)[ply].castleStatus;
    board.enPass       = (board.hist)[ply].enPass;
    board.halfMoves    = (board.hist)[ply].halfMoves;
    board.fullMoves    = (board.hist)[ply].fullMoves;
    board.key          = (board.hist)[ply].key;
}

void make(Board &board, unsigned short move) {
    Square fromSq = getFrom(move);
    Square toSq   = getTo(move);

    Piece fromPc = board.board[fromSq];
    Piece toPc   = board.board[toSq];

    board.hist[board.gamePly].checkPcs     = board.checkPcs;
    board.hist[board.gamePly].lastCapt     = board.lastCapt;
    board.hist[board.gamePly].castleStatus = board.castleRights;
    board.hist[board.gamePly].enPass       = board.enPass;
    board.hist[board.gamePly].halfMoves    = board.halfMoves;
    board.hist[board.gamePly].fullMoves    = board.fullMoves;
    board.hist[board.gamePly].key          = board.key;

    board.key ^= (board.enPass != SQUARE_64) ? zobristEpFile[FILE_OF(board.enPass)] : 0;
    board.halfMoves++;
    if (fromPc == W_PAWN || fromPc == B_PAWN)
        board.halfMoves = 0;

    board.lastCapt = NO_PC;
    board.enPass   = SQUARE_64;

    int fPcIdx = fromPc;
    // if (fromPc != NO_PC && fromPc >= B_PAWN)
    //     fPcIdx -= 2;

    int tPcIdx = toPc;
    // if (toPc != NO_PC && toPc >= B_PAWN)
    //     tPcIdx -= 2;

    switch (getCapture(move)) {
    case QUIET:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq]   = fromPc;
        board.lastCapt      = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];

        if (getPcType(fromPc) == KING || getPcType(fromPc) == ROOK) {
            assert(fromSq != SQUARE_64);
            board.castleRights &= castleMod[fromSq];
        }
        break;
    case DOUBLE_PAWN: {
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq]   = fromPc;
        board.lastCapt      = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];

        Bitboard canEp =
            (shift<EAST>(SQUARE_BB(toSq)) | shift<WEST>(SQUARE_BB(toSq))) & board.pieces(PAWN, ~board.turn);
        if (canEp) {
            board.enPass =
                (board.turn == WHITE) ? Sq(shift<NORTH>(SQUARE_BB(fromSq))) : Sq(shift<SOUTH>(SQUARE_BB(fromSq)));
            board.key ^= zobristEpFile[FILE_OF(board.enPass)];
        }
    } break;
    case CAPTURE:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq]   = fromPc;
        board.lastCapt      = toPc;

        board.halfMoves = 0;
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[toPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[getPcType(fromPc)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(toPc)] ^= SQUARE_BB(toSq);

        assert(fPcIdx != NO_PC);
        assert(tPcIdx != NO_PC);
        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[tPcIdx][toSq];

        if (getPcType(toPc) == ROOK) {
            board.castleRights &= castleMod[toSq];
        }

        if (getPcType(fromPc) == KING || getPcType(fromPc) == ROOK) {
            board.castleRights &= castleMod[fromSq];
        }

        break;
    case EP_CAPTURE: {
        Bitboard captured = board.turn == WHITE ? shift<SOUTH>(toSq) : shift<NORTH>(toSq);
        Piece cPiece      = board.board[Sq(captured)];

        tPcIdx = cPiece;

        board.halfMoves = 0;
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq]   = fromPc;
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.color[~board.turn] ^= captured;
        board.pieceBB[cPiece] ^= captured;
        board.board[Sq(captured)] = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(cPiece)] ^= captured;

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[tPcIdx][Sq(captured)];
    } break;
    case K_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? H1 : H8;
        rTo   = (board.turn == WHITE) ? F1 : F8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[fromSq] = NO_PC;
        board.board[rFrom]  = NO_PC;

        board.board[toSq] = fromPc;
        board.board[rTo]  = rPiece;

        board.lastCapt = NO_PC;
        board.castleRights &= castleMod[fromSq];

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[rPiece][rFrom] ^ zobristPieceSq[rPiece][rTo];
    } break;
    case Q_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? A1 : A8;
        rTo   = (board.turn == WHITE) ? D1 : D8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[fromSq] = NO_PC;
        board.board[rFrom]  = NO_PC;

        board.board[toSq] = fromPc;
        board.board[rTo]  = rPiece;

        board.lastCapt = NO_PC;
        board.castleRights &= castleMod[fromSq];

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[rPiece][rFrom] ^ zobristPieceSq[rPiece][rTo];
    } break;
    case P_KNIGHT:
    case P_BISHOP:
    case P_ROOK:
    case P_QUEEN: {
        int pTo       = getCapture(move) - P_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + pTo + (8 * board.turn));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);
        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);

        board.board[fromSq] = NO_PC;
        board.board[toSq]   = promoPc;
        board.lastCapt      = NO_PC;

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[promoPc][toSq];
    } break;
    case CP_KNIGHT:
    case CP_BISHOP:
    case CP_ROOK:
    case CP_QUEEN: {
        int pTo       = getCapture(move) - CP_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + (pTo + (8 * board.turn)));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);
        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);
        board.cPieceBB[getPcType(toPc)] ^= SQUARE_BB(toSq);

        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[toPc] ^= SQUARE_BB(toSq);

        board.board[fromSq] = NO_PC;
        board.board[toSq]   = promoPc;
        board.lastCapt      = toPc;

        if (getPcType(toPc) == ROOK)
            board.castleRights &= castleMod[toSq];

        board.key ^= zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[promoPc][toSq];
        board.key ^= zobristPieceSq[tPcIdx][toSq];
    } break;
    }

    assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);

    int temp = board.castleRights ^ board.hist[board.ply].castleStatus;
    board.key ^= zobristCastleRights[temp];

    board.turn = ~board.turn;
    board.key ^= (zobristBlackToMove * int(board.turn));
    board.checkPcs =
        board.turn == WHITE
            ? board.attacksToKing<BLACK>(Sq(board.pieces(KING, board.turn)), board.color[0] | board.color[1])
            : board.attacksToKing<WHITE>(Sq(board.pieces(KING, board.turn)), board.color[0] | board.color[1]);

    board.ply++;
    board.gamePly++;

    if (board.turn == WHITE)
        board.fullMoves++;
}

void unmake(Board &board, unsigned short move) {
    int copy   = 1;
    board.turn = ~board.turn;
    board.ply--;
    board.gamePly--;

    int ply            = board.gamePly;
    board.checkPcs     = board.hist[ply].checkPcs;
    board.castleRights = board.hist[ply].castleStatus;
    board.enPass       = board.hist[ply].enPass;
    board.halfMoves    = board.hist[ply].halfMoves;
    board.fullMoves    = board.hist[ply].fullMoves;
    board.key          = board.hist[ply].key;

    Square fromSq = getFrom(move);
    Square toSq   = getTo(move);

    Piece piece    = board.board[toSq];
    Piece capPiece = board.lastCapt;

    switch (getCapture(move)) {
    case QUIET:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq]   = NO_PC;
        assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);
        break;
    case DOUBLE_PAWN:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq]   = NO_PC;
        assert(copy == 1);
        break;
    case CAPTURE:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq]   = capPiece;
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[capPiece] ^= SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(capPiece)] ^= SQUARE_BB(toSq);
        break;
    case EP_CAPTURE: {
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq]   = NO_PC;

        Bitboard captured = board.turn == WHITE ? shift<SOUTH>(toSq) : shift<NORTH>(toSq);
        Piece cPiece      = Piece(W_PAWN + (8 * (~board.turn)));
        board.color[~board.turn] ^= captured;
        board.pieceBB[cPiece] ^= captured;
        board.board[Sq(captured)] = cPiece;
        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[PAWN] ^=
            (board.turn == WHITE) ? shift<SOUTH>(SQUARE_BB(board.enPass)) : shift<NORTH>(SQUARE_BB(board.enPass));
    } break;
    case K_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? H1 : H8;
        rTo   = (board.turn == WHITE) ? F1 : F8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[toSq]   = NO_PC;
        board.board[rTo]    = NO_PC;
        board.board[fromSq] = piece;
        board.board[rFrom]  = rPiece;
        assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);
    } break;
    case Q_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? A1 : A8;
        rTo   = (board.turn == WHITE) ? D1 : D8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[toSq]   = NO_PC;
        board.board[rTo]    = NO_PC;
        board.board[fromSq] = piece;
        board.board[rFrom]  = rPiece;
        assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);
    } break;
    case P_KNIGHT:
    case P_BISHOP:
    case P_ROOK:
    case P_QUEEN: {
        int pTo       = getCapture(move) - P_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + (pTo + (8 * board.turn)));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.pieceBB[W_PAWN + (8 * board.turn)] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);

        board.board[fromSq] = Piece(W_PAWN + (8 * board.turn));
        board.board[toSq]   = NO_PC;
        assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);
    } break;
    case CP_KNIGHT:
    case CP_BISHOP:
    case CP_ROOK:
    case CP_QUEEN: {
        Board c2(board);
        int pTo       = getCapture(move) - CP_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + (pTo + (8 * board.turn)));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.pieceBB[W_PAWN + (8 * board.turn)] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);
        board.cPieceBB[getPcType(capPiece)] ^= SQUARE_BB(toSq);
        board.board[fromSq] = Piece(W_PAWN + (8 * board.turn));
        board.board[toSq]   = capPiece;

        if (!(board.turn == 0 || board.turn == 1)) {
            std::cout << "TURN: " << board.turn << std::endl;
        }
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[capPiece] ^= SQUARE_BB(toSq);
        assert(__builtin_popcountll(board.cPieceBB[KING]) == 2);
    } break;
    }

    board.lastCapt = board.hist[ply].lastCapt;
}

} // namespace Yayo

#endif // MOVEGEN_H_
