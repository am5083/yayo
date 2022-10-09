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

#include "movegen.hpp"

namespace Yayo {
void initMvvLva() {
    for (int Atk = 0; Atk < 7; Atk++) {
        for (int Vic = 0; Vic < 7; Vic++) {
            mvvLvaTable[Vic][Atk] =
                  victimScores[Vic] + 6 - (victimScores[Atk] / 100);
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

void makeNullMove(Board &board) {
    int ply = board.gamePly;

    (board.hist)[ply].checkPcs = board.checkPcs;
    (board.hist)[ply].lastCapt = board.lastCapt;
    (board.hist)[ply].castleStatus = board.castleRights;
    (board.hist)[ply].enPass = board.enPass;
    (board.hist)[ply].halfMoves = board.halfMoves;
    (board.hist)[ply].fullMoves = board.fullMoves;
    (board.hist)[ply].key = board.key;

    board.key ^= (board.enPass != SQUARE_64)
                       ? zobristEpFile[FILE_OF(board.enPass)]
                       : 0;
    board.key ^= 1;

    board.checkPcs = 0;
    board.lastCapt = NO_PC;
    board.enPass = SQUARE_64;

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

    int ply = board.gamePly;
    board.checkPcs = (board.hist)[ply].checkPcs;
    board.lastCapt = (board.hist)[ply].lastCapt;
    board.castleRights = (board.hist)[ply].castleStatus;
    board.enPass = (board.hist)[ply].enPass;
    board.halfMoves = (board.hist)[ply].halfMoves;
    board.fullMoves = (board.hist)[ply].fullMoves;
    board.key = (board.hist)[ply].key;
}

void make(Board &board, unsigned short move) {
    Square fromSq = getFrom(move);
    Square toSq = getTo(move);

    Piece fromPc = board.board[fromSq];
    Piece toPc = board.board[toSq];

    board.hist[board.gamePly].checkPcs = board.checkPcs;
    board.hist[board.gamePly].lastCapt = board.lastCapt;
    board.hist[board.gamePly].castleStatus = board.castleRights;
    board.hist[board.gamePly].enPass = board.enPass;
    board.hist[board.gamePly].halfMoves = board.halfMoves;
    board.hist[board.gamePly].fullMoves = board.fullMoves;
    board.hist[board.gamePly].key = board.key;

    board.key ^= (board.enPass != SQUARE_64)
                       ? zobristEpFile[FILE_OF(board.enPass)]
                       : 0;
    board.halfMoves++;
    if (fromPc == W_PAWN || fromPc == B_PAWN)
        board.halfMoves = 0;

    board.lastCapt = NO_PC;
    board.enPass = SQUARE_64;

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
        board.board[toSq] = fromPc;
        board.lastCapt = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^=
              SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];

        if (getPcType(fromPc) == KING || getPcType(fromPc) == ROOK) {
            board.castleRights &= castleMod[fromSq];
        }
        break;
    case DOUBLE_PAWN: {
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq] = fromPc;
        board.lastCapt = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^=
              SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];

        Bitboard canEp =
              (shift<EAST>(SQUARE_BB(toSq)) | shift<WEST>(SQUARE_BB(toSq))) &
              board.pieces(PAWN, ~board.turn);
        if (canEp) {
            board.enPass = (board.turn == WHITE)
                                 ? Sq(shift<NORTH>(SQUARE_BB(fromSq)))
                                 : Sq(shift<SOUTH>(SQUARE_BB(fromSq)));
            board.key ^= zobristEpFile[FILE_OF(board.enPass)];
        }
    } break;
    case CAPTURE:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq] = fromPc;
        board.lastCapt = toPc;

        board.halfMoves = 0;
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[toPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[getPcType(fromPc)] ^=
              SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(toPc)] ^= SQUARE_BB(toSq);

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[tPcIdx][toSq];

        if (getPcType(toPc) == ROOK) {
            board.castleRights &= castleMod[toSq];
        }

