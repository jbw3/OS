struct AtExitEntry
{
    void (*destructor)(void*);
    void* objptr;
    void* dso;
};

constexpr unsigned int MAX_AT_EXIT_FUNCTIONS = 64;
AtExitEntry atExitFunctions[MAX_AT_EXIT_FUNCTIONS];
unsigned int numAtExitFuncs = 0;

extern "C"
int __cxa_atexit(void (*destructor)(void*), void* objptr, void* dso)
{
    if (numAtExitFuncs >= MAX_AT_EXIT_FUNCTIONS)
    {
        // return an error if there are no spots left in the array
        return -1;
    }
    else
    {
        atExitFunctions[numAtExitFuncs].destructor = destructor;
        atExitFunctions[numAtExitFuncs].objptr = objptr;
        atExitFunctions[numAtExitFuncs].dso = dso;

        ++numAtExitFuncs;
        return 0;
    }
}

extern "C"
void __cxa_finalize(void* destructor)
{
    // a null pointer passed in means call all destructors
    if (destructor == nullptr)
    {
        for (; numAtExitFuncs > 0; --numAtExitFuncs)
        {
            unsigned int idx = numAtExitFuncs - 1;
            (atExitFunctions[idx].destructor)(atExitFunctions[idx].objptr);
        }
    }
    else
    {
        // search for the destructor
        unsigned int idx = 0;
        while (idx < numAtExitFuncs && atExitFunctions[idx].destructor != destructor)
        {
            ++idx;
        }

        // if we found the destructor, then call it
        if (idx >= numAtExitFuncs)
        {
            (atExitFunctions[idx].destructor)(atExitFunctions[idx].objptr);

            // shift entries in the array down by one
            for (unsigned int i = idx; idx + 1 < numAtExitFuncs; ++i)
            {
                atExitFunctions[i] = atExitFunctions[i + 1];
            }

            --numAtExitFuncs;
        }
    }
}
