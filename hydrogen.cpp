#include <iostream>
#include <ginac/ginac.h>

const int SAMPLE_SIZE = 5000;
const int WAVEFUNCTION_COUNT = 20;

struct Measurement
{
    double r, theta, phi;
    double p;

    Measurement(double r, double theta, double phi, double p) : r(r), theta(theta), phi(phi), p(p)
    {
    }
};

bool compareByProbability(const Measurement &a, const Measurement &b)
{
    return a.p > b.p;
}

double averageProbability(std::vector<Measurement> v)
{
    double sum = 0;
    for (Measurement m : v)
    {
        sum += m.p;
    }
    return sum / double(v.size());
}

void normalize(std::vector<Measurement> &v)
{
    double average = averageProbability(v);
    for (int i = 0; i < v.size(); i++)
    {
        v[i].p /= (average * 100);
    }
}

void PrintJSON(int n, int l, int m, std::vector<Measurement> v)
{
    //
    // export let wavefunctionData = [
    //  { n: 1, m: 1, l: 1, p: [.1,.2], theta: [.1,.2], phi: [.2.3.4] }
    // }, { n: 2, m: 2}]
    //
    //
    //
    //
    //
    double avgP = averageProbability(v);

    std::printf("{ \"n\": %d, \"l\": %d, \"m\": %d, \"p\": [", n, l, m);

    // print probabilities
    for (int i = 0; i < v.size(); i++)
    {
        if (i != 0)
            printf(",");

        Measurement m = v.at(i);
        std::printf("%f", m.p);
    }

    // print probabilities
    std::printf("],\n \"r\": [");
    for (int i = 0; i < v.size(); i++)
    {
        if (i != 0)
            printf(",");

        Measurement m = v.at(i);
        std::printf("%f", m.r);
    }

    // print probabilities
    std::printf("],\n \"theta\": [");
    for (int i = 0; i < v.size(); i++)
    {
        if (i != 0)
            printf(",");

        Measurement m = v.at(i);
        std::printf("%f", m.theta);
    }

    std::printf("],\n \"phi\": [");
    for (int i = 0; i < v.size(); i++)
    {
        if (i != 0)
            printf(",");

        Measurement m = v.at(i);
        std::printf("%f", m.phi);
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
    int count = 0;

    for (int n = 2; n <= 10; n++)
    {
        for (int l = 0; l < n; l++)
        {
            for (int m = 0; m <= l; m++)
            {
                if (count >= WAVEFUNCTION_COUNT)
                {
                    goto end;
                }
                // std::printf("%d %d %d\n", n, l, m);
                if (count != 0)
                {
                    std::printf(",\n");
                }
                GiNaC::ex psi = simplify(HydrogrenWaveFunction(r, theta, phi, a, n, l, m), simplifyMap).subs(a == 1);

                std::vector<Measurement> measurements;
                while (measurements.size() < SAMPLE_SIZE)
                {
                    double randR = ((float)rand() / (float)(RAND_MAX)) * 70;                    // 0 -> 5
                    double randTheta = ((float)rand() / (float)(RAND_MAX)) * M_PI - (M_PI / 2); // -pi -> pi
                    double randPhi = ((float)rand() / (float)(RAND_MAX)) * 2 * M_PI;            // 0->2pi
                    GiNaC::ex probabilityEx = GiNaC::pow(psi.subs(GiNaC::lst{r == randR, theta == randTheta, phi == randPhi}), 2).evalf();
                    double probability = GiNaC::ex_to<GiNaC::numeric>(probabilityEx).to_double();

                    if (probability < .000001)
                    {
                        continue;
                    }

                    Measurement m = Measurement(randR, randTheta, randPhi, probability);
                    measurements.push_back(m);
                }

                std::sort(measurements.begin(), measurements.end(), compareByProbability);
                normalize(measurements);
                PrintJSON(n, l, m, measurements);
                count++;
            }
        }
    }
end:
    std::printf("]\n");

    return 0;
}