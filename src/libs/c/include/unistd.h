#ifndef _UNISTD_H
#define _UNISTD_H 1

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

typedef __SIZE_TYPE__ size_t;
typedef long ssize_t;

#ifdef __cplusplus
extern "C"
{
#endif

ssize_t write(int fildes, const void* buf, size_t nbyte);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UNISTD_H */
