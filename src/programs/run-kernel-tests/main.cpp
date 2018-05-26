#include <os.h>
#include <stdio.h>

int main()
{
    size_t numTests = 0;
    size_t numFailed = 0;
    int rv = runKernelTests(&numTests, &numFailed);

    size_t numPassed = numTests - numFailed;

    /// @todo This printf format only works if sizeof(size_t) == sizeof(int)
    static_assert(sizeof(size_t) == sizeof(int), "This printf format assumes the size of size_t equals the size of int.");
    printf("Total: %u, Passed: %u, Failed: %u\n", numTests, numPassed, numFailed);

    return rv;
}
