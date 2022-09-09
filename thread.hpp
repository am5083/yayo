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

namespace Yayo {

class Search {
  public:
    Search() { info = nullptr; }
    Search(const Search &)            = delete;
    Search &operator=(const Search &) = delete;

  public:
    const bool checkForStop() const;
    constexpr bool canReduce(int alpha, int move, moveList &mList);

    void startSearch(Info *_info);

    void wait();
    void isReady();
    void joinThread();
    void stopSearch();
    void printBoard() const;
    void _setFen(std::string fen);
    void _make(std::uint16_t move);
    void scoreMoves(moveList *mList);
    int quiescent(int alpha, int beta);
    int negaMax(int alpha, int beta, int depth);

    int search();

  private:
    int pvTable[MAX_PLY + 6][MAX_PLY + 6];
    int pvTableLen[MAX_PLY + 6];
    int killerMoves[MAX_PLY + 6][2];
    int killerMates[MAX_PLY + 6][2];
    int historyMoves[2][64][64];

    TPTable tpTbl;

    void updatePv(int ply, int move);
    void printPv();

  public:
    std::uint64_t get_nodes() const { return this->bench_nodes; }
    std::uint64_t bench_nodes = 0;

  private:
    int abortDepth;
    int numRep;
    int selDepth;
    int quiescentDepth;
    bool searched = false;

    bool canNullMove;

    std::uint64_t nodes;
    std::unique_ptr<std::thread> searchThread;
    Board _board;
    Info *info;
};

constexpr bool Search::canReduce(int alpha, int move, moveList &mList) {
    if (_board.checkPcs)
        return false;
    if (getCapture(move) >= CAPTURE)
        return false;
    // if (historyMoves[_board.turn][getFrom(move)][getTo(move)] > 1500)
    //     return false;
    // if (eval(_board, mList) > alpha)
    //     return false;

    return true;
}

} // namespace Yayo
#endif // THREAD_H_
