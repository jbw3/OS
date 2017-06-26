#ifndef _STDIO_H
#define _STDIO_H 1

#define EOF (-1)

#ifdef __cplusplus
extern "C"
{
#endif

typedef __builtin_va_list va_list;

int sprintf(char* buff, const char* fmt, ...);

int vsprintf(char* buff, const char* fmd, va_list args);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _STDIO_H */
