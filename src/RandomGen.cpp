#include "RandomGen.hpp"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void RandomGen::init() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

double RandomGen::uniformContinuous() {
    // Generate uniform random in [0, 1)
    // Avoid exactly 1.0 or 0.0 heavily if generating logs, but here we keep it safe:
    double u = static_cast<double>(std::rand()) / RAND_MAX;
    if (u == 0.0) u = 0.000001; // prevent log(0) error
    if (u == 1.0) u = 0.999999;
    return u;
}

double RandomGen::uniformContinuous(double a, double b) {
    return a + (b - a) * uniformContinuous();
}

int RandomGen::uniformDiscrete(int a, int b) {
    // We want a result between a and b inclusive
    double u = uniformContinuous();
    int range = b - a + 1;
    return a + static_cast<int>(u * range);
}

double RandomGen::exponential(double lambda) {
    double u = uniformContinuous();
    return -std::log(u) / lambda;
}

void RandomGen::boxMuller(double& z0, double& z1) {
    double u1 = uniformContinuous();
    double u2 = uniformContinuous();

    double radius = std::sqrt(-2.0 * std::log(u1));
    double theta = 2.0 * M_PI * u2;

    z0 = radius * std::cos(theta);
    z1 = radius * std::sin(theta);
}

double RandomGen::normal(double mu, double sigma) {
    double z0, z1;
    boxMuller(z0, z1); // Box-Muller gives N(0,1)
    return mu + z0 * sigma;
}

double RandomGen::weibull(double lambda, double k) {
    double u = uniformContinuous();
    // X = lambda * (-ln(U))^(1/k)
    return lambda * std::pow(-std::log(u), 1.0 / k);
}

int RandomGen::poisson(double lambda) {
    // Product method
    double L = std::exp(-lambda);
    double p = 1.0;
    int k = 0;

    do {
        k++;
        p *= uniformContinuous();
    } while (p > L);

    return k - 1;
}

int RandomGen::binomial(int n, double p) {
    int successes = 0;
    for (int i = 0; i < n; ++i) {
        if (uniformContinuous() <= p) {
            successes++;
        }
    }
    return successes;
}

int RandomGen::geometric(double p) {
    // Number of failures before the first success
    double u = uniformContinuous();
    // floor( ln(U) / ln(1 - p) )
    double failures = std::floor(std::log(u) / std::log(1.0 - p));
    return static_cast<int>(failures);
}

std::vector<int> RandomGen::randomPermutation(int n) {
    std::vector<int> arr(n);
    for (int i = 0; i < n; ++i) {
        arr[i] = i;
    }

    // Fisher-Yates shuffle
    for (int i = n - 1; i > 0; --i) {
        // Find index between 0 and i inclusive
        int j = uniformDiscrete(0, i);
        // Swap
        std::swap(arr[i], arr[j]);
    }

    return arr;
}
