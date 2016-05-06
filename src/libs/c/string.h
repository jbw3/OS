#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

int memcmp(const void* ptr1, const void* ptr2, size_t num);

void* memcpy(void* dst, const void* src, size_t num);

void* memmove(void* dst, const void* src, size_t num);

void* memset(void* ptr, int value, size_t num);

char* strcat(char* str1, const char* str2);

char* strncat(char* str1, const char* str2, size_t num);

char* strchr(const char* str, int ch);

int strcmp(const char* str1, const char* str2);

char* strcpy(char* dst, const char* src);

size_t strlen(const char* str);

char* strtok(char* str, const char* delimiters);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STRING_H_ */
