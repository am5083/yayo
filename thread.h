#ifndef THREAD_H_
#define THREAD_H_
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "util.h"
#include <cstring>
#include <string>

using namespace Yayo;

int SEARCHED = 0;

class Search {
  public:
    Search() { info = nullptr; }
    Search(const Search &) = delete;
    Search &operator=(const Search &) = delete;

  public:
    void startSearch(Info *_info) {
        if (SEARCHED)
            searchThread.get()->join();
        SEARCHED = 1;
        info = _info;
        searchThread.reset(new std::thread(&Search::search, this));
    }

    bool checkForStop() {
        if (info->stopTime <= get_time() && info->timeGiven) {
            return true;
        }

        return false;
    }

    void joinThread() {
        if (SEARCHED) {
            searchThread.get()->join();
        }
    }

    void stopSearch() { info->stopTime = 0; }

    void printBoard() { _board.print(); }

    void _setFen(std::string fen) { _board.setFen(fen); }
    void _make(std::uint16_t move) { make(_board, move); }

    void isReady() {
        if (SEARCHED) {
            info = new Info;
            info->stopTime = 0;
            searchThread.get()->join();
            free(info);
            SEARCHED = 0;
        }
        std::cout << "readyok\n";
    }

    int search() {
        int depth = info->depth;

        moveList mList = {0};
        generate(_board, &mList);

        int score, max = -100000;
        for (int j = 1; j < depth; j++) {
            for (int i = 0; i < mList.nMoves; i++) {
                make(_board, mList.moves[i].move);
                score = negaMax(-100000, 100000, j);
                unmake(_board, mList.moves[i].move);

                if (score > max) {
                    max = score;
                    best = mList.moves[i].move;
                }
            }

            std::cout << "info depth " << depth << " cp " << max << " pv ";

            for (int i = 0; i <= j; i++) {
                std::cout << i << std::endl;
                print_move(pvTable[0][i]);
            }

            std::cout << "\n";
        }

        std::cout << "bestmove ";
        print_move(best);
        std::cout << "\n";

        return 0;
    }

    int negaMax(int alpha, int beta, int depth) {
        if (depth == 0)
            return eval(_board);

        if (checkForStop())
            return 1000000;

        moveList mList = {0};
        generate(_board, &mList);

        for (int i = 0; i < mList.nMoves; i++) {
            make(_board, mList.moves[i].move);
            int score = -negaMax(-beta, -alpha, depth - 1);
            unmake(_board, mList.moves[i].move);

            if (score >= beta)
                return beta;
            if (score > alpha) {
                updatePv(_board.ply, mList.moves[i].move);
                alpha = score;
            }
        }

        return alpha;
    }

  private:
    int pvTable[MAX_PLY + 6][MAX_PLY + 6] = {0};
    int pvTableLen[MAX_PLY + 6] = {0};

    void updatePv(int ply, int move) {
        pvTable[ply][0] = move;

        for (int i = 0; i < pvTableLen[ply + 1]; i++)
            pvTable[ply][i + 1] = pvTable[ply + 1][i];

        pvTableLen[ply] = 1 + pvTableLen[ply + 1];
    }

  private:
    int best;
    std::unique_ptr<std::thread> searchThread;
    Board _board;
    Info *info;
};
#endif // THREAD_H_
