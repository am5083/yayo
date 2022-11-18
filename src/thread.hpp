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

#ifndef THREAD_H_
#define THREAD_H_
#include "board.hpp"
#include "eval.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "tt.hpp"
#include "util.hpp"
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Yayo {

class Search {
  public:
    Search() {
        info = nullptr;

        memset(&lmpThresholds, 0, sizeof(lmpThresholds));
        memset(&lmrDepthReduction, 0, sizeof(lmrDepthReduction));
        // memset(&pvTable, 0, sizeof(pvTable));
        // memset(&pvTableLen, 0, sizeof(pvTableLen));
        // memset(&killerMoves, 0, sizeof(killerMoves));
        // memset(&historyMoves, 0, sizeof(historyMoves));
        // memset(&Hist, 0, sizeof(Hist));

        for (int depth = 0; depth < 64; depth++) {
            // std::cout << "depth: " << depth << std::endl;
            for (int moves = 0; moves < 64; moves++) {
                // std::cout << "moves: " << moves << ", R = ";
                lmrDepthReduction[depth][moves] =
                      1 + log(depth) * log(moves) / 2.5;

                // std::cout << lmrDepthReduction[depth][moves] << std::endl;
            }
        }

        for (int i = 1; i < 9; i++) {
            lmpThresholds[0][i] = (3 + i * i) / 2;
            lmpThresholds[1][i] = 3 + i * i;
        }
    }
    Search(const Search &) = delete;
    Search &operator=(const Search &) = delete;

  public:
    const bool checkForStop() const;
    constexpr bool canReduce(int alpha, int move, Move &m);

    void startSearch(Info *_info);

    void clearTT();
    void wait();
    void isReady();
    void joinThread();
    void stopSearch();
    void printBoard() const;
    void _setFen(std::string fen);
    void _make(std::uint16_t move);
    void scoreMoves(moveList *mList, int ttMove = 0);
    int quiescent(int alpha, int beta);
    int negaMax(int alpha, int beta, int depth, bool nullMove, bool pvNode,
                bool cutNode, bool isExtension = false);
    moveList generateMoves();
    Board getBoard();

    int search();

  private:
    int pvTable[MAX_PLY + 6][MAX_PLY + 6];
    int pvTableLen[MAX_PLY + 6];
    int killerMoves[MAX_PLY + 6][2];
    int killerMates[MAX_PLY + 6][2];
    int historyMoves[2][64][64];
    int lmrDepthReduction[64][64];
    int lmpThresholds[2][9];

    HistEntry Hist[256];

    void updatePv(int ply, unsigned short move);
    void printPv();

  public:
    bool probe = true;
    std::vector<int> getPv();
    void setInfo(Info *i) { info = i; }

  public:
    std::uint64_t get_nodes() const { return this->bench_nodes; }
    std::uint64_t bench_nodes = 0;

  private:
    int abortDepth;
    int numRep;
    int selDepth;
    int quiescentDepth;
    bool searched = false;

    mutable int stopFlag = 0;
    mutable std::uint64_t stopCount = 0;

    std::unique_ptr<std::thread> searchThread;
    std::uint64_t nodes;
    std::uint64_t qnodes;
    Board _board;
    Info *info;
};

constexpr bool Search::canReduce(int alpha, int move, Move &m) {
    if (getCapture(move) >= CAPTURE) {
        if (m.score > 0)
            return false;
        else
            return true;
    }

    if (historyMoves[_board.turn][getFrom(move)][getTo(move)] >= 2500)
        return false;

    if (getPcType(_board.board[getFrom(move)]) == PAWN)
        return false;

    if (killerMoves[_board.ply][0] == move)
        return false;
    // if (eval(_board, mList) > alpha)
    //     return false;

    return true;
}

} // namespace Yayo
#endif // THREAD_H_
