#include <cmath>
#include <iostream>
#include <string>

std::pair<bool, std::pair<double, double>> solveQuadratic(double a, double b, double c)
{
    double discriminant = (std::pow(b, 2)) - (4 * a * c);
    if (discriminant < 0)
    {
        return {false, {0.0, 0.0}};
    }
    if (discriminant == 0)
    {
        double ans = -b / (2 * a);
        return {true, {ans, ans}};
    }
    double first_ans = (-b + std::sqrt(discriminant)) / (2 * a);
    double second_ans = (-b - std::sqrt(discriminant)) / (2 * a);
    return {true, {first_ans, second_ans}};
}
using quadRes = std::pair<bool, std::pair<double, double>>;

int main()
{
    struct Case
    {
        double a, b, c;
        bool real;
        double x1, x2;
        const char *expected;
    };
    Case cases[] = {
        // Case 1: D > 0 (two distinct real roots)
        {1, -3, 2, true, 2, 1, "x1 = 2, x2 = 1"},
        {1, 2, -8, true, 2, -4, "x1 = 2, x2 = -4"},
        {2, -7, 3, true, 3, 0.5, "x1 = 3, x2 = 1/2"},
        // Case 2: D = 0 (one repeated real root)
        {1, -4, 4, true, 2, 2, "x = 2"},
        {1, 6, 9, true, -3, -3, "x = -3"},
        {4, 4, 1, true, -0.5, -0.5, "x = -1/2"},
        // Case 3: D < 0 (complex roots, no real roots) -> only the bool is checked
        {1, 2, 5, false, 0, 0, "x = -1 +/- 2i"},
        {1, 0, 4, false, 0, 0, "x = +/- 2i"},
        {2, 2, 1, false, 0, 0, "x = -1/2 +/- 1/2 i"},
    };

    auto near = [](double x, double y)
    { return std::abs(x - y) < 1e-9; };
    int failed = 0;
    for (const Case &t : cases)
    {
        quadRes res = solveQuadratic(t.a, t.b, t.c);
        double r1 = res.second.first, r2 = res.second.second;
        bool ok = res.first == t.real &&
                  (!t.real || (near(r1, t.x1) && near(r2, t.x2)) || (near(r1, t.x2) && near(r2, t.x1)));
        failed += !ok;
        std::cout << (ok ? "PASS" : "FAIL") << "  a=" << t.a << " b=" << t.b << " c=" << t.c
                  << "  got " << res.first << " (" << r1 << ", " << r2 << ")"
                  << "  expected " << t.expected << "\n";
    }
    return failed ? 1 : 0;
}

// b^2-4ac
