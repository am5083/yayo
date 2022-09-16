#include "thread.hpp"
#include "uci.hpp"

int main(int argc, char *argv[]) {
    using namespace Yayo;

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
