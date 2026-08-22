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

inline int deltaMargin = 210;

inline int rfpDepth = 8;
inline int rfpP1 = 61;
inline int rfpP2 = 7;

// currently tuning
inline int nmpDepth = 2;
inline int nmpRed = 4;
inline int nmpDepthDiv = 2;
inline int nmpPar1 = 101;

inline int razorDepth = 3;
inline int razorMargin1 = 142;
inline int razorMargin2 = 166;

inline int iidDepth = 4;

inline int futilityMargin1 = 138;
inline int futilityMargin2 = 86;

inline int quietSeeThrshld = -66;
inline int capSeeThrshld = -101;

// next to tune
inline int maxNumFailed = 5;
inline int betaWindowInit = 18;
inline int alphaWindowInit = -18;
inline double alphaWindowMultiplier = 1.32;
inline double betaWindowMultiplier = 1.57;
inline int aspDepth = 3;

inline int lmrP1 = 1;
inline double lmrP2 = 2.5;

inline int lmpPar1 = 3;
inline int lmpPar2 = 5;
inline double lmpDiv1 = 1.3262048850827794;
inline double lmpDiv2 = 2.2026788100055468;

struct HistEntry {
    unsigned move;
    int eval;
};

class Search {
  public:
    Search() {
        info = nullptr;
        memset(pvTable, 0, sizeof(pvTable));
        memset(pvTableLen, 0, sizeof(pvTableLen));
        memset(killerMoves, 0, sizeof(killerMoves));
        memset(killerMates, 0, sizeof(killerMates));
        memset(historyMoves, 0, sizeof(historyMoves));
        memset(Hist, 0, sizeof(Hist));
        memset(lmrDepthReduction, 0, sizeof(lmrDepthReduction));
        memset(lmpThresholds, 0, sizeof(lmpThresholds));

        for (int depth = 0; depth < 64; depth++) {
            for (int moves = 0; moves < 64; moves++) {
                lmrDepthReduction[depth][moves] =
                      lmrP1 + std::log(depth) * std::log(moves) / lmrP2;
            }
        }

        for (int i = 1; i < 9; i++) {
            lmpThresholds[0][i] = (lmpPar1 + i * i) / lmpDiv1;
            lmpThresholds[1][i] = (lmpPar2 + i * i) / lmpDiv2;
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
