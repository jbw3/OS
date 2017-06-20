
namespace os {

/**
 * @brief getAddressOfPage returns the base address of the
 * page to which this address belongs.
 */
uint32_t getAddressOfPage(uint32_t addr);

/**
 * @brief kAutoMap maps the given physical address to
 * an arbitrary kernel virtual address, and return the
 * virtual address used. This is useful when a specific
 * physical address must be mapped, but the exact virtual
 * address is unimportant.
 *
 * CLS TODO: consider mapping entire first 16MB phys to 3GB virt
 * for speed?
 */
uint32_t kAutoMap(uint32_t pAddr);

}