// Finite-difference check of the tuner gradient. Pass = max relative error < 1e-3.
#define private public
#include "src/tuner.hpp"
#undef private
#include "src/movegen.hpp"
#include <iostream>
#include <cmath>
using namespace Yayo;
int main(int argc, char **argv) {
    init_arrays(); initMvvLva();
    TunerEntries t(argv[1]);
    double K = 1.0;
    static double params[NUM_FEATURES][2] = {{0}};
    static double grad[NUM_FEATURES][2] = {{0}};
    t.computeGradient(grad, params, K, 0);
    // analytic gradient of error E = mean (r - sigmoid)^2 is -2K/(400 N) * sum (r-S)S(1-S) F * phase
    // the tuner's grad stores sum (r-S)S(1-S)F*phase, so dE/dp = -(2K/400)/N * grad
    int idx[] = {1, 2, 3, 4, 100, 250, 395, 425, 450, 470};
    double maxrel = 0;
    for (int k : idx) for (int pe = 0; pe < 2; pe++) {
        double h = 1e-3;
        params[k][pe] += h; double ep = t.tunedEvalErrors(params, K);
        params[k][pe] -= 2*h; double em = t.tunedEvalErrors(params, K);
        params[k][pe] += h;
        double num = (ep - em) / (2*h);
        double ana = -(2.0 * K / 400.0) / t.numEntries * grad[k][pe];
        double rel = std::abs(num - ana) / std::max(1e-9, std::abs(num));
        maxrel = std::max(maxrel, rel);
        printf("feature %3d %s numeric %+.6e analytic %+.6e rel %.2e\n", k, pe ? "eg" : "mg", num, ana, rel);
    }
    printf("max relative error %.3e -> %s\n", maxrel, maxrel < 1e-3 ? "PASS" : "FAIL");
    return maxrel < 1e-3 ? 0 : 1;
}
