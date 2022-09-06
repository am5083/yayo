#ifndef UCI_H_
#define UCI_H_
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "thread.h"
#include "util.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace Yayo;
using namespace Yayo::Bitboards;

static inline int parseMove(Board &board, std::string move) {
    moveList mList = {0};
    generate(board, &mList);

    int fromSq = (move[0] - 'a') + (8 - (move[1] - '0')) * 8;
    int toSq   = (move[2] - 'a') + (8 - (move[3] - '0')) * 8;
    int promPc = 0;

    for (int i = 0; i < mList.nMoves; i++) {
        int mv = mList.moves[i].move;
        if (fromSq == getFrom(mv) && toSq == getTo(mv)) {
            promPc = getCapture(mv);

            if (promPc >= P_KNIGHT) {
                if ((promPc == CP_QUEEN || promPc == P_QUEEN) && move[4] == 'q')
                    return mv;
                else if ((promPc == CP_ROOK || promPc == P_ROOK) && move[4] == 'r')
                    return mv;
                else if ((promPc == CP_BISHOP || promPc == P_BISHOP) && move[4] == 'b')
                    return mv;
                else if ((promPc == CP_KNIGHT || promPc == P_KNIGHT) && move[4] == 'n')
                    return mv;
                continue;
            }
            return mv;
        }
    }
    return encodeMove(Square(fromSq), Square(toSq), MoveFlag(QUIET));
}

namespace Yayo {

Bitboard divide(Board &board, moveList *mL, int start, int cur) {
    moveList mList = {0};
    generate(board, &mList);

    if (cur == 1) {
        if (mL == nullptr)
            return mList.nMoves;

        for (int i = 0; i < mList.nMoves; i++) {
            mL->moves[i] = mList.moves[i];
        }

        mL->nMoves = mList.nMoves;

        return mList.nMoves;
    }

    Bitboard localNodes = 0;
    for (int i = 0; i < mList.nMoves; i++) {
        make(board, mList.moves[i].move);
        Bitboard thisMoves = divide(board, nullptr, start, cur - 1);
        localNodes += thisMoves;
        unmake(board, mList.moves[i].move);
        if (cur == start) {
            print_move(mList.moves[i].move);
            std::cout << ": " << thisMoves << std::endl;
        }
    }

    return localNodes;
}

class UCI {
  public:
    UCI(Search &searcher) : search(searcher){};
    void Main();

  private:
    void Uci();
    void NewGame();
    void IsReady();
    void Go(Info *info);
    void Stop();
    void Perft(int depth);

