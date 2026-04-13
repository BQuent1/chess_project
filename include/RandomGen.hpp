#ifndef RANDOMGEN_HPP
#define RANDOMGEN_HPP

#include <vector>

class RandomGen {
public:
    // Initialise the random seed
    static void init();

    // Base uniform distribution U(0,1)
    static double uniformContinuous();

    // Uniform distribution U(a,b)
    static double uniformContinuous(double a, double b);

    // Discrete uniform distribution between a and b (inclusive)
    static int uniformDiscrete(int a, int b);

    // Exponential distribution
    static double exponential(double lambda);

    // Normal distribution (Box-Muller)
    // Returns a pair of independent standard normal variables N(0,1)
    static void boxMuller(double& z0, double& z1);
    
    // Normal distribution N(mu, sigma^2)
    static double normal(double mu, double sigma);

    // Weibull distribution W(lambda, k)
    static double weibull(double lambda, double k);

    // Poisson distribution P(lambda)
    static int poisson(double lambda);

    // Binomial distribution B(n, p)
    static int binomial(int n, double p);

    // Geometric distribution G(p)
    static int geometric(double p);

    // Fisher-Yates Random Permutation
    // Shuffles an array of indices from 0 to n-1 and returns it
    static std::vector<int> randomPermutation(int n);
};

#endif // RANDOMGEN_HPP
