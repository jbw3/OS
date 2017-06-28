#pragma once

#include <pageframemgr.h>

namespace mem {

/**
 * @brief isMappedByKernel returns true if the given physical address
 * is mapped by a kernel page.
 * @param physAddr is the physical address
 * @param virtAddr is set to the corresponding virtual address if the
 * given physical address is mapped. This value is only valid if the
 * function returns true.
 */
bool isMappedByKernel(uint32_t physAddr, uint32_t& virtAddr);

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