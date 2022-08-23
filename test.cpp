#include "board.h"
#include "movegen.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

using namespace Yayo;
using namespace Yayo::Bitboards;

typedef struct S_POS {
    char fen[512];
    unsigned long long results[6];
} S_POS;

static S_POS posArray[126];

static inline uint64_t perft(Board &board, int depth) {
    moveList mList = {0};
    uint64_t nodes = 0;

    generate(board, &mList);
    if (depth == 1)
        return mList.nMoves;

    for (int i = 0; i < mList.nMoves; i++) {
        make(board, mList.moves[i].move);
        nodes += perft(board, depth - 1);
        unmake(board, mList.moves[i].move);
    }

    return nodes;
}

static inline void parseTestFile(const char *file) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t size;

    fp = fopen(file, "r");
    if (fp == NULL) {
        exit(-1);
    }

    int i = 0;
    while ((size = getline(&line, &len, fp)) != -1 && i < 126) {
        char fen[512], ln[512];
        unsigned long long d1, d2, d3, d4, d5, d6;
        strcpy(ln, line);
        char *c = strtok(ln, ";");
        strcpy(fen, c);
        c = strtok(NULL, ";");
        sscanf(c + 3, " %llu;", &d1);
        c = strtok(NULL, ";");
        sscanf(c + 3, " %llu;", &d2);
        c = strtok(NULL, ";");
        sscanf(c + 3, " %llu;", &d3);
        c = strtok(NULL, ";");
        sscanf(c + 3, " %llu;", &d4);
        c = strtok(NULL, ";");
        sscanf(c + 3, " %llu;", &d5);
        c = strtok(NULL, " ");
        c = strtok(NULL, " ");

        if (c != NULL) {
            sscanf(c, " %llu;", &d6);
        } else {
            d6 = 0;
        }

        strcpy(posArray[i].fen, fen);
        posArray[i].results[0] = d1;
        posArray[i].results[1] = d2;
        posArray[i].results[2] = d3;
        posArray[i].results[3] = d4;
        posArray[i].results[4] = d5;
        posArray[i].results[5] = d6;
        i++;
    }
}

void runFenSuite() {
    Yayo::Bitboards::init_arrays();
    Yayo::Board board;

    bool failed = false;
    for (int i = 0; i < 126; i++) {
        board.setFen(posArray[i].fen);
        bool pass = strcmp(posArray[i].fen, board.fen().c_str()) > 0;
        if (!pass)
            failed = true;
        printf("#%d: %s, %s | %s\n", i, posArray[i].fen, board.fen().c_str(), pass ? "PASSED" : "FAILED");
    }

    printf("\n");
    printf("RESULT: %s\n", failed ? "FAILED FEN TESTING" : "ALL TESTS PASSED");
}

/*
**
** r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1
** 1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1
** B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1
*/

void runPerftSuite(int depth) {
    init_arrays();
    Board board;

    int failed = 0;
    std::uint64_t total = 0;
    std::uint64_t positions = 0;
    parseTestFile("test.epd");
    for (int i = 0; i < 126; i++) {
        board.setFen(posArray[i].fen);
        std::cout << "FEN: " << posArray[i].fen << std::endl;
        for (int j = 1; j <= depth; j++) {
            unsigned long long ref = posArray[i].results[j - 1];
            unsigned long long test = perft(board, j);
            if (j < 6) {
                positions += ref;
            }
            total += ref;
            std::string v;
            if (ref == test)
                v = "SUCCESS";
            else {
                v = "FAIL";
                failed++;
            }

            printf("line %d, perft(%d): %llu, ref(%d): %llu, valid? %s\n", i, j, test, j, ref, v.c_str());
        }

        printf("\n\n");
    }

    std::cout << "\nTESTED " << total << " MOVES ACROSS " << positions << " POSITIONS\n";

    if (failed) {
        printf("FAILED %d TEST CASES OUT OF 126\n", failed);
    } else {
        printf("PASS ALL POSITIONS\n");
    }
}

int main(int argc, char *argv[]) { runPerftSuite(atoi(argv[1])); }
