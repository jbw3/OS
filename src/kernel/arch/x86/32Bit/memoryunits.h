#ifndef MEMORY_UINTS_H_
#define MEMORY_UINTS_H_

unsigned long long int operator ""_KiB(unsigned long long int n)
{
    return n << 10;
}

unsigned long long int operator ""_MiB(unsigned long long int n)
{
    return n << 20;
}

unsigned long long int operator ""_GiB(unsigned long long int n)
{
    return n << 30;
}

unsigned long long int operator ""_TiB(unsigned long long int n)
{
    return n << 40;
}

#endif // MEMORY_UINTS_H_
