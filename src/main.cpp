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
        }
    }

    uci.Main();
    return 0;
}
