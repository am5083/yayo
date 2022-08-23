#include "board.h"
#include "movegen.h"
#include "search.h"
#include <cassert>
#include <cstring>
#include <thread>

using namespace Yayo;
using namespace Yayo::Bitboards;

Bitboard divide(Board &board, int start, int cur) {
    moveList mList = {0};
    generate(board, &mList);
    if (cur == 1) {
        return mList.nMoves;
    }

    Bitboard localNodes = 0;
    for (int i = 0; i < mList.nMoves; i++) {
        make(board, mList.moves[i].move);
        Bitboard thisMoves = divide(board, start, cur - 1);
        localNodes += thisMoves;
        unmake(board, mList.moves[i].move);
        if (cur == start) {
            print_move(mList.moves[i].move);
            std::cout << ": " << thisMoves << "\n";
        }
    }

    return localNodes;
}

uint64_t perft(Board &board, int depth) {
    moveList mList = {0};
    uint64_t nodes = 0;

    generate(board, &mList);
    if (depth == 1)
        return mList.nMoves;

    for (int i = 0; i < mList.nMoves; i++) {
        Board copy(board);
        make(board, mList.moves[i].move);
        nodes += perft(board, depth - 1);
        unmake(board, mList.moves[i].move);
        assert(copy == board);
    }

    return nodes;
}

int main(int argc, char *argv[]) {
    init_arrays();
    Board board;
    board.setFen(START_POS);

    Info info[1];

    while (1) {
        std::string input;
        getline(std::cin, input);

        std::istringstream iss(input);
        std::string cmd;

        iss >> std::skipws >> cmd;

        if (cmd == "isready") {
            std::cout << "readyok\n";
        } else if (cmd == "position") {
            std::string position;

            while (iss >> position) {
                if (position == "startpos") {
                    board.setFen(START_POS);
                } else if (position == "fen") {
                    std::string fen;

                    for (int i = 0; i < 6; i++) {
                        std::string sub;
                        iss >> sub;
                        fen += sub;
                    }

                    board.setFen(fen);
                } else if (position == "moves") {
                    std::string moves;

                    while (iss >> moves) {
                    }
                }
            }
        }
    }
    return 0;
}
