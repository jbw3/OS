#include <pageframemgr.h>
#include <pagetable.h>
#include <paging.h>
#include <screen.h>
#include <system.h>

namespace mem {

struct PageTablePointer
{
    /**
     * @brief The index into the kernel page directory
     * that corresponds to this page table
     */
    uint16_t kPageDirIdx;

    /**
     * @brief A pointer to the page table
     */
    uint32_t* pageTable;
};

static bool myFlag = false;
static PageTable __ptPageTable;

// 1K pages/pt * 4K/page = 4M/pt
const int NUM_PAGE_TABLES = 256;    // 256 PTs map high 1GB of memory

// array of page table pointers. Page tables are mapped
// into the array based on pageDirIndex, offset from
// the kernel's starting page dir index (768)
static PageTablePointer __pageTablePtrs[NUM_PAGE_TABLES];

PageTable::PageTable()
    : _pageDir(nullptr),
    _pageDirIdx(0),
    _pageTable(nullptr)
{
}

PageTable::PageTable(uint32_t* pageDir, uint16_t pageDirIdx)
    : _pageDir(pageDir),
    _pageDirIdx(pageDirIdx)
{
    setPageTablePointer();

    if (pageDirIdx == 0x301 && !myFlag)
    {
        myFlag = true;
        screen << "0x301 page table pointer (phys): 0x" << (uint32_t)(pageDir[pageDirIdx] & PAGE_DIR_ADDRESS) << "\n";
        screen << "0x301 page table pointer (virt): 0x" << _pageTable << "\n";
        uint32_t ptPtrVal = (uint32_t)_pageTable;
        uint32_t idx = (ptPtrVal & 0xFFC0'0000);
        idx >>= 22;
        uint32_t ptAddr = (uint32_t)pageDir[idx];
        uint32_t ptVirtAddr = ptAddr + KERNEL_VIRTUAL_BASE;
        uint32_t* ptPtr = (uint32_t*)ptVirtAddr;
        uint32_t idx2 = (ptPtrVal & 0x003F'F000);
        idx2 >>= 12;
        //screen << "pt's page table addr (phys): 0x" << ptAddr << "\n";
        //screen << "pt's page table addr (virt): 0x" << ptVirtAddr << "\n";
        //screen << "pt's page frame addr (phys): 0x" << ptPtr[idx2] << "\n";
    }
}

// private PTPageTable constructor
PageTable::PageTable(uint32_t* ptPageTable, uint32_t* pageDir, uint16_t pageDirIdx)
    : _pageDir(pageDir),
    _pageDirIdx(pageDirIdx),
    _pageTable(ptPageTable)
{
}

void PageTable::initPTPageTable(uint32_t* ptPageTable, uint32_t* pageDir, uint16_t pageDirIdx)
{
    // go ahead and set up kernel page table pointer here
    __pageTablePtrs[0].kPageDirIdx = KERNEL_PAGEDIR_START_IDX;
    __pageTablePtrs[0].pageTable = getKernelPageDirStart();

    __ptPageTable = PageTable(ptPageTable, pageDir, pageDirIdx);
    __ptPageTable.initPageTable();      // this sets up PTPT in pt pointer array
}

PageTable* PageTable::getPTPageTable()
{
    return &__ptPageTable;
}

uint32_t* PageTable::getPointer()
{
    return _pageTable;
}

uint32_t* PageTable::getPageDirPointer()
{
    return _pageDir;
}

uint16_t PageTable::getPageDirIndex()
{
    return _pageDirIdx;
}

bool PageTable::isFull()
{
    // cls: start at end for speed - loop will execute 1x
    // for page tables filled from front to back

    for (int16_t i = PAGE_TABLE_NUM_ENTRIES-1; i >= 0; i--)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return false;   // found an available entry
        }
    }
    return true;    // each entry was used
}

bool PageTable::isEmpty()
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            return false;   // found an active entry
        }
    }
    return true;    // each entry was inactive
}

void PageTable::initPageTable()
{
    for (int i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        _pageTable[i] &= ~PAGE_TABLE_PRESENT;
    }

    // init page table pointer
    auto idx = ptArrayIndex();
    __pageTablePtrs[idx].kPageDirIdx = _pageDirIdx;
    __pageTablePtrs[idx].pageTable = _pageTable;
}

uint16_t PageTable::nextAvailablePage()
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return i;   // this entry is available
        }
    }
    return PAGE_TABLE_NUM_ENTRIES;  // invalid index
}

uint32_t PageTable::mapNextAvailablePageToAddress(uint32_t physAddr)
{
    uint16_t pageIdx = nextAvailablePage(); // get next available page index
    uint16_t offset = physAddr & 0xFFF;     // 12 bits offset
    uint32_t pageAddress = virtAddressOfPage(pageIdx);
    mapPage(_pageDir, pageAddress, physAddr);
    PageFrameMgr::get()->reservePageFrame(physAddr);    // mark as reserved for page frame manager
    return pageAddress + offset;    // return virt address now mapped to physAddr
}

uint32_t PageTable::virtAddressOfPage(uint16_t pageIdx)
{
    uint32_t address = (_pageDirIdx & 0x3FF) << 22; // 10 bits of page dir index
    address |= (pageIdx & 0x3FF) << 12;             // 10 bits of page table index
    return address;
}

uint32_t PageTable::physAddressOfPageFrame(uint16_t pageIdx)
{
    return _pageTable[pageIdx] & PAGE_TABLE_ADDRESS;
}

bool PageTable::isMapped(uint32_t physAddr, uint32_t& virtAddr)
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        //screen << "_pageTable value: 0x" << (uint32_t)_pageTable << "\n";
        //screen << "accessed pageTable[i]\n";
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            // get physical bounds of page frame
            uint32_t pageFrameAddress = physAddressOfPageFrame(i);
            uint32_t maxPageFrameAddr = pageFrameAddress + 0xFFF;

            // does the physical address fall within this page frame?
            if (physAddr >= pageFrameAddress && physAddr <= maxPageFrameAddr)
            {
                // set the corresponding virtual address
                auto offset = physAddr & 0xFFF;
                auto virtBase = (_pageDirIdx & 0x3FF) << 22;    // 10 bits of page dir index
                virtBase |= (i & 0x3FF) << 12;                  // 10 bits of page table index
                virtAddr = virtBase | offset;
                return true;
            }
        }
    }
    return false;   // no mappings found
}

int PageTable::ptArrayIndex()
{
    return _pageDirIdx - KERNEL_PAGEDIR_START_IDX;
}

void PageTable::setPageTablePointer()
{
    _pageTable = __pageTablePtrs[ptArrayIndex()].pageTable;
}

}