  private:
    Search &search;
};

void UCI::Uci() {
    std::cout << "id name Yayo" << std::endl;
    std::cout << "id author am5083" << std::endl;
    std::cout << "uciok" << std::endl;
}

void UCI::NewGame() { search._setFen(START_POS); }

void UCI::IsReady() { search.isReady(); }

void UCI::Go(Info *info) { search.startSearch(info); }

void UCI::Stop() { search.stopSearch(); }

void UCI::Perft(int depth) {
    Board board;

    unsigned long long n;
    if (depth == 1) {
        moveList mList = {0};
        n              = divide(board, &mList, depth, depth);

        for (int i = 0; i < mList.nMoves; i++) {
            print_move(mList.moves[i].move);
            std::cout << ": 1\n";
        }

        std::cout << std::endl;
    } else {
        n = divide(board, nullptr, depth, depth);
        std::cout << std::endl;
    }

    std::cout << "total: " << n << std::endl;
}

void UCI::Main() {
    init_arrays();
    initMvvLva();

    Board board;
    board.setFen(START_POS);
    search._setFen(START_POS);

    Info info[1];

    std::cout << "id name Yayo" << std::endl;
    // std::cout << "uciok" << std::endl;

    while (1) {
        std::string input;
        getline(std::cin, input);

        std::istringstream iss(input);
        std::string cmd;

        iss >> std::skipws >> cmd;

        if (cmd == "isready") {
            IsReady();
        } else if (cmd == "position") {
            std::string position;

            while (iss >> position) {
                if (position == "startpos") {
                    board.setFen(START_POS);
                    search._setFen(START_POS);
                } else if (position == "fen") {
                    std::string fen;

                    for (int i = 0; i < 6; i++) {
                        std::string sub;
                        iss >> sub;
                        fen += sub + " ";
                    }
                    search._setFen(fen);
                    board.setFen(fen);
                } else if (position == "moves") {
                    std::string moves;

                    while (iss >> moves) {
                        int move = parseMove(board, moves);
                        search._make(move);
                        make(board, move);
                    }
                }
            }
        } else if (cmd == "ucinewgame") {
            NewGame();
        } else if (cmd == "see") {
            std::string move;
            iss >> move;
            int m = parseMove(board, move);
            std::cout << "static exchange evaluation: "
                      << board.see(getTo(m), board.board[getTo(m)], getFrom(m), board.board[getFrom(m)]);
            std::cout << std::endl;
        } else if (cmd == "d") {
            search.printBoard();
        } else if (cmd == "make") {
            std::string move;
            iss >> move;
            int m = parseMove(board, move);
            search._make(m);
        } else if (cmd == "go") {
            int depth       = -1;
            int movestogo   = 30;
            int movetime    = -1;
            int time        = -1;
            int increment   = 0;
            bool turn       = board.turn;
            info->timeGiven = false;

            std::string tc;

            while (iss >> tc) {
                if (tc == "infinite") {
                    depth = 100;
                    continue;
                } else if (tc == "binc" && turn == BLACK) {
                    iss >> increment;
                } else if (tc == "winc" && turn == WHITE) {
                    iss >> increment;
                } else if (tc == "btime" && turn == BLACK) {
                    iss >> time;
                } else if (tc == "wtime" && turn == WHITE) {
                    iss >> time;
                } else if (tc == "movestogo") {
                    iss >> movestogo;
                } else if (tc == "movetime") {
                    iss >> movetime;
                } else if (tc == "depth") {
                    iss >> depth;
                }
            }

            if (movetime != -1) {
                time = movetime;
                movestogo *= 2;
            }

            info->startTime = get_time();
            info->depth     = depth;

            int cStopTime, hStopTime;

            if (time != -1) {
                cStopTime = time / (movestogo + 1) + increment;
                hStopTime = std::min(cStopTime * 5, time / std::min(4, movestogo));

                hStopTime            = std::max(10, std::min(hStopTime, time));
                cStopTime            = std::max(1, std::min(cStopTime, hStopTime));
                info->timeControl    = cStopTime;
                info->maxTimeControl = hStopTime;
                info->stopTime       = info->startTime + cStopTime;
                info->timeGiven      = true;
            }

            if (depth == -1) {
                info->depth = 12;
            }

            info->uciStop = false;
            info->uciQuit = false;

            Go(info);
        } else if (cmd == "quit") {
            search.stopSearch();
            search.joinThread();
            break;
        } else if (cmd == "stop") {
            Stop();
        } else if (cmd == "uci") {
            Uci();
        } else if (cmd == "setoption") {
        } else if (cmd == "eval") {
            moveList mList = {0};
            generate(board, &mList);
            std::cout << eval(board, mList) << std::endl;
        } else if (cmd == "perft") {
            int depth;
            iss >> depth;

            unsigned long long n;
            if (depth == 1) {
                moveList mList = {0};
                n              = divide(board, &mList, depth, depth);

                for (int i = 0; i < mList.nMoves; i++) {
                    print_move(mList.moves[i].move);
                    std::cout << ": 1\n";
                }

                std::cout << std::endl;
            } else {
                n = divide(board, nullptr, depth, depth);
                std::cout << std::endl;
            }

            std::cout << "total: " << n << std::endl;
        }
    }
}

} // namespace Yayo
#endif // UCI_H_