        if (getPcType(fromPc) == KING || getPcType(fromPc) == ROOK) {
            board.castleRights &= castleMod[fromSq];
        }

        break;
    case EP_CAPTURE: {
        Bitboard captured =
              board.turn == WHITE ? shift<SOUTH>(toSq) : shift<NORTH>(toSq);
        Piece cPiece = board.board[Sq(captured)];

        tPcIdx = cPiece;

        board.halfMoves = 0;
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = NO_PC;
        board.board[toSq] = fromPc;
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.color[~board.turn] ^= captured;
        board.pieceBB[cPiece] ^= captured;
        board.board[Sq(captured)] = NO_PC;

        board.cPieceBB[getPcType(fromPc)] ^=
              SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(cPiece)] ^= captured;

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^= zobristPieceSq[tPcIdx][Sq(captured)];
    } break;
    case K_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? H1 : H8;
        rTo = (board.turn == WHITE) ? F1 : F8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[fromSq] = NO_PC;
        board.board[rFrom] = NO_PC;

        board.board[toSq] = fromPc;
        board.board[rTo] = rPiece;

        board.lastCapt = NO_PC;
        board.castleRights &= castleMod[fromSq];

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^=
              zobristPieceSq[rPiece][rFrom] ^ zobristPieceSq[rPiece][rTo];
    } break;
    case Q_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? A1 : A8;
        rTo = (board.turn == WHITE) ? D1 : D8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[fromSq] = NO_PC;
        board.board[rFrom] = NO_PC;

        board.board[toSq] = fromPc;
        board.board[rTo] = rPiece;

        board.lastCapt = NO_PC;
        board.castleRights &= castleMod[fromSq];

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[fPcIdx][toSq];
        board.key ^=
              zobristPieceSq[rPiece][rFrom] ^ zobristPieceSq[rPiece][rTo];
    } break;
    case P_KNIGHT:
    case P_BISHOP:
    case P_ROOK:
    case P_QUEEN: {
        int pTo = getCapture(move) - P_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + pTo + (8 * board.turn));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[fromPc] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);
        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);

        board.board[fromSq] = NO_PC;
        board.board[toSq] = promoPc;
        board.lastCapt = NO_PC;

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[promoPc][toSq];
    } break;
    case CP_KNIGHT:
    case CP_BISHOP:
    case CP_ROOK:
    case CP_QUEEN: {
        int pTo = getCapture(move) - CP_KNIGHT;
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
        board.board[toSq] = promoPc;
        board.lastCapt = toPc;

        if (getPcType(toPc) == ROOK)
            board.castleRights &= castleMod[toSq];

        board.key ^=
              zobristPieceSq[fPcIdx][fromSq] ^ zobristPieceSq[promoPc][toSq];
        board.key ^= zobristPieceSq[tPcIdx][toSq];
    } break;
    }

    int temp = board.castleRights ^ board.hist[board.ply].castleStatus;
    board.key ^= zobristCastleRights[temp];

    board.turn = ~board.turn;
    board.key ^= 1;

    board.checkPcs =
          board.turn == WHITE
                ? board.attacksToKing<BLACK>(Sq(board.pieces(KING, board.turn)),
                                             board.color[0] | board.color[1])
                : board.attacksToKing<WHITE>(Sq(board.pieces(KING, board.turn)),
                                             board.color[0] | board.color[1]);

    board.ply++;
    board.gamePly++;

    if (board.turn == WHITE)
        board.fullMoves++;
}

