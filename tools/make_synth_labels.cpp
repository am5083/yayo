// Build a synthetic labeled dataset for tuner tests.
// Label = Bernoulli(sigmoid((eval + 60*bishop diff - 40*knight diff + noise)/150)).
// The tuner must move bishop value up and knight value down.
#include "../src/board.hpp"
#include "../src/movegen.hpp"
#include "../src/eval.hpp"
#include <fstream>
#include <iostream>
#include <random>
using namespace Yayo;
int main(int argc, char **argv) {
    init_arrays(); initMvvLva();
    std::ifstream in(argv[1]); std::ofstream out(argv[2]);
    int limit = atoi(argv[3]);
    std::mt19937 rng(12345); std::normal_distribution<double> noise(0, 80);
    std::uniform_real_distribution<double> u(0, 1);
    std::string line; int n = 0;
    while (n < limit && getline(in, line)) {
        std::string fen = line.substr(0, line.find(" ;"));
        if (fen.find_first_of("0123456789", fen.rfind(' ')) == std::string::npos) fen += " 0 1";
        Board b; b.setFen(fen);
        int e = Eval<NO_TRACE>(b).eval(); if (b.turn == BLACK) e = -e;
        int bd = popcount(b.pieces(BISHOP, WHITE)) - popcount(b.pieces(BISHOP, BLACK));
        int nd = popcount(b.pieces(KNIGHT, WHITE)) - popcount(b.pieces(KNIGHT, BLACK));
        double t = (e + 60 * bd - 40 * nd + noise(rng)) / 150.0;
        double p = 1.0 / (1.0 + exp(-t));
        double r = u(rng) < p ? 1.0 : 0.0;
        if (u(rng) < 0.15) r = 0.5;
        out << fen << " [" << (r == 1.0 ? "1.0" : r == 0.5 ? "0.5" : "0.0") << "]\n";
        n++;
    }
    std::cout << "wrote " << n << " labeled positions\n";
}
