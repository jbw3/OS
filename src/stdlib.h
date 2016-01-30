#ifndef STDLIB_H_
#define STDLIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

int abs(int n);

int atoi(const char* str);

long labs(long n);

long long llabs(long long n);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STDLIB_H_
