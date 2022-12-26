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

#include "board.hpp"
#include "bitboard.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Yayo {

static std::string uncdPcs[12] = {
      "♙", "♘", "♗", "♖", "♕", "♔", "♟", "♞", "♝", "♜", "♛", "♚",
};

void Board::print() const {
    for (int rank = 0; rank < 8; rank++) {
        // print all the file letters before the first rank
        if (rank == 0) {
            printf("   ");
            for (int i = 0; i < 8; i++)
                printf(" %c ", 'A' + i);
            printf("\n");
        }

        printf("%2d ", 8 - rank);

        // print the status of the bit at each square
        for (int file = 0; file < 8; file++) {
            int s = rank * 8 + file;

            int piece = -1;
            for (int i = 0; i < 16; i++) {
                if (GET(pieceBB[i], s)) {
                    piece = i;
                    break;
                }
            }

            if (piece >= B_PAWN)
                piece -= 2;
            printf(" %2s", (piece != -1) ? uncdPcs[piece - 1].c_str() : ".");
        }

        printf(" %d", 8 - rank);
        printf("\n");
    }

    printf("   ");
    for (int i = 0; i < 8; i++)
        printf(" %c ", 'A' + i);
    printf("\n\n");

    int check = 0;
    if (popcount(checkPcs) >= 2)
        check = 0;
    else if (checkPcs)
        check = board[Sq(checkPcs)];
    if (check >= B_PAWN)
        check -= 2;

    printf("Side to move: %s\n", (turn) ? "Black" : "White");
    printf("Castle rights: %d\n\n", castleRights);
    printf("En Passant: %s\n", (enPass < 64) ? nToSq[enPass].c_str() : "N/A");
    printf("Checking piece: %s\n",
           checkPcs ? uncdPcs[check - 1].c_str() : "N/A");
    printf("FEN: %s\n", fen().c_str());

    printf("Position key: %llu\n\n", key);
}

std::string Board::fen() const {
    std::string fen;
    std::unordered_map<Piece, char> pieceToChar{
          {W_PAWN, 'P'},   {W_KNIGHT, 'N'}, {W_BISHOP, 'B'}, {W_ROOK, 'R'},
          {W_QUEEN, 'Q'},  {W_KING, 'K'},   {B_PAWN, 'p'},   {B_KNIGHT, 'n'},
          {B_BISHOP, 'b'}, {B_ROOK, 'r'},   {B_QUEEN, 'q'},  {B_KING, 'k'},
    };

    for (Rank rank = RANK_8; rank <= RANK_1; rank++) {
        int idx = 0;
        for (File file = FILE_A; file <= FILE_H; file++) {
            Square sq = GET_SQ(rank, file);
            if (board[sq] == NO_PC) {
                idx++;
            } else {
                if (idx)
                    fen += char(idx + '0');
                idx = 0;
                fen += pieceToChar[board[sq]];
            }
        }
        if (idx)
            fen += char(idx + '0');
        if (rank < RANK_1)
            fen += "/";
    }

    fen += " ";
    fen += (turn == WHITE) ? "w" : "b";
    fen += " ";

    if (castleRights & 8)
        fen += "K";
    if (castleRights & 4)
        fen += "Q";
    if (castleRights & 2)
        fen += "k";
    if (castleRights & 1)
        fen += "q";
    if (!castleRights)
        fen += "-";

    fen += " ";

    if (enPass != SQUARE_64) {
        fen += nToSq[enPass];
    } else {
        fen += "-";
    }

    fen += " ";
    std::string nmoves;
    int num = halfMoves;
    while (num)
        nmoves += char('0' + num % 10), num /= 10;
    std::reverse(nmoves.begin(), nmoves.end());
    if (halfMoves)
        fen += nmoves;
    else
        fen += "0";

    fen += " ";
    nmoves = "", num = fullMoves;
    while (num)
        nmoves += char('0' + num % 10), num /= 10;
    std::reverse(nmoves.begin(), nmoves.end());
    if (fullMoves)
        fen += nmoves;
    else
        fen += "0";

    return fen;
}

