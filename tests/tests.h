#ifndef TESTS_H_
#define TESTS_H_
#include "../board.h"
#include "../bitboard.h"
#include <strings.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

namespace Tests {
typedef struct S_POS {
    char fen[512];
    unsigned long long results[6];
} S_POS;

static S_POS posArray[126];

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
        sscanf(c+3, " %llu;", &d1);
        c = strtok(NULL, ";");
        sscanf(c+3, " %llu;", &d2);
        c = strtok(NULL, ";");
        sscanf(c+3, " %llu;", &d3);
        c = strtok(NULL, ";");
        sscanf(c+3, " %llu;", &d4);
        c = strtok(NULL, ";");
        sscanf(c+3, " %llu;", &d5);
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
        if (!pass) failed = true;
        printf("#%d: %s, %s | %s\n", i, posArray[i].fen, board.fen().c_str(), pass ? "PASSED" : "FAILED");
    }

    printf("\n");
    printf("RESULT: %s\n", failed ? "FAILED FEN TESTING" : "ALL TESTS PASSED");
}
}

/*
**void runPerftSuite() {
    initAllArrays();
    Board *board = NULL;

    int failed = 0;
    parseTestFile("test.epd");
    for (int i = 0; i < 126; i++) {
        Board *board = NULL;
        init_board(&board, posArray[i].fen);
        for (int j = 1; j <= 6; j++) {
            unsigned long long ref = posArray[i].results[j-1];
            unsigned long long test = perft(board, j);
            char *v;
            if (ref == test)
                v = "SUCCESS";
            else {
                v = "FAIL";
                failed++;
            }

            printf("line %d, perft(%d): %llu, ref(%d): %llu, valid? %s\n", i, j, test, j, ref, v);
        }

        printf("\n\n");
        destroy_board(board);
    }

    if (failed) {
        printf("FAILED %d POSITIONS\n", failed);
    } else {
        printf("PASS ALL POSITIONS\n");
    }
}

**
**
**
*/



#endif // TESTS_H_
