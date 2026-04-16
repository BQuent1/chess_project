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
    double u = static_cast<double>(std::rand()) / RAND_MAX;
    if (u == 0.0) u = 0.000001; // prevent log(0) error
    if (u == 1.0) u = 0.999999;
    return u;
}

double RandomGen::uniformContinuous(double a, double b) {
    return a + (b - a) * uniformContinuous();
}

int RandomGen::uniformDiscrete(int a, int b) {
    double u = uniformContinuous();
    int range = b - a + 1;
    return a + static_cast<int>(u * range);
}

double RandomGen::exponential(double lambda) {
    // loi exponentielle utilisee pour le temps entre deux evenements (éclairs)
    double u = uniformContinuous();
    return -std::log(u) / lambda;
}

void RandomGen::boxMuller(double& z0, double& z1) { // normal
    double u1 = uniformContinuous();
    double u2 = uniformContinuous();

    double radius = std::sqrt(-2.0 * std::log(u1));
    double theta = 2.0 * M_PI * u2;

    z0 = radius * std::cos(theta);
    z1 = radius * std::sin(theta);
}

double RandomGen::normal(double mu, double sigma) {
    // loi normale utilisee pour representer l'erreur humaine (decalage)
    double z0, z1;
    boxMuller(z0, z1);
    return mu + z0 * sigma;
}

double RandomGen::weibull(double lambda, double k) {
    // loi de weibull utilisee pour representer la duree de vie des pieces
    double u = uniformContinuous();
    // X = lambda * (-ln(U))^(1/k)
    return lambda * std::pow(-std::log(u), 1.0 / k);
}

int RandomGen::poisson(double lambda) {
    // loi de poisson utilisee pour representer le nombre d'evenements dans un intervalle de temps (nbr spectateurs)
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
    // loi binomiale utilisee pour representer le nombre de succes dans un nombre d'essais (nbr pieces enchantees)
    int successes = 0;
    for (int i = 0; i < n; ++i) {
        if (uniformContinuous() <= p) {
            successes++;
        }
    }
    return successes;
}

int RandomGen::geometric(double p) {
    // nb d'echecs avant le premier succes (nbr vrilles)
    double u = uniformContinuous();
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
        int j = uniformDiscrete(0, i);
        std::swap(arr[i], arr[j]);
    }

    return arr;
}
