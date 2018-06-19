#ifndef _OS_H
#define _OS_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int getNumModules();

void getModuleName(int index, char* name);

#ifdef __cplusplus
} /* extern "C" */
#endif

int runKernelTests(size_t* numTestsPtr = nullptr, size_t* numFailedPtr = nullptr);

#endif /* _OS_H */
