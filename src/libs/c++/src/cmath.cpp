#include <cmath>

namespace std
{

bool isnan(float n)
{
    return n != n;
}

bool isnan(double n)
{
    return n != n;
}

bool isnan(long double n)
{
    return n != n;
}

} // namespace std
