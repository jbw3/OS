#ifndef STRING_H_
#define STRING_H_

#define NULL ((void*)0)

typedef __SIZE_TYPE__ size_t;

#ifdef __cplusplus
extern "C"
{
#endif

void* memcpy(void* dst, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
size_t strlen(const char* str);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STRING_H_
