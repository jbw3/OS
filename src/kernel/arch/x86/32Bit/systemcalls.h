#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

struct registers;

/**
 * @brief System call interrupt handler.
 */
void systemCallHandler(const registers* regs);

#endif // SYSTEM_CALLS_H_
