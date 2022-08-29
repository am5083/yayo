#ifndef THREAD_H_
#define THREAD_H_
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "util.h"
#include <cstring>
#include <fstream>
#include <string>

using namespace Yayo;

int SEARCHED = 0;
static std::ofstream ofs("log.yayo", std::ios::out);

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
        for (int i = 0; i < MAX_PLY + 6; i++) {
            pvTableLen[i] = NO_MOVE;
            for (int j = 0; j < MAX_PLY + 6; j++) {
                pvTable[i][j] = NO_MOVE;
            }
        }
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

    void _make(std::uint16_t move) {
        make(_board, move);
        _board.ply = 0;
    }

    void isReady() {
        if (SEARCHED) {
            info = new Info;
            info->stopTime = 0;
            searchThread.get()->join();
            free(info);
            SEARCHED = 0;
        }
        std::cout << "readyok" << std::endl;
    }

    void pvCheck() {
        updatePv(0, encodeMove(E2, E4, DOUBLE_PAWN));
        updatePv(1, encodeMove(E7, E5, DOUBLE_PAWN));
        updatePv(2, encodeMove(C7, C5, DOUBLE_PAWN));

        for (int i = 0; i < 3; i++) {
            std::cout << "pvTableLen[" << i << "] = " << pvTableLen[i] << std::endl;
            std::cout << "pvTable[0][" << i << "] = ";
            print_move(pvTable[0][i]);
            std::cout << std::endl;
        }
    }

    int search() {
        int depth = info->depth;

        int score;
        for (int j = 1; j <= depth; j++) {
            if (checkForStop())
                break;
            score = negaMax(-5000000, 5000000, j);
            std::cout << "info depth " << j << " cp " << score << " pv ";
            printPv();
            std::cout << std::endl;
        }

        std::cout << "bestmove ";
        print_move(pvTable[0][0]);
        std::cout << std::endl;

        return 0;
    }

    int negaMax(int alpha, int beta, int depth) {
        if (checkForStop())
            return 1000000;

        if (depth == 0)
            return eval(_board);

        if (_board.ply > 0 && (_board.halfMoves >= 100 || _board.isDraw()))
            return 1 - (nodes & 2);

        if (_board.ply > 0 && isRepetition(_board)) {
            if (numRepetition(_board) >= 2) {
                return 1 - (nodes & 2);
            }
        }

        moveList mList = {0};
        generate(_board, &mList);

        for (int i = 0; i < mList.nMoves; i++) {
            nodes++;
            make(_board, mList.moves[i].move);
            int score = -negaMax(-beta, -alpha, depth - 1);
            unmake(_board, mList.moves[i].move);

            if (score >= beta)
                return score;
            if (score > alpha) {
                updatePv(_board.ply, mList.moves[i].move);
                alpha = score;
            }
        }

        if (mList.nMoves == 0 && _board.checkPcs) {
            pvTableLen[0] = _board.ply;
            return -200000 + _board.ply;
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

    void printPv() {
        for (int i = 0; i < pvTableLen[0]; i++) {
            if (pvTable[0][i] == NO_MOVE)
                break;
            print_move(pvTable[0][i]);
            std::cout << " ";
        }
    }

  private:
    std::uint64_t nodes;
    int numRep;
    std::unique_ptr<std::thread> searchThread;
    Board _board;
    Info *info;
};
#endif // THREAD_H_
