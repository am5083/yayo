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

struct HistEntry {
    int move, eval;
};

class Search {
  public:
    Search() {
        info = nullptr;
        memset(lmrDepthReduction, 0, sizeof(lmrDepthReduction));

        for (int depth = 0; depth < 64; depth++) {
            // std::cout << "depth: " << depth << std::endl;
            for (int moves = 0; moves < 64; moves++) {
                // std::cout << "moves: " << moves << ", R = ";
                lmrDepthReduction[depth][moves] =
                      1 + std::log(depth) * std::log(moves) / 2.5;
                // std::cout << lmrDepthReduction[depth][moves] << std::endl;
            }
        }
    }
    Search(const Search &) = delete;
    Search &operator=(const Search &) = delete;

  public:
    const bool checkForStop() const;
    constexpr bool canReduce(int alpha, int move, Move &m);

    void startSearch(Info *_info);

    void clearTT(int size);
    void wait();
    void isReady();
    void joinThread();
    void stopSearch();
    void printBoard() const;
    void _setFen(std::string fen);
    void _make(std::uint16_t move);
    void scoreMoves(moveList *mList, unsigned ttMove = 0);
    int quiescent(int alpha, int beta);
    int negaMax(int alpha, int beta, int depth, bool cutNode,
                bool isExtension = false);
    moveList generateMoves();
    Board getBoard();

    int search();

  private:
    int pvTable[MAX_PLY + 6][MAX_PLY + 6];
    int pvTableLen[MAX_PLY + 6];
    int killerMoves[MAX_PLY + 6][2];
    int killerMates[MAX_PLY + 6][2];
    int historyMoves[2][64][64];
    long lmrDepthReduction[64][64];
    HistEntry Hist[512];

    void updatePv(int ply, unsigned move);
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
