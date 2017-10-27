#include <os.h>
#include "systemcall.h"

int getNumModules()
{
    return systemCall(SYSTEM_CALL_GET_NUM_MODULES);
}

void getModuleName(int index, char* name)
{
    systemCall(SYSTEM_CALL_GET_MODULE_NAME, index, name);
}
