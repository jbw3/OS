#pragma once

#include <pageframemgr.h>

namespace os {

/**
 * @brief getAddressOfPage returns the base address of the
 * page to which this address belongs.
 */
uint32_t getAddressOfPage(uint32_t addr);

/**
 * @brief autoMapKernelPageForAddress maps the given physical address to
 * an arbitrary kernel virtual address, and returns the
 * virtual address that now corresponds to the supplied physical address.
 * This is useful when a specific physical address must be mapped, but the
 * exact virtual address is unimportant.
 */
uint32_t autoMapKernelPageForAddress(uint32_t physAddr, PageFrameMgr* pfMgr);
// CLS TODO: consider mapping entire first 16MB phys to 3GB virt
// for speed?

}