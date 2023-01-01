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
#include "move.hpp"
#include "util.hpp"
#include <cmath>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

namespace Yayo {

static int deltaMargin = 200;

static int rfpDepth = 8;
static int rfpP1 = 75;
static int rfpP2 = 28;

// currently tuning
static int nmpDepth = 2;
static int nmpRed = 4;
static int nmpDepthDiv = 6;
static int nmpPar1 = 3;
static int nmpPar2 = 100;

static int razorDepth = 3;
static int razorMargin1 = 130;
static int razorMargin2 = 150;

static int iidDepth = 4;

static int futilityMargin1 = 125;
static int futilityMargin2 = 100;

static int quietSeeThrshld = -60;
static int capSeeThrshld = -80;

// next to tune
static int maxNumFailed = 5;
static int alphaWindowInit = -12;
static int betaWindowInit = 12;
static double alphaWindowMultiplier = 1.45;
static double betaWindowMultiplier = 1.45;
static int aspDepth = 3;

static int lmpPar1 = 6;
static int lmpDiv1 = 2;
static int lmpPar2 = 5;
static int lmpM1 = 2;
static int lmpM2 = 3;
static int lmpDiv2 = 2;

struct HistEntry {
    unsigned move;
    int eval;
};

class Search {
  public:
    Search() {
        info = nullptr;
        memset(lmrDepthReduction, 0, sizeof(lmrDepthReduction));
        memset(lmpThresholds, 0, sizeof(lmpThresholds));

        for (int depth = 0; depth < 64; depth++) {
            for (int moves = 0; moves < 64; moves++) {
                lmrDepthReduction[depth][moves] =
                      1 + std::log(depth) * std::log(moves) / 2.5;
            }
        }

        for (int i = 1; i < 9; i++) {
            lmpThresholds[0][i] = (lmpPar1 + lmpM1 + i * i) / lmpDiv1;
            lmpThresholds[1][i] = (lmpPar2 + lmpM2 + i * i) / lmpDiv2;
        }
    }
    Search(const Search &) = delete;
    Search &operator=(const Search &) = delete;

  public:
    bool checkForStop() const;
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
    int historyMoves[17][64][64];
    long lmrDepthReduction[64][64];
    int lmpThresholds[2][9];
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
    Board board;
    Info *info;
};

} // namespace Yayo
#endif // THREAD_H_
