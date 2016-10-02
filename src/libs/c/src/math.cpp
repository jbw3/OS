#include <math.h>

extern "C"
{

int isnan_f(float n)
{
    return n != n;
}

int isnan_d(double n)
{
    return n != n;
}

int isnan_ld(long double n)
{
    return n != n;
}

} // extern "C"
