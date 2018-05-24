#ifndef _OS_H
#define _OS_H 1

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

#endif /* _OS_H */
