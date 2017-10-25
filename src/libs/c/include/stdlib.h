#ifndef _STDLIB_H
#define _STDLIB_H 1

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int abs(int n);

long labs(long n);

long long llabs(long long n);

int atoi(const char* str);

void exit(int status);

long strtol(const char* str, char** strEnd, int base);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _STDLIB_H */