void unmake(Board &board, unsigned short move) {
    int copy = 1;
    board.turn = ~board.turn;
    board.ply--;
    board.gamePly--;

    int ply = board.gamePly;
    board.checkPcs = board.hist[ply].checkPcs;
    board.castleRights = board.hist[ply].castleStatus;
    board.enPass = board.hist[ply].enPass;
    board.halfMoves = board.hist[ply].halfMoves;
    board.fullMoves = board.hist[ply].fullMoves;
    board.key = board.hist[ply].key;

    Square fromSq = getFrom(move);
    Square toSq = getTo(move);

    Piece piece = board.board[toSq];
    Piece capPiece = board.lastCapt;

    switch (getCapture(move)) {
    case QUIET:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq] = NO_PC;
        break;
    case DOUBLE_PAWN:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq] = NO_PC;
        break;
    case CAPTURE:
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq] = capPiece;
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[capPiece] ^= SQUARE_BB(toSq);
        board.cPieceBB[getPcType(piece)] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[getPcType(capPiece)] ^= SQUARE_BB(toSq);
        break;
    case EP_CAPTURE: {
        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.board[fromSq] = piece;
        board.board[toSq] = NO_PC;

        Bitboard captured =
              board.turn == WHITE ? shift<SOUTH>(toSq) : shift<NORTH>(toSq);
        Piece cPiece = Piece(W_PAWN + (8 * (~board.turn)));
        board.color[~board.turn] ^= captured;
        board.pieceBB[cPiece] ^= captured;
        board.board[Sq(captured)] = cPiece;
        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[PAWN] ^= (board.turn == WHITE)
                                      ? shift<SOUTH>(SQUARE_BB(board.enPass))
                                      : shift<NORTH>(SQUARE_BB(board.enPass));
    } break;
    case K_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? H1 : H8;
        rTo = (board.turn == WHITE) ? F1 : F8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[toSq] = NO_PC;
        board.board[rTo] = NO_PC;
        board.board[fromSq] = piece;
        board.board[rFrom] = rPiece;
    } break;
    case Q_CASTLE: {
        Square rFrom, rTo;
        Piece rPiece = Piece(W_ROOK + (8 * board.turn));

        rFrom = (board.turn == WHITE) ? A1 : A8;
        rTo = (board.turn == WHITE) ? D1 : D8;

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.color[board.turn] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.cPieceBB[KING] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.cPieceBB[ROOK] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);
        board.pieceBB[piece] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);
        board.pieceBB[rPiece] ^= SQUARE_BB(rFrom) ^ SQUARE_BB(rTo);

        board.board[toSq] = NO_PC;
        board.board[rTo] = NO_PC;
        board.board[fromSq] = piece;
        board.board[rFrom] = rPiece;
    } break;
    case P_KNIGHT:
    case P_BISHOP:
    case P_ROOK:
    case P_QUEEN: {
        int pTo = getCapture(move) - P_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + (pTo + (8 * board.turn)));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.pieceBB[W_PAWN + (8 * board.turn)] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);

        board.board[fromSq] = Piece(W_PAWN + (8 * board.turn));
        board.board[toSq] = NO_PC;
    } break;
    case CP_KNIGHT:
    case CP_BISHOP:
    case CP_ROOK:
    case CP_QUEEN: {
        Board c2(board);
        int pTo = getCapture(move) - CP_KNIGHT;
        Piece promoPc = Piece(W_KNIGHT + (pTo + (8 * board.turn)));

        board.color[board.turn] ^= SQUARE_BB(fromSq) ^ SQUARE_BB(toSq);

        board.pieceBB[W_PAWN + (8 * board.turn)] ^= SQUARE_BB(fromSq);
        board.pieceBB[promoPc] ^= SQUARE_BB(toSq);

        board.cPieceBB[PAWN] ^= SQUARE_BB(fromSq);
        board.cPieceBB[KNIGHT + pTo] ^= SQUARE_BB(toSq);
        board.cPieceBB[getPcType(capPiece)] ^= SQUARE_BB(toSq);
        board.board[fromSq] = Piece(W_PAWN + (8 * board.turn));
        board.board[toSq] = capPiece;

        if (!(board.turn == 0 || board.turn == 1)) {
            std::cout << "TURN: " << board.turn << std::endl;
        }
        board.color[~board.turn] ^= SQUARE_BB(toSq);
        board.pieceBB[capPiece] ^= SQUARE_BB(toSq);
    } break;
    }

    board.lastCapt = board.hist[ply].lastCapt;
}

} // namespace Yayo
