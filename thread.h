#ifndef THREAD_H_
#define THREAD_H_
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"
#include "util.h"
#include <cstring>
#include <fstream>
#include <string>

using namespace Yayo;

int SEARCHED = 0;
static std::ofstream ofs("log.yayo", std::ios::out);

const int INF = 100000;

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
        numRep = 0;
        for (int i = 0; i < MAX_PLY + 6; i++) {
            pvTableLen[i] = NO_MOVE;
            killerMoves[i][0] = NO_MOVE;
            killerMoves[i][1] = NO_MOVE;

            for (int j = 0; j < MAX_PLY + 6; j++) {

                if (i < 64 && j < 64) {
                    historyMoves[0][i][j] = 0;
                    historyMoves[1][i][j] = 0;
                }
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

    void scoreMoves(moveList *mList) {
        for (int i = 0; i < mList->nMoves; i++) {
            int move = mList->moves[i].move;

            if (move == pvTable[0][_board.ply]) {
                mList->moves[i].score = 10000000 * 2;
                continue;
            }

            if (int(getCapture(move)) < int(CAPTURE)) {
                if (killerMates[_board.ply][0] == move) {
                    mList->moves[i].score = 100000;
                }

                else if (killerMates[_board.ply][1] == move) {
                    mList->moves[i].score = 95000;
                }

                else if (killerMoves[_board.ply][0] == move) {
                    mList->moves[i].score = 90000;
                }

                else if (killerMoves[_board.ply][1] == move) {
                    mList->moves[i].score = 80000;
                }

                else {
                    mList->moves[i].score = historyMoves[_board.turn][getFrom(move)][getTo(move)];
                }
            }
        }
    }

    int negaMax(int alpha, int beta, int depth) {
        if (checkForStop())
            return 10000000;

        nodes++;

        int ply = _board.ply;
        if (_board.ply > 0) {
            alpha = std::max(alpha, -INF + _board.ply);
            beta = std::min(beta, INF - _board.ply - 1);

            if (alpha >= beta) {
                if (killerMates[ply][0] != pvTable[ply][0]) {
                    killerMates[ply][1] = killerMates[ply][0];
                    killerMates[ply][0] = pvTable[ply][0];
                }

                return alpha;
            }

            if (_board.halfMoves >= 100 || _board.isDraw())
                return 1 - (nodes & 2);

            if (isRepetition(_board)) {
                numRep++;
                if (numRepetition(_board) >= 2 || numRep > 2) {
                    return 1 - (nodes & 2);
                }
            }
        }

        if (depth == 0)
            return eval(_board);

        pvTableLen[_board.ply] = 0;

        moveList mList = {{{0}}};
        generate(_board, &mList);
        scoreMoves(&mList);

        int movesSearched = 0;
        for (int i = 0; i < mList.nMoves; i++) {
            mList.swapBest(i);
            const int move = mList.moves[i].move;

            int score;
            make(_board, mList.moves[i].move);
            movesSearched++;
            if (movesSearched == 1) {
                score = -negaMax(-beta, -alpha, depth - 1);
            } else {
                score = -negaMax(-alpha - 1, -alpha, depth - 1);

                if (score > alpha) {
                    score = -negaMax(-beta, -alpha, depth - 1);
                }
            }

            unmake(_board, mList.moves[i].move);

            if (score >= beta) {
                if (getCapture(move) < CAPTURE) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = mList.moves[i].move;

                    historyMoves[_board.turn][getFrom(move)][getTo(move)] += depth * depth;
                }
                return score;
            }

            if (score > alpha) {
                updatePv(_board.ply, mList.moves[i].move);
                alpha = score;
            }
        }

        if (mList.nMoves == 0 && _board.checkPcs) {
            return -INF + _board.ply;
        } else if (mList.nMoves == 0) { // stalemate
            return 1 - (nodes & 2);
        }

        return alpha;
    }

    int search() {
        int depth = info->depth;

        int score;
        for (int j = 1; j <= depth; j++) {
            if (checkForStop())
                break;

            nodes = 0;
            score = negaMax(-INF, INF, j);
            std::cout << "info depth " << j << " score cp " << score << " nodes " << nodes << " pv ";
            printPv();
            std::cout << std::endl;
        }

        std::cout << "bestmove ";
        print_move(pvTable[0][0]);
        std::cout << std::endl;

        return 0;
    }

  private:
    int pvTable[MAX_PLY + 6][MAX_PLY + 6];
    int pvTableLen[MAX_PLY + 6];
    int killerMoves[MAX_PLY + 6][2];
    int killerMates[MAX_PLY + 6][2];
    int historyMoves[2][64][64];

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
