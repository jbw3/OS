#include <paging.h>
#include <screen.h>
#include <vmem.h>

namespace os {

static uint32_t* _kPageDir = getKernelPageDirStart();

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr)
 {
     screen << "\n\npage dir location: " << Screen::hex << _kPageDir << "\n";
     screen << Screen::dec;
     screen << "num available PDEs: " << pageInfo::kNumAvailablePageDirEntries() << "\n";

     // need to find next available...

     // mark reserved in page frame manager
     return 0;
 }

}