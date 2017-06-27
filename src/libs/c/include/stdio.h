#ifndef _STDIO_H
#define _STDIO_H 1

#define EOF (-1)

#ifdef __cplusplus
extern "C"
{
#endif

typedef __builtin_va_list va_list;

int putchar(int ch);

int printf(const char* fmt, ...);

int vprintf(const char* fmt, va_list args);

int sprintf(char* buff, const char* fmt, ...);

int vsprintf(char* buff, const char* fmt, va_list args);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _STDIO_H */
