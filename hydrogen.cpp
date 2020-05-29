#include <iostream>
#include <ginac/ginac.h>

const int BUCKET_COUNT = 50;
const int SAMPLE_SIZE = 10;

struct Measurement
{
    double r, theta, phi;
    double p;

    Measurement(double r, double theta, double phi, double p) : r(r), theta(theta), phi(phi), p(p)
    {
    }
};

void PrintJSON(int n, int m, int l, std::vector<Measurement> v)
{
    //
    // export let wavefunctionData = [
    //  { n: 1, m: 1, l: 1, data: [] }
    // }, { n: 2, m: 2}]
    //
    //
    //
    //
    //
    std::printf("{ \"n\": %d, \"l\": %d, \"m\": %d, \"count\": 0, \"data\": [", n, l, m);

    for (int i = 0; i < v.size(); i++)
    {
        Measurement m = v.at(i);
        std::printf("{ \"r\": %f, \"theta\": %f, \"phi\": %f, \"p\": %f}", m.r, m.theta, m.phi, m.p);
        if (i != v.size() - 1)
        {
            std::printf(",");
        }
    }
    std::printf("]}");
}

GiNaC::ex RodriguesFormula(const GiNaC::symbol &x, int l)
{
    GiNaC::ex diffed = GiNaC::pow(GiNaC::pow(x, 2) - 1, l);
    return GiNaC::normal(1 / (GiNaC::pow(2, l) * GiNaC::factorial(l)) * GiNaC::diff(diffed, x, l));
}

GiNaC::ex AssociatedLegendreFunction(const GiNaC::symbol &x, int m, int l)
{

    GiNaC::ex diffed = GiNaC::diff(RodriguesFormula(x, l), x, m);
    return GiNaC::normal(GiNaC::pow(-1, m) * GiNaC::pow(1 - GiNaC::pow(x, 2), GiNaC::numeric(m) / 2) * diffed);
}

GiNaC::ex SphericalHarmonic(const GiNaC::symbol theta, const GiNaC::symbol phi, unsigned int m, unsigned int l)
{
    GiNaC::ex numerator = (2 * l + 1) * GiNaC::factorial(l - m);
    GiNaC::ex denominator = (4 * GiNaC::Pi) * GiNaC::factorial(l + m);

    return GiNaC::normal(GiNaC::sqrt(numerator / denominator) *
                         GiNaC::exp(GiNaC::I * m * phi) * AssociatedLegendreFunction(theta, m, l).subs(theta == GiNaC::cos(theta)));
}

GiNaC::ex AssociatedLaguerrePolynomial(const GiNaC::symbol &x, int p, int q)
{
    GiNaC::ex diffed = GiNaC::diff(GiNaC::exp(-x) * GiNaC::pow(x, p + q), x, q);
    return GiNaC::normal((GiNaC::pow(x, -p) * GiNaC::exp(x) / GiNaC::factorial(q)) * diffed);
}

GiNaC::ex HydrogrenWaveFunction(const GiNaC::symbol r, const GiNaC::symbol theta, const GiNaC::symbol phi, const GiNaC::symbol a, int n, int l, int m)
{
    GiNaC::symbol tmp("tmp");
    GiNaC::ex p1 = GiNaC::sqrt(GiNaC::pow(2 / (n * a), 3) * (GiNaC::factorial(n - l - 1) / (2 * n * GiNaC::factorial(n + l))));
    GiNaC::ex p2 = GiNaC::exp(-r / (n * a));
    GiNaC::ex p3 = GiNaC::pow(2 * r / (n * a), l);
    GiNaC::ex p4 = AssociatedLaguerrePolynomial(tmp, 2l + 1, n - l - 1).subs(tmp == (2 * r / (n * a)));
    GiNaC::ex p5 = SphericalHarmonic(theta, phi, m, l);

    return GiNaC::normal(p1 * p2 * p3 * p4 * p5);
}

// https://www.ginac.de/pipermail/ginac-list/2007-June/001181.html
GiNaC::ex simplify(const GiNaC::ex &x, const GiNaC::exmap &m)
{
    if (GiNaC::is_a<GiNaC::mul>(x))
    {
        GiNaC::ex y = x;
        GiNaC::ex xprev;
        do
        {
            xprev = y;
            y = y.subs(m, GiNaC::subs_options::subs_algebraic).expand();
        } while (xprev != y);
        return y;
    }
    if (GiNaC::is_a<GiNaC::add>(x))
    {
        GiNaC::exvector ev;
        for (size_t i = 0; i < x.nops(); ++i)
            ev.push_back(simplify(x.op(i), m));

        return GiNaC::add(ev);
    }
    return x;
}

int main()
{

    GiNaC::symbol r("r");
    GiNaC::symbol theta("theta");
    GiNaC::symbol phi("phi");

    GiNaC::exmap simplifyMap;
    simplifyMap[GiNaC::exp(GiNaC::wild(0)) * GiNaC::exp(GiNaC::wild(1))] = GiNaC::exp(GiNaC::wild(0) + GiNaC::wild(1));

    GiNaC::symbol a("a");

    std::printf("export let wavefunctionData = [\n");
    bool isFirst = true;
    for (int n = 2; n <= 3; n++)
    {
        for (int l = 0; l < n; l++)
        {
            for (int m = 0; m <= l; m++)
            {
                // std::printf("%d %d %d\n", n, l, m);
                if (isFirst)
                {
                    isFirst = false;
                }
                else
                {
                    std::printf(",\n");
                }
                GiNaC::ex psi = simplify(HydrogrenWaveFunction(r, theta, phi, a, n, l, m), simplifyMap).subs(a == 1);

                std::vector<Measurement> measurements;
                for (int i = 0; i < SAMPLE_SIZE; i++)
                {
                    double randR = ((float)rand() / (float)(RAND_MAX)) * 100;                   // 0 -> 5
                    double randTheta = ((float)rand() / (float)(RAND_MAX)) * M_PI - (M_PI / 2); // -pi -> pi
                    double randPhi = ((float)rand() / (float)(RAND_MAX)) * 2 * M_PI;            // 0->2pi
                    GiNaC::ex probabilityEx = GiNaC::pow(psi.subs(GiNaC::lst{r == randR, theta == randTheta, phi == randPhi}), 2).evalf();
                    double probability = GiNaC::ex_to<GiNaC::numeric>(probabilityEx).to_double();

                    Measurement m = Measurement(randR, randTheta, randPhi, probability);
                    measurements.push_back(m);
                }

                PrintJSON(n, l, m, measurements);
            }
        }
    }
    std::printf("]\n");

    return 0;
}