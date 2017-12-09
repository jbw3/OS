#ifndef MEMORY_UINTS_H_
#define MEMORY_UINTS_H_

constexpr unsigned long long int operator ""_KiB(unsigned long long int n)
{
    return n << 10;
}

constexpr unsigned long long int operator ""_MiB(unsigned long long int n)
{
    return n << 20;
}

constexpr unsigned long long int operator ""_GiB(unsigned long long int n)
{
    return n << 30;
}

constexpr unsigned long long int operator ""_TiB(unsigned long long int n)
{
    return n << 40;
}

#endif // MEMORY_UINTS_H_
