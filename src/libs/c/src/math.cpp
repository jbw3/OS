#include <cmath>

extern "C"
{

int isnan_f(float n)
{
    return std::isnan(n);
}

int isnan_d(double n)
{
    return std::isnan(n);
}

int isnan_ld(long double n)
{
    return std::isnan(n);
}

} // extern "C"
