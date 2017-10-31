#include <os.h>
#include "systemcall.h"

void clearTerminal()
{
    systemCall(SYSTEM_CALL_CLEAR_TERMINAL);
}

int getNumModules()
{
    return systemCall(SYSTEM_CALL_GET_NUM_MODULES);
}

void getModuleName(int index, char* name)
{
    systemCall(SYSTEM_CALL_GET_MODULE_NAME, index, name);
}

uint16_t getKey()
{
    return systemCall(SYSTEM_CALL_GET_KEY);
}

void setTerminalBackground(enum EColor color)
{
    systemCall(SYSTEM_CALL_CONFIG_TERMINAL, static_cast<int>(color), -1);
}

void setTerminalForeground(enum EColor color)
{
    systemCall(SYSTEM_CALL_CONFIG_TERMINAL, -1, static_cast<int>(color));
}
