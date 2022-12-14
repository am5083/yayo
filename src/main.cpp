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

#include "thread.hpp"
#include "tuner.hpp"
#include "uci.hpp"

using namespace Yayo;

int main(int argc, char *argv[]) {
    std::unique_ptr<Search> searcher(new Search);
    UCI uci(*searcher.get());

    if (argc == 2) {
        if (strcmp(argv[1], "bench") == 0) {
            uci.Bench();
            return 0;
        } else if (strcmp(argv[1], "tune") == 0) {
            init_arrays();
            initMvvLva();
            TunerEntries tuner("selfplay.pgn");
            tuner.runTuner();
            return 0;
        } else if (strcmp(argv[1], "perft") == 0) {
            init_arrays();
            initMvvLva();

            std::uint64_t start_time = get_time();
            std::uint64_t total_nodes = uci.Perft(7);
            std::uint64_t end_time = get_time();
            long double total_time =
                  1.0 * double(end_time - start_time) / 1000.0;

            std::uint64_t nps = (long)(total_nodes / total_time);
            std::cout << "nps: " << nps << std::endl;

            return 0;
        }
    }

    uci.Main();
    return 0;
}
