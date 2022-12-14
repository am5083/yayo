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

#include "uci.hpp"
#include "eval.hpp"

static inline int parseMove(Board &board, std::string move) {
    moveList mList = {0};
    generate(board, &mList);

    int fromSq = (move[0] - 'a') + (8 - (move[1] - '0')) * 8;
    int toSq = (move[2] - 'a') + (8 - (move[3] - '0')) * 8;
    int promPc = 0;

    for (int i = 0; i < mList.nMoves; i++) {
        int mv = mList.moves[i].move;
        if (fromSq == getFrom(mv) && toSq == getTo(mv)) {
            promPc = getCapture(mv);

            if (promPc >= P_KNIGHT) {
                if ((promPc == CP_QUEEN || promPc == P_QUEEN) && move[4] == 'q')
                    return mv;
                else if ((promPc == CP_ROOK || promPc == P_ROOK) &&
                         move[4] == 'r')
                    return mv;
                else if ((promPc == CP_BISHOP || promPc == P_BISHOP) &&
                         move[4] == 'b')
                    return mv;
                else if ((promPc == CP_KNIGHT || promPc == P_KNIGHT) &&
                         move[4] == 'n')
                    return mv;
                continue;
            }
            return mv;
        }
    }
    return encodeMove(Square(fromSq), Square(toSq), MoveFlag(QUIET));
}

Bitboard Yayo::divide(Board &board, moveList *mL, int start, int cur) {
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
            std::cout << ": " << thisMoves << "\n";
        }
    }

    return localNodes;
}

void UCI::Bench() {
    init_arrays();
    initMvvLva();

    Info info[1];

    std::uint64_t start_time = get_time();
    std::uint64_t total_nodes = 0;
    std::uint64_t total_time = 0;

    for (auto &fen : benchPos) {
        search.clearTT(8);
        search._setFen(fen);
        info->timeGiven = false;
        info->depth = 5;
        info->startTime = start_time;
        search.startSearch(info);
        search.wait();
        total_time += get_time() - start_time;
        total_nodes += search.get_nodes();
    }

    search.joinThread();
    total_nodes += search.get_nodes();

    std::uint64_t end_time = get_time();
    total_time /= 1000;

    std::cout << total_nodes << " nodes " << (long)(total_nodes / total_time)
              << " nps" << std::endl;
}

void UCI::Uci() {
    std::cout << "id name Yayo" << std::endl;
    std::cout << "id author kv3732" << std::endl;
    std::cout << std::endl;

    std::cout << "option name Threads type spin default 1 min 1 max 1"
              << std::endl;
    std::cout << "option name Hash type spin default " << TP_INIT_SIZE
              << " min 1 max 1024" << std::endl;
    std::cout << "option name Ponder type check default False" << std::endl;
    std::cout << "uciok" << std::endl;
}

void UCI::NewGame() {
    tt.reset();
    search.clearTT(ttSize);
    search._setFen(START_POS);
}

void UCI::IsReady() { search.isReady(); }

void UCI::Go(Info *info) {
    tt.increaseAge();
    search.startSearch(info);
}

void UCI::Stop() { search.stopSearch(); }

std::uint64_t UCI::Perft(int depth) {
    Board board;
    board.setFen(START_POS);

    unsigned long long n;
    if (depth == 1) {
        moveList mList = {0};
        n = divide(board, &mList, depth, depth);

        for (int i = 0; i < mList.nMoves; i++) {
            print_move(mList.moves[i].move);
            std::cout << ": 1\n";
        }

        std::cout << "\n";
    } else {
        n = divide(board, nullptr, depth, depth);
        std::cout << "\n";
    }

    std::cout << "total: " << n << "\n";

    return n;
}

void UCI::Main() {
    init_arrays();
    initMvvLva();

    Board board;
    board.setFen(START_POS);
    search._setFen(START_POS);

    EvalWeights ev;
    Info info[1];

    std::cout << "Yayo Engine - Verision 0.1.0" << std::endl;
    std::cout << std::endl;

    NewGame();

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
        } else if (cmd == "bench") {
            Bench();
        } else if (cmd == "ucinewgame") {
            NewGame();
        } else if (cmd == "see") {
            std::string move;
            iss >> move;
            int m = parseMove(board, move);
            std::cout << "static exchange evaluation: "
                      << board.see(getTo(m), board.board[getTo(m)], getFrom(m),
                                   board.board[getFrom(m)]);
            std::cout << std::endl;
        } else if (cmd == "mirror") {
            std::string move;
            iss >> move;
            int m = getFrom(parseMove(board, move));
            std::cout << nToSq[mirror(m)] << std::endl;
        } else if (cmd == "d") {
            search.printBoard();
        } else if (cmd == "make") {
            std::string move;
            iss >> move;
            int m = parseMove(board, move);
            search._make(m);
        } else if (cmd == "go") {
            int depth = 256;
            int movestogo = 30;
            int movetime = -1;
            int time = -1;
            int increment = 0;
            bool turn = board.turn;
            info->timeGiven = false;

            std::string tc;

            while (iss >> tc) {
                if (tc == "infinite") {
                    depth = 2000;
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
            info->depth = depth;

            int cStopTime, hStopTime;

            if (time != -1) {
                cStopTime = time / (movestogo + 1) + increment - 5;
                hStopTime =
                      std::min(cStopTime * 5, time / std::min(4, movestogo));

                hStopTime = std::max(10, std::min(hStopTime, time));
                cStopTime = std::max(1, std::min(cStopTime, hStopTime));
                info->timeControl = cStopTime;
                info->maxTimeControl = hStopTime;
                info->stopTime = info->startTime + cStopTime;
                info->timeGiven = true;
            }

            if (movetime != -1) {
                info->stopTime = info->startTime + movetime;
            }

            if (depth == -1) {
                info->depth = 12;
            }

            info->uciStop = false;
            info->uciQuit = false;

            Go(info);
        } else if (cmd == "quit") {
            Stop();
            break;
        } else if (cmd == "stop") {
            Stop();
        } else if (cmd == "trace") {
            Board board = search.getBoard();
            Trace trace;
            int evaluate = Eval<TRACE>(board, trace).eval();
            TracePeek tp(trace, ev);

            int phase = 0;
            int mgPhase = 0;
            int egPhase = 0;

            // clang-format off
            phase = 4 * popcount(board.pieces(QUEEN)) +
                2 * popcount(board.pieces(ROOK)) +
                1 * popcount(board.pieces(BISHOP)) +
                1 * popcount(board.pieces(KNIGHT));
            // clang-format on

            mgPhase = phase;
            if (mgPhase > 24)
                mgPhase = 24;
            egPhase = 24 - mgPhase;

            tp.calculate(std::make_tuple(board.turn, mgPhase, egPhase));
            // tp.print();
            std::cout << "eval score: " << evaluate << std::endl;
        } else if (cmd == "uci") {
            Uci();
        } else if (cmd == "setoption") {
            std::string args;
            iss >> args;

            if (args == "name") {
                iss >> args;

                if (args == "Hash") {
                    iss >> args;

                    if (args == "value") {
                        int size = 0;
                        iss >> size;
                        ttSize = size;
                        NewGame();
                    }
                }
            }

        } else if (cmd == "eval") {
            moveList mList = {0};
            generate(board, &mList);
            std::cout << Eval(board).eval() << std::endl;
        } else if (cmd == "perft") {
            int depth;
            iss >> depth;

            unsigned long long n;
            if (depth == 1) {
                moveList mList = {0};
                n = divide(board, &mList, depth, depth);

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
