#include <iostream>
#include <ginac/ginac.h>

GiNaC::ex RodriguesForumla(const GiNaC::symbol &x, int l)
{
    GiNaC::ex diffed = GiNaC::pow(GiNaC::pow(x, 2) - 1, l);
    return GiNaC::normal(1 / (GiNaC::pow(2, l) * GiNaC::factorial(l)) * GiNaC::diff(diffed, x, l));
}

GiNaC::ex AssociatedLegendreFunction(const GiNaC::symbol &x, int m, int l)
{
    GiNaC::ex diffed = GiNaC::diff(RodriguesForumla(x, l), x, m);
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

GiNaC::ex HydrogrenWaveFunction(const GiNaC::symbol r, const GiNaC::symbol theta, const GiNaC::symbol phi, int n, int l, int m)
{
    GiNaC::symbol tmp("tmp");
    // GiNaC::numeric a = .0000000000529;
    GiNaC::symbol a("a");
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
    GiNaC::symbol x("x");

    std::cout << "Rodrigues Formula:" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::cout << "P_" << i << "(x) == " << RodriguesForumla(x, i) << std::endl;
    }

    std::cout << "Associated Legendre Function:" << std::endl;
    for (int l = 0; l < 3; l++)
    {
        for (int m = 0; m <= l; m++)
        {
            std::cout << "P_" << m << l << "(x) == " << AssociatedLegendreFunction(x, m, l) << std::endl;
            std::cout << "P_" << m << l << "(cos(x)) == " << AssociatedLegendreFunction(x, m, l).subs(x == GiNaC::sin(x)) << std::endl;
        }
    }

    GiNaC::symbol r("r");
    GiNaC::symbol theta("theta");
    GiNaC::symbol phi("phi");

    std::cout << "SphericalHarmonics:" << std::endl;
    for (int l = 0; l < 3; l++)
    {
        for (int m = 0; m <= l; m++)
        {
            std::cout << "Y_" << m << l << "(theta, phi) == " << SphericalHarmonic(theta, phi, m, l) << std::endl;
        }
    }

    GiNaC::exmap simplifyMap;
    simplifyMap[GiNaC::exp(GiNaC::wild(0)) * GiNaC::exp(GiNaC::wild(1))] = GiNaC::exp(GiNaC::wild(0) + GiNaC::wild(1));

    std::cout
        << "Associated Laguerre Polynomials:" << std::endl;
    for (int p = 0; p <= 2; p++)
    {
        for (int q = 0; q <= 2; q++)
        {
            std::cout << "L_" << p << q << "(x) == " << simplify(AssociatedLaguerrePolynomial(x, p, q), simplifyMap) << std::endl;
        }
    }

    std::cout
        << "Hydrogen Wave Function:" << std::endl;
    for (int n = 1; n <= 4; n++)
    {
        for (int l = 0; l < n; l++)
        {
            for (int m = 0; m <= l; m++)
            {
                std::cout << "L_" << n << l << m << "(r, theta, phi) == " << simplify(HydrogrenWaveFunction(r, theta, phi, n, l, m), simplifyMap) << std::endl;
            }
        }
    }

    return 0;
}