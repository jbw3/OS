#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

extern "C"
{

const uint32_t* getPageDirStart();

const uint32_t* getPageDirEnd();

void initPageDir();

void enablePaging();

} // extern "C"

#endif // PAGING_H_
