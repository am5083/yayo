#include "thread.h"
#include "uci.h"

using namespace Yayo;

int main(int argc, char *argv[]) {
    std::unique_ptr<Search> searcher(new Search);
    UCI uci(*searcher.get());
    uci.Main();
    return 0;
}
