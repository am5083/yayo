#ifndef UCI_H_
#define UCI_H_
#include "board.h"
#include "eval.h"
#include "move.h"
#include "movegen.h"
#include "thread.h"
#include "util.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace Yayo;
using namespace Yayo::Bitboards;

std::string benchPos[] = {
    "r3k2r/2pb1ppp/2pp1q2/p7/1nP1B3/1P2P3/P2N1PPP/R2QK2R w KQkq a6 0 14   ",
    "4rrk1/2p1b1p1/p1p3q1/4p3/2P2n1p/1P1NR2P/PB3PP1/3R1QK1 b - - 2 24     ",
    "r3qbrk/6p1/2b2pPp/p3pP1Q/PpPpP2P/3P1B2/2PB3K/R5R1 w - - 16 42        ",
    "6k1/1R3p2/6p1/2Bp3p/3P2q1/P7/1P2rQ1K/5R2 b - - 4 44  ",
    "8/8/1p2k1p1/3p3p/1p1P1P1P/1P2PK2/8/8 w - - 3 54      ",
    "7r/2p3k1/1p1p1qp1/1P1Bp3/p1P2r1P/P7/4R3/Q4RK1 w - - 0 36     ",
    "r1bq1rk1/pp2b1pp/n1pp1n2/3P1p2/2P1p3/2N1P2N/PP2BPPP/R1BQ1RK1 b - - 2 10      ",
    "3r3k/2r4p/1p1b3q/p4P2/P2Pp3/1B2P3/3BQ1RP/6K1 w - - 3 87      ",
    "2r4r/1p4k1/1Pnp4/3Qb1pq/8/4BpPp/5P2/2RR1BK1 w - - 0 42       ",
    "4q1bk/6b1/7p/p1p4p/PNPpP2P/KN4P1/3Q4/4R3 b - - 0 37  ",
    "2q3r1/1r2pk2/pp3pp1/2pP3p/P1Pb1BbP/1P4Q1/R3NPP1/4R1K1 w - - 2 34     ",
    "1r2r2k/1b4q1/pp5p/2pPp1p1/P3Pn2/1P1B1Q1P/2R3P1/4BR1K b - - 1 37      ",
    "r3kbbr/pp1n1p1P/3ppnp1/q5N1/1P1pP3/P1N1B3/2P1QP2/R3KB1R b KQkq b3 0 17       ",
    "8/6pk/2b1Rp2/3r4/1R1B2PP/P5K1/8/2r5 b - - 16 42      ",
    "1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - - 8 29        ",
    "8/p2B4/PkP5/4p1pK/4Pb1p/5P2/8/8 w - - 29 68  ",
    "3r4/ppq1ppkp/4bnp1/2pN4/2P1P3/1P4P1/PQ3PBP/R4K2 b - - 2 20   ",
    "5rr1/4n2k/4q2P/P1P2n2/3B1p2/4pP2/2N1P3/1RR1K2Q w - - 1 49    ",
    "1r5k/2pq2p1/3p3p/p1pP4/4QP2/PP1R3P/6PK/8 w - - 1 51  ",
    "q5k1/5ppp/1r3bn1/1B6/P1N2P2/BQ2P1P1/5K1P/8 b - - 2 34        ",
    "r1b2k1r/5n2/p4q2/1ppn1Pp1/3pp1p1/NP2P3/P1PPBK2/1RQN2R1 w - - 0 22    ",
    "r1bqk2r/pppp1ppp/5n2/4b3/4P3/P1N5/1PP2PPP/R1BQKB1R w KQkq - 0 5      ",
    "r1bqr1k1/pp1p1ppp/2p5/8/3N1Q2/P2BB3/1PP2PPP/R3K2n b Q - 1 12 ",
    "r1bq2k1/p4r1p/1pp2pp1/3p4/1P1B3Q/P2B1N2/2P3PP/4R1K1 b - - 2 19       ",
    "r4qk1/6r1/1p4p1/2ppBbN1/1p5Q/P7/2P3PP/5RK1 w - - 2 25        ",
    "r7/6k1/1p6/2pp1p2/7Q/8/p1P2K1P/8 w - - 0 32  ",
    "r3k2r/ppp1pp1p/2nqb1pn/3p4/4P3/2PP4/PP1NBPPP/R2QK1NR w KQkq - 1 5    ",
    "3r1rk1/1pp1pn1p/p1n1q1p1/3p4/Q3P3/2P5/PP1NBPPP/4RRK1 w - - 0 12      ",
    "5rk1/1pp1pn1p/p3Brp1/8/1n6/5N2/PP3PPP/2R2RK1 w - - 2 20      ",
    "8/1p2pk1p/p1p1r1p1/3n4/8/5R2/PP3PPP/4R1K1 b - - 3 27 ",
    "8/4pk2/1p1r2p1/p1p4p/Pn5P/3R4/1P3PP1/4RK2 w - - 1 33 ",
    "8/5k2/1pnrp1p1/p1p4p/P6P/4R1PK/1P3P2/4R3 b - - 1 38  ",
    "8/8/1p1kp1p1/p1pr1n1p/P6P/1R4P1/1P3PK1/1R6 b - - 15 45       ",
    "8/8/1p1k2p1/p1prp2p/P2n3P/6P1/1P1R1PK1/4R3 b - - 5 49        ",
    "8/8/1p4p1/p1p2k1p/P2npP1P/4K1P1/1P6/3R4 w - - 6 54   ",
    "8/8/1p4p1/p1p2k1p/P2n1P1P/4K1P1/1P6/6R1 b - - 6 59   ",
    "8/5k2/1p4p1/p1pK3p/P2n1P1P/6P1/1P6/4R3 b - - 14 63   ",
    "8/1R6/1p1K1kp1/p6p/P1p2P1P/6P1/1Pn5/8 w - - 0 67     ",
    "1rb1rn1k/p3q1bp/2p3p1/2p1p3/2P1P2N/PP1RQNP1/1B3P2/4R1K1 b - - 4 23   ",
    "4rrk1/pp1n1pp1/q5p1/P1pP4/2n3P1/7P/1P3PB1/R1BQ1RK1 w - - 3 22        ",
    "r2qr1k1/pb1nbppp/1pn1p3/2ppP3/3P4/2PB1NN1/PP3PPP/R1BQR1K1 w - - 4 12 ",
    "2r2k2/8/4P1R1/1p6/8/P4K1N/7b/2B5 b - - 0 55  ",
    "6k1/5pp1/8/2bKP2P/2P5/p4PNb/B7/8 b - - 1 44  ",
    "2rqr1k1/1p3p1p/p2p2p1/P1nPb3/2B1P3/5P2/1PQ2NPP/R1R4K w - - 3 25      ",
    "r1b2rk1/p1q1ppbp/6p1/2Q5/8/4BP2/PPP3PP/2KR1B1R b - - 2 14    ",
    "6r1/5k2/p1b1r2p/1pB1p1p1/1Pp3PP/2P1R1K1/2P2P2/3R4 w - - 1 36 ",
    "rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2        ",
    "2rr2k1/1p4bp/p1q1p1p1/4Pp1n/2PB4/1PN3P1/P3Q2P/2RR2K1 w - f6 0 20     ",
    "3br1k1/p1pn3p/1p3n2/5pNq/2P1p3/1PN3PP/P2Q1PB1/4R1K1 w - - 0 23       ",
    "2r2b2/5p2/5k2/p1r1pP2/P2pB3/1P3P2/K1P3R1/7R w - - 23 93",
};

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
    void Bench();

  private:
    Search &search;
};

