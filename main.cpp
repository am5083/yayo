#include "thread.h"
#include "uci.h"

int main(int argc, char *argv[]) {
    using namespace Yayo;

    std::unique_ptr<Search> searcher(new Search);
    UCI uci(*searcher.get());
    uci.Main();

    return 0;
}
