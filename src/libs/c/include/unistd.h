#ifndef _UNISTD_H
#define _UNISTD_H 1

#define STDIN_FILENO  (0)
#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

typedef int pid_t;
typedef __SIZE_TYPE__ size_t;
typedef long ssize_t;

#ifdef __cplusplus
extern "C"
{
#endif

int close(int fildes);

int dup(int fildes);

int dup2(int fildes, int fildes2);

int execl(const char* path, const char* arg0, ...);

int execv(const char* path, char* const argv[]);

pid_t fork();

pid_t getpid();

pid_t getppid();

ssize_t read(int fildes, void* buf, size_t nbyte);

ssize_t write(int fildes, const void* buf, size_t nbyte);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UNISTD_H */