void UCI::Bench() {
    Info info[1];

    std::uint64_t start_time  = get_time();
    std::uint64_t total_nodes = 0;

    for (auto &fen : benchPos) {
        std::unique_ptr<Search> searcher(new Search);
        searcher->_setFen(fen);

        info->timeGiven = false;
        info->depth     = 7;
        info->startTime = start_time;
        searcher->startSearch(info);

        total_nodes += searcher->get_nodes();
        searcher->joinThread();

        delete searcher.get();
    }

    search.joinThread();
    total_nodes += search.get_nodes();

    std::uint64_t end_time = get_time();
    long double total_time = 1.0 * (end_time - start_time) / 1000.0;

    std::cout << total_nodes << " nodes " << int(total_nodes / total_time) << " nps" << std::endl;
}

void UCI::Uci() {
    std::cout << "id name Yayo" << std::endl;
    std::cout << "id author kv3732" << std::endl;
    std::cout << std::endl;

    // std::cout << "option name Threads type spin default 1 min 1 max 1" << std::endl;
    // std::cout << "option name Hash type spin default 400 min 400 max 1024" << std::endl;
    // std::cout << "option name Ponder type check default False" << std::endl;
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

    std::cout << "Yayo Engine - Verision 0.1.0" << std::endl;
    std::cout << std::endl;

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
