#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

int writeSignedNum(long long num, char* str, int base, int uppercase);

int writeUnsignedNum(unsigned long long num, char* str, int base, int uppercase);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _STRINGUTILS_H */
