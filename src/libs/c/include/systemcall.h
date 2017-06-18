#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

const unsigned int SYSTEM_CALL_WRITE = 0;

#ifdef __cplusplus
extern "C"
{
#endif

int systemCall0(unsigned int num);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_CALLS_H_