void Board::setFen(std::string &fen) {
    key = 0;
    ply = 0;
    gamePly = 0;
    lastCapt = NO_PC;
    color[WHITE] = 0;
    color[BLACK] = 0;

    for (int i = 0; i < 64; i++) {
        if (i < 16)
            pieceBB[i] = 0;
        if (i < 7)
            cPieceBB[i] = 0;
        board[i] = NO_PC;
    }

    std::unordered_map<char, int> f_ctoi{
          {'P', W_PAWN}, {'N', W_KNIGHT}, {'B', W_BISHOP},
          {'R', W_ROOK}, {'Q', W_QUEEN},  {'K', W_KING},

          {'p', B_PAWN}, {'n', B_KNIGHT}, {'b', B_BISHOP},
          {'r', B_ROOK}, {'q', B_QUEEN},  {'k', B_KING},

          {'w', WHITE},  {'b', BLACK},
    };

    std::unordered_map<char, int> f_ctop{
          {'P', PAWN},  {'N', KNIGHT}, {'B', BISHOP},
          {'R', ROOK},  {'Q', QUEEN},  {'K', KING},

          {'p', PAWN},  {'n', KNIGHT}, {'b', BISHOP},
          {'r', ROOK},  {'q', QUEEN},  {'k', KING},

          {'w', WHITE}, {'b', BLACK},
    };

    int idx = 0;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            const int sq = rank * 8 + file;
            if (((fen[idx]) >= 'a' && (fen[idx]) <= 'z') ||
                ((fen[idx]) >= 'A' && (fen[idx]) <= 'Z')) {
                board[sq] = Piece(f_ctoi[fen[idx]]);
                pieceBB[f_ctoi[fen[idx]]] =
                      pieceBB[f_ctoi[fen[idx]]] | SQUARE_BB(Square(sq));
                cPieceBB[f_ctop[fen[idx]]] =
                      cPieceBB[f_ctop[fen[idx]]] | SQUARE_BB(Square(sq));
                key ^= zobristPieceSq[board[sq]][sq];
                idx++;
            }

            if ((fen[idx]) > '0' && (fen[idx]) <= '9') {
                int64_t p = -1;
                int j = fen[idx] - '0';

                for (unsigned long long i : cPieceBB)
                    if (GET(i, sq))
                        p = GET(i, sq);

                if (p == -1) {
                    for (int i = 0; i < j; i++) {
                        board[sq + i] = NO_PC;
                    }
                    file--;
                }

                file += j;
                idx++;
            }

            if (fen[idx] == '/')
                idx++;
        }
    }
    idx++;

    if (fen[idx] == 'w')
        turn = WHITE;
    else {
        turn = BLACK;
        key ^= 1;
    }

    castleRights = 0;
    idx += 2;
    if (fen[idx] == 'K')
        idx++, castleRights |= (1 << 3), key ^= zobristCastleRights[1 << 3];
    if (fen[idx] == 'Q')
        idx++, castleRights |= (1 << 2), key ^= zobristCastleRights[1 << 2];
    if (fen[idx] == 'k')
        idx++, castleRights |= (1 << 1), key ^= zobristCastleRights[1 << 1];
    if (fen[idx] == 'q')
        idx++, castleRights |= 1, key ^= zobristCastleRights[1];
    if (fen[idx] == '-')
        idx++;

    idx++;
    if (fen[idx] != '-') {
        const int f = fen[idx] - 'a';
        idx++;
        const int r = 8 - (fen[idx] - '0');
        enPass = Square(8 * r + f);
    } else {
        enPass = SQUARE_64;
        idx += 2;
    }

    const int len = fen.length();
    int n = 0;
    while (idx < len && fen[idx] < '9' && fen[idx] > '0')
        n = n * 10 + fen[idx] - '0', idx++;
    halfMoves = n;

    idx += 2;
    n = 0;
    while (idx < len && fen[idx] < '9' && fen[idx] > '0')
        n = n * 10 + fen[idx] - '0', idx++;
    fullMoves = n;

    key ^= (enPass != SQUARE_64) ? zobristEpFile[enPass % 8] : 0;

    color[WHITE] = pieceBB[W_PAWN] | pieceBB[W_KNIGHT] | pieceBB[W_BISHOP] |
                   pieceBB[W_ROOK] | pieceBB[W_QUEEN] | pieceBB[W_KING];

    color[BLACK] = pieceBB[B_PAWN] | pieceBB[B_KNIGHT] | pieceBB[B_BISHOP] |
                   pieceBB[B_ROOK] | pieceBB[B_QUEEN] | pieceBB[B_KING];

    checkPcs = (turn == WHITE)
                     ? attacksToKing<BLACK>(Sq(pieces(KING, WHITE)),
                                            color[WHITE] | color[BLACK])
                     : attacksToKing<WHITE>(Sq(pieces(KING, BLACK)),
                                            color[WHITE] | color[BLACK]);
}

std::uint64_t Board::hash() const {
    std::uint64_t h = 0;
    h ^= turn * 1;

    for (int i = 0; i < 64; i++) {
        if (board[i] != NO_PC)
            h ^= zobristPieceSq[board[i]][i];
    }

    h ^= zobristCastleRights[castleRights];

    h ^= enPass != SQUARE_64 ? zobristEpFile[FILE_OF(enPass)] : 0;

    return h;
}

int Board::see(Square toSq, Piece toPc, Square from, Piece fromPc) {
    int gain[32];
    int ply = 0;

    Bitboard occ = pieces();
    Bitboard xRayPcs =
          pieces(PAWN) | pieces(BISHOP) | pieces(ROOK) | pieces(QUEEN);
    Bitboard fromMap = SQUARE_BB(from);
    Bitboard atkDefMap = attacksToSq(toSq, occ);

    std::array<std::uint16_t, 8> pcVal = {0,          PAWN_VAL, KNIGHT_VAL,
                                          BISHOP_VAL, ROOK_VAL, QUEEN_VAL,
                                          KING_VAL,   0};

    if (toPc != NO_PC)
        gain[ply] = pcVal[getPcType(toPc)];
    else
        gain[ply] = 0;

    char startTurn = turn;

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

        startTurn ^= 1;
        fromMap = getLVA(Color(startTurn), atkDefMap, &fromPc);
    } while (fromMap && ply < 32);

    while (--ply && (ply - 1) >= 0) {
        gain[ply - 1] = -std::max(-gain[ply - 1], gain[ply]);
    }

    return gain[0];
}

Board::Board() {
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

    memset(color, 0, sizeof(color));
    memset(board, 0, sizeof(board));
    memset(pieceBB, 0, sizeof(pieceBB));
    memset(cPieceBB, 0, sizeof(cPieceBB));
}

Board::Board(const Board &other) {
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

constexpr bool Board::operator==(const Board &b1) const {
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

            print_bitboard(cPieceBB[1]);
            print_bitboard(b1.cPieceBB[1]);

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
} // namespace Yayo
