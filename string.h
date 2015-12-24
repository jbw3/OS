#ifndef STRING_H_
#define STRING_H_

#define NULL ((void*)0)

typedef __SIZE_TYPE__ size_t;

void* memcpy(void* dst, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
size_t strlen(const char* str);

#endif // STRING_H_
