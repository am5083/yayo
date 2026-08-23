// Verify incremental zobrist key == recomputed key after "position startpos moves ..." + search-like make/unmake
#include "../src/board.hpp"
#include "../src/movegen.hpp"
#include <iostream>
using namespace Yayo;
static int bad = 0, total = 0;
static int pm(Board &board, const std::string &move) {
    moveList mList; generate(board, &mList);
    int fromSq = (move[0] - 'a') + (8 - (move[1] - '0')) * 8;
    int toSq = (move[2] - 'a') + (8 - (move[3] - '0')) * 8;
    for (int i = 0; i < mList.nMoves; i++) { unsigned mv = mList.moves[i].move;
        if (fromSq == getFrom(mv) && toSq == getTo(mv)) return mv; }
    return 0;
}
static void walk(Board &b, int depth) {
    moveList ml; generate(b, &ml);
    for (int i = 0; i < ml.nMoves; i++) {
        make(b, ml.moves[i].move);
        total++;
        if (b.key != b.hash()) { if (bad < 5) std::cout << "MISMATCH after " << b.fen() << "\n"; bad++; }
        if (depth > 1) walk(b, depth - 1);
        unmake(b, ml.moves[i].move);
    }
}
int main() {
    init_arrays(); initMvvLva();
    Board b; std::string fen = START_POS; b.setFen(fen);
    // game moves with ply reset (as UCI 'position ... moves' does)
    const char *game[] = {"e2e4","e7e5","g1f3","b8c6","f1c4","g8f6","e1g1","f8c5","d2d3","d7d6","c1g5","e8g8","b1c3","h7h6","g5h4","a7a6"};
    for (auto m : game) { make(b, pm(b, m)); b.ply = 0; }
    std::cout << "after game moves: key==hash? " << (b.key == b.hash()) << "\n";
    walk(b, 3);
    std::cout << "nodes " << total << " key mismatches " << bad << " -> " << (bad ? "FAIL" : "PASS") << "\n";
    return bad != 0;
}
