#include "Sketch.hpp"
#include <cmath>

using MinHash = std::vector<uint64_t>;

double compute_dist(double k, double jaccard)
{
    if (jaccard == 0.0)
        return 1.0;

    return -(1 / k) * std::log(2 * jaccard / (1 + jaccard));
}

double random_occurance(double k, double n)
{
    return n / (n + std::pow(4, k));
}

double compute_r_value(double k, double n)
{
    double upper = random_occurance(k, n) * random_occurance(k, n);
    double lower = random_occurance(k, n) + random_occurance(k, n) - upper;
    return upper / lower;
}

double init_a(double shared_k, double r_value, double r_ratio, double sketch_size)
{
    // this is a_0
    double b = pow(1 - r_value, sketch_size);

    for (int i = 0; i < shared_k; i++)
    {
        b *= (sketch_size - i) / (i + 1) * r_ratio;
    }

    return b;
}

double compute_threshold(double r_value, double sketch_size)
{
    double d = 20;
    double upper = d * r_value * sketch_size - 1 + r_value;
    double lower = d * r_value + 1;
    return std::ceil(upper / lower);
}

double sum_ratio(double i, double r_ratio, double sketch_size)
{
    return (sketch_size - i) / (i + 1) * r_ratio;
}

double approx_p(double a, double threshold, double r_ratio, double shared_k, 
        double sketch_size)
{
    double partial = 0;

    const uint32_t n = shared_k + threshold + 1;
    for (uint32_t i = shared_k; i < n; i++)
    {
        a *= sum_ratio(i, r_ratio, sketch_size);
        partial += a;
    }

    return partial;
}

double compute_p_value(double shared_k, double r_value, double sketch_size)
{
    if (shared_k == 0)
    {
        return 1.0;
    }

    double r_ratio = r_value / (1 - r_value);
    double a = init_a(shared_k, r_value, r_ratio, sketch_size);
    double threshold = std::max(3, (int) compute_threshold(r_value, sketch_size));
    double p_value = approx_p(a, threshold, r_ratio, shared_k, sketch_size);

    return p_value;
}

double shared_kmers(MinHash& m1, MinHash& m2)
{
    uint32_t shared = 0;
    uint32_t unique = 0;

    for (size_t i = 0, j = 0; unique < m1.size() && i != m1.size() && j != m1.size(); )
    {
        if (m1[i] == m2[j])
        {
            shared++;
            i++;
            j++;
        }
        else if (m1[i] < m2[j])
        {
            i++;
        }
        else if (m1[i] > m2[j])
        {
            j++;
        }

        unique++;
    }

    return shared;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        std::cout << "Usage: " << argv[0] << " <sketch1> <sketch2>\n";
        exit(1);
    }

    Sketch s1 = Sketch::read(argv[1]);
    Sketch s2 = Sketch::read(argv[2]);

    if (s1.k != s2.k)
    {
        std::cerr << "sketches use different kmer sizes (-k)\n";
        exit(1);
    }

    if (s1.c != s2.c)
    {
        std::cerr << "sketches use different thresholds (-c)\n";
        exit(1);
    }

    if (s1.s != s2.s)
    {
        std::cerr << "sketches have different sizes (-s)\n";
        exit(1);
    }

    double shared_k = shared_kmers(s1.min_hash, s2.min_hash);
    double jaccard = shared_k / (double) s1.s;
    double dist = compute_dist(s1.k, jaccard);
    double r = compute_r_value(s1.k, s1.s);
    double p = compute_p_value(shared_k, s1.s, r);

    std::cout << argv[1] << "\t";
    std::cout << argv[2] << "\t";
    std::cout << dist << "\t";
    std::cout << p << "\t";
    std::cout << shared_k << "/" << s1.s << "\t";
    std::cout << s1.k << "\t";
    std::cout << s1.s << "\n";
}